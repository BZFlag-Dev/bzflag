// HTTPServer.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTPVDIR.h"
#include <algorithm>
#include <sstream>

std::map<std::string,BZFSHTTPVDir*> virtualDirs;

bool registered = false;

BZ_GET_PLUGIN_VERSION

bool RegisterVDir ( void* param )
{
  BZFSHTTPVDir *handler = (BZFSHTTPVDir*)param;
  if (!handler)
    return false;

  std::string name = handler->getVDir();
  if (!name.size())
    return false;

  if (virtualDirs.find(name) != virtualDirs.end())
    return false;
  
  virtualDirs[name] = handler;
  return true;
}

bool RemoveVDir ( void* param )
{
  BZFSHTTPVDir *handler = (BZFSHTTPVDir*)param;
  if (!handler)
    return false;

  std::string name = handler->getVDir();
  if (!name.size())
    return false;

  if (virtualDirs.find(name) == virtualDirs.end())
    return false;

  virtualDirs.erase(virtualDirs.find(name));
  return true;
}

typedef enum 
{
  eUnknown = 0,
  eHead,
  eGet,
  ePost,
  ePut,
  eDelete,
  eTrace,
  eOptions,
  eConnect,
  eOther
}HTTPRequestType;

class HTTPConnection
{
public:
  HTTPConnection() : connectionID(-1), requestComplete(false), headerComplete(false){};

  int connectionID;

  std::string currentData;

  std::string vdir;
  std::string resource;
  std::vector<std::string> header;
  std::string body;
  std::string host;

  HTTPRequestType request;
  bool		  headerComplete;
  bool		  requestComplete;

  std::map<std::string,std::string> paramaters;
};

typedef std::map<int,HTTPConnection> HTTPConnectionMap;

class HTTPServer : public bz_EventHandler, bz_NonPlayerConnectionHandler
{
public:
  HTTPServer();
  virtual ~HTTPServer();

  // BZFS callback methods
  virtual void process ( bz_EventData *eventData );
  virtual void pending ( int connectionID, void *d, unsigned int s );
  virtual void disconnect ( int connectionID );

protected:
  void update ( void );

  HTTPConnectionMap liveConnections;

  std::string baseURL;

private:
  void sendContinue ( int connectionID );
};

HTTPServer *server = NULL;

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
  registered = bz_callbackExists("RegisterHTTPDVDir");

  if (!registered)
  {
    if(server)
      delete(server);
    server = new HTTPServer;

    bz_registerCallBack("RegisterHTTPDVDir",&RegisterVDir);
    bz_registerCallBack("RemoveHTTPDVDir",&RemoveVDir);

    bz_registerEvent (bz_eTickEvent,server);
    bz_registerEvent (bz_eNewNonPlayerConnection,server);

    registered = true;
    bz_debugMessage(4,"HTTPServer plug-in loaded");
  }
  else
    bz_debugMessage(1,"HTTPServer *WARNING* plug-in loaded more then once, this instance will not be used");

  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  if (registered)
  {
    bz_removeCallBack("RegisterHTTPDVDir",&RegisterVDir);
    bz_removeCallBack("RemoveHTTPDVDir",&RemoveVDir);

    bz_removeEvent (bz_eTickEvent,server);
    bz_removeEvent (bz_eNewNonPlayerConnection,server); 
  }

  if(server)
    delete(server);

  server = NULL;
  bz_debugMessage(4,"HTTPServer plug-in unloaded");
  return 0;
}

HTTPServer::HTTPServer()
{
  baseURL = "http://";
  std::string host = "localhost";
  if (bz_getPublicAddr().size())
    host = bz_getPublicAddr().c_str();

  // make sure it has the port
  if ( strrchr(host.c_str(),':') == NULL )
    host += format(":%d",bz_getPublicPort());

  baseURL += host +"/";
}

HTTPServer::~HTTPServer()
{

}

