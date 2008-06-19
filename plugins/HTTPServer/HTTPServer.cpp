// HTTPServer.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTPVDIR.h"
#include <algorithm>

std::map<std::string,BZFSHTTPVDir*> virtualDirs;

bool registered = false;

BZ_GET_PLUGIN_VERSION

bool RegisterVDir ( void* param)
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

bool RemoveVDir ( void* param)
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
  int connectionID;

  std::string currentData;

  std::string vdir;
  std::string resource;

  HTTPRequestType request;

  std::vector<std::string> headerLines;
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
    bz_registerCallBack("RegisterHTTPDVDir",&RegisterVDir);
    bz_registerCallBack("RemoveHTTPDVDir",&RemoveVDir);

    bz_registerEvent (bz_eTickEvent,server);
    bz_registerEvent (bz_eNewNonPlayerConnection,server);

    if(server)
      delete(server);
    server = new HTTPServer;

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

  // see what our status is
  if (!connection.request)
  {
    std::string &data = connection.currentData;
    const char *p = strstr(data.c_str(),"\r\n");

    if (p != NULL )
    {
      size_t offset = p-data.c_str();
      // ok lets get a new string that is just the data
      std::string command;
      std::copy(data.begin(),data.begin()+offset,command.begin());

      // increment past the command to the start of the \r\n so the rest can
      // parse it, since they need to find the \r\n\r\n and it could be right after this command if it's a simple command
      data.erase(data.begin(),data.begin()+offset);

      std::vector<std::string> params = tokenize(command,std::string(" "),0,false);

      if (params.size() > 2)
      {
	std::string request = tolower(params[0]);
	if (request == "get")
	  connection.request = eGet;
	else if (request == "head")
	  connection.request = eHead;
	else if (request == "post")
	  connection.request = ePost;
	else if (request == "put")
	  connection.request = ePut;
	else if (request == "delete")
	  connection.request = eDelete;
	else if (request == "trace")
	  connection.request = eTrace;
	else if (request == "options")
	  connection.request = eOptions;
	else if (request == "connect")
	  connection.request = eConnect;
	else 
	  connection.request = eOther;
      }
    }
  }
  
  if (connection.request)
  {
    // umm yeah do stuff to see if we have all the data for our request
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