void HTTPServer::process ( bz_EventData *eventData )
{
  if ( eventData->eventType == bz_eTickEvent) 
      update();
  else
  {
    bz_NewNonPlayerConnectionEventData_V1 *connData = (bz_NewNonPlayerConnectionEventData_V1*)eventData;

    // log out the data if our level is high enough
    if (bz_getDebugLevel() >= 4)
    {
      char *temp = (char*)malloc(connData->size+1);
      memcpy(temp,connData->data,connData->size);
      temp[connData->size] = 0;
      bz_debugMessagef(4,"Plug-in HTTPServer: Non ProtoConnection connection from %d with %s",connData->connectionID,temp);
      free(temp);
    }

    // we go an accept everyone so that we can see if they are going to give us an HTTP command
    if(bz_registerNonPlayerConnectionHandler ( connData->connectionID, this ) )
    {
      // record the connection
      HTTPConnection connection;
      connection.connectionID = connData->connectionID;
      connection.request = eUnknown;

      HTTPConnectionMap::iterator itr = liveConnections.find(connection.connectionID);

      if (itr != liveConnections.end())
	liveConnections.erase(itr);	// something weird is happening here

      liveConnections[connection.connectionID] = connection;
      
      // go and process any data they have and see what the deal is
      pending ( connData->connectionID, (char*)connData->data, connData->size );
    }
  }
}

void HTTPServer::pending ( int connectionID, void *d, unsigned int s )
{
  HTTPConnectionMap::iterator itr = liveConnections.find(connectionID);

  if (itr == liveConnections.end())
    return;

  HTTPConnection &connection = itr->second;

  // grab the current data
  char *t = (char*)malloc(s+1);
  memcpy(t,d,s);
  t[s] = 0;
  connection.currentData += t;
  free(t);

  std::stringstream stream(connection.currentData);

  // see what our status is
  if (!connection.request)
  {
    std::string request, resource, httpVersion;
    stream >> request >> resource >> httpVersion;

    if (request.size() && resource.size() && httpVersion.size())
    {
      if (compare_nocase(request,"get") == 0)
	connection.request = eGet;
      else if (compare_nocase(request,"head") == 0)
	connection.request = eHead;
      else if (compare_nocase(request,"post") == 0)
	connection.request = ePost;
      else if (compare_nocase(request,"put") == 0)
	connection.request = ePut;
      else if (compare_nocase(request,"delete") == 0)
	connection.request = eDelete;
      else if (compare_nocase(request,"trace") == 0)
	connection.request = eTrace;
      else if (compare_nocase(request,"options") == 0)
	connection.request = eOptions;
      else if (compare_nocase(request,"connect") == 0)
	connection.request = eConnect;
      else 
	connection.request = eOther;

      if (httpVersion != "HTTP/1.1" && httpVersion != "HTTP/1.0")
	bz_debugMessagef(1,"HTTPServer HTTP version of %s requested",httpVersion.c_str());

      if (resource.size() > 1)
      {
	size_t p = resource.find_first_of('/');
	if (p != std::string::npos)
	{
	  if (p == 0)
	    p = resource.find_first_of('/',p+1);

	  if (p == std::string::npos ) // there is only one / so the stuff after the slash in the vdir and the resource is NULL
	  {
	    connection.vdir.resize(resource.size()-1);
	    std::copy(resource.begin()+1,resource.end(),connection.vdir.begin());
	  }
	  else
	  {
	    connection.vdir.resize(p-1);
	    std::copy(resource.begin()+1,resource.begin()+p,connection.vdir.begin());

	    connection.resource.resize(resource.size()-p-1);
	    std::copy(resource.begin()+p+1,resource.end(),connection.resource.begin());
	  }
	}
      }
    }
  }

  if (connection.request)
  {
    if (!connection.requestComplete && (connection.request == ePost ||connection.request == ePut) ) // if the request is a post, tell the client to send us the rest of the body
      sendContinue(connectionID);

    if (!connection.headerComplete)
    {
      bool done = false;

      while (!done)
      {
	std::string key;
	stream >> key;
      }
     // stream.getline()

    }
    // parse out as many of the header lines as we have until we reach the end
  }
}

void HTTPServer::disconnect ( int connectionID )
{

}

void HTTPServer::update ( void )
{

}

void HTTPServer::sendContinue ( int connectionID )
{
  std::string httpHeaders;
  httpHeaders += "HTTP/1.1 200 OK\n\n";

  bz_sendNonPlayerData ( connectionID, httpHeaders.c_str(), (unsigned int)httpHeaders.size());
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
