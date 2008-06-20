// HTTPServer.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTPVDIR.h"
#include <algorithm>
#include <sstream>

typedef std::map<std::string,BZFSHTTPVDir*> VirtualDirs;

VirtualDirs virtualDirs;

bool registered = false;
int lastRequestID = 1;

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

class HTTPConnection;

class HTTPRequest
{
public:
  HTTPRequest(HTTPConnection &connection);

  HTTPRequestType request;

  std::string vdir;
  std::string resource;
  std::map<std::string, std::string> paramaters;

  std::map<std::string,std::string> header;

  std::string body;
};

class HTTPConnection
{
public:
  HTTPConnection() : connectionID(-1), requestComplete(false) 
		     ,headerComplete(false), contentSize(0) 
		     ,bodyEnd(0), request(eUnknown){};

  void flush ( void )
  {
    body = "";
    contentSize = 0;
    bodyEnd = 0;
    headerComplete = false;
    requestComplete = false;
    request = eUnknown;
    vdir = "";
    resource = "";
    host = "";
    header.clear();
  }

  void update ( void );

  int connectionID;

  std::string currentData;
  std::string body;
  size_t      contentSize;
  size_t      bodyEnd;
  bool	      headerComplete;
  bool	      requestComplete;

  HTTPRequestType   request;
  std::string	    vdir;
  std::string	    resource;
  std::string	    host;
  std::map<std::string, std::string> header;

  class HTTPTask
  {
  public:
    HTTPTask(HTTPReply& r);

    bool update ( int connectionID );

    std::string page;
    size_t pos;

    HTTPReply reply;
    size_t generated;

    // other stuff like cookies, etc..
  };

  std::vector<HTTPTask> processingTasks;  // tasks working
  std::vector<HTTPTask> pendingTasks;	  // tasks waiting
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

  void processRequest (const HTTPRequest &request, int connectionID );

  HTTPConnectionMap liveConnections;

  std::string baseURL;

private:
  void send100Continue ( int connectionID );
  void send403Error ( int connectionID );
  void send501Error ( int connectionID );
  void sendOptions ( int connectionID, bool p );

  void put(BZFSHTTPVDir* vdir, int connectionID, const HTTPRequest &request);
  void generateIndex(int connectionID, const HTTPRequest &request);
  void generatePage(BZFSHTTPVDir* vdir, int connectionID, const HTTPRequest &request);
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
  if(d && s)
  {
    char *t = (char*)malloc(s+1);
    memcpy(t,d,s);
    t[s] = 0;
    connection.currentData += t;
    free(t);
  }

  // see what our status is
  if (!connection.request)
  {
    std::stringstream stream(connection.currentData);

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

  if (connection.request) // we know the type, so we can get the rest of the data or bail out
  {
    if (!connection.requestComplete && (connection.request == ePost ||connection.request == ePut) ) // if the request is a post, tell the client to send us the rest of the body
      send100Continue(connectionID);

    size_t headerEnd = connection.currentData.find_first_of("\r\n\r\n");
  
    if (!connection.headerComplete && headerEnd != std::string::npos)
    {
      bool done = false;  // ok we have the header and we don't haven't processed it yet

      // read past the command
      size_t p = connection.currentData.find_first_of("\r\n");
      p+= 2;

      while ( p < headerEnd )
      {
	size_t p2 = connection.currentData.find_first_of("\r\n",p);

	std::string line;
	line.resize(p2-p);
	std::copy(connection.currentData.begin()+p,connection.currentData.begin()+p2,line.begin());
	p = p2+1;

	std::vector<std::string> headerLine = tokenize(line,std::string(":"),1,false);
	if ( headerLine.size() > 1)
	{
	  std::string &key = headerLine[0];
	  if ( compare_nocase(key,"Host") == 0)
	    connection.host = line.c_str()+key.size()+2;
	  else if ( compare_nocase(key,"Content-Length") == 0)
	    connection.contentSize = (size_t)atoi(headerLine[1].c_str());
	  else
	    connection.header[key] = headerLine[1];
	}
      }
      connection.headerComplete = true;
    }

    if (connection.headerComplete && !connection.requestComplete)
    {
      connection.bodyEnd = headerEnd+4;

      if (connection.request != ePost && connection.request != ePut)
	connection.requestComplete = true; // there is nothing after the header we care about
      else
      {
	if (connection.contentSize)
	{
	  headerEnd += 4;
	  if (connection.currentData.size()-headerEnd >= connection.contentSize)
	  {
	    // read in that body!
	    connection.body.resize(connection.currentData.size()-headerEnd);
	    std::copy(connection.currentData.begin()+headerEnd,connection.currentData.end(),connection.body.end());
	    connection.requestComplete = true;

	    connection.bodyEnd += connection.contentSize;
	  }
	}
	else
	  connection.requestComplete = true;
      }
    }
  }

  if (connection.requestComplete)
  {
    // special, if it's a trace, just fire it back to them
    if (connection.request == eTrace)
    {
      bz_sendNonPlayerData ( connectionID, connection.currentData.c_str(),(unsigned int)connection.bodyEnd);

      std::string nubby = connection.currentData.c_str()+connection.bodyEnd;
      connection.currentData = nubby;
      connection.flush();
    }
    else
    {
      // parse it all UP and build up a complete request
      std::string nubby = connection.currentData.c_str()+connection.bodyEnd;
      connection.currentData = nubby;

      // rip off what we need for the request, and then flush
      processRequest(HTTPRequest(connection),connectionID);
      connection.flush();
    }

    if (connection.currentData.size())
      pending(connectionID,NULL,0);
  }
}

void HTTPServer::disconnect ( int connectionID )
{
  HTTPConnectionMap::iterator itr = liveConnections.find(connectionID);

  if (itr != liveConnections.end())
    liveConnections.erase(itr);
}

void HTTPServer::update ( void )
{
  HTTPConnectionMap::iterator itr = liveConnections.begin();

  while (itr != liveConnections.end())
  {
    itr->second.update();
    itr++;
  }
}

void HTTPServer::put(BZFSHTTPVDir* vdir, int connectionID,const HTTPRequest &request)
{
  HTTPConnectionMap::iterator itr = liveConnections.find(connectionID);

  if (itr == liveConnections.end())
    return;
  
  HTTPConnection &connection = itr->second;

  HTTPReply reply;
  reply.baseURL = baseURL+"/";
  reply.baseURL += vdir->getVDir();
  reply.baseURL += "/";

  int requestID = lastRequestID++;

  void *d = malloc(request.body.size());
  std::stringstream stream(request.body);
  stream.get((char*)d,(std::streamsize)request.body.size());

  if (vdir->put(reply,vdir->getVDir(),request.resource.c_str(),connectionID,requestID,d,request.body.size()))
    connection.processingTasks.push_back(HTTPConnection::HTTPTask(reply));
  else
    connection.pendingTasks.push_back(HTTPConnection::HTTPTask(reply));

  free(d);
  connection.update();
}

void HTTPServer::generateIndex(int connectionID, const HTTPRequest &request)
{
  HTTPConnectionMap::iterator itr = liveConnections.find(connectionID);

  if (itr == liveConnections.end())
    return;

  HTTPConnection &connection = itr->second;

  HTTPReply reply;

  reply.docType = HTTPReply::eHTML;
  reply.returnCode = HTTPReply::e200OK;
  reply.body = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"><html><head><title>Index page for " + baseURL + "</title></head>";
  reply.body += "<body>";

  VirtualDirs::iterator dirItr = virtualDirs.begin();

  while (dirItr != virtualDirs.end())
  {
    std::string vdirName = dirItr->second->getVDir();
    std::string vDirDescription = dirItr->second->getDescription();
    reply.body += "<a href=\"" + baseURL + vdirName + "\">" + vdirName +"</a>&nbsp" +vDirDescription +"</br>"; 
    dirItr++;
  }

  reply.body += "</body></html>";

  connection.processingTasks.push_back(HTTPConnection::HTTPTask(reply));
  connection.update();
}

void HTTPServer::generatePage(BZFSHTTPVDir* vdir, int connectionID, const HTTPRequest &request)
{
  HTTPConnectionMap::iterator itr = liveConnections.find(connectionID);

  if (itr == liveConnections.end())
    return;

  HTTPConnection &connection = itr->second;

  HTTPReply reply;
  reply.baseURL = baseURL+"/";
  reply.baseURL += vdir->getVDir();
  reply.baseURL += "/";

  int requestID = lastRequestID++;

  if (vdir->generatePage(reply,vdir->getVDir(),request.resource.c_str(),connectionID,requestID))
    connection.processingTasks.push_back(HTTPConnection::HTTPTask(reply));
  else
    connection.pendingTasks.push_back(HTTPConnection::HTTPTask(reply));

  connection.update();
}

void HTTPServer::processRequest (const HTTPRequest &request, int connectionID )
{
  // check the request to see if it'll have any thing we care to process

  // find the vdir handler

  BZFSHTTPVDir *vdir = NULL;

  VirtualDirs::iterator itr = virtualDirs.find(request.vdir);

  if (itr != virtualDirs.end())
    vdir = itr->second;

  switch(request.request)
  {
    case ePut:
      if (!vdir || !vdir->supportPut())
	send403Error(connectionID);
      else
	put(vdir,connectionID,request);
      break;

    case eHead:
    case eGet:
    case ePost:
      if(!vdir)
	generateIndex(connectionID,request);
      else
	generatePage(vdir,connectionID,request);
      break;

    case eOptions:
      sendOptions(connectionID, vdir ? vdir->supportPut() : false );
      break;

    case eDelete:
    case eConnect:
      send501Error(connectionID);
      break;
  }
}

void HTTPServer::send100Continue ( int connectionID )
{
  std::string httpHeaders;
  httpHeaders += "HTTP/1.1 100 Continue\n\n";

  bz_sendNonPlayerData ( connectionID, httpHeaders.c_str(), (unsigned int)httpHeaders.size());
}

void HTTPServer::send403Error ( int connectionID )
{
  std::string httpHeaders;
  httpHeaders += "HTTP/1.1 403 Forbidden\n\n";

  bz_sendNonPlayerData ( connectionID, httpHeaders.c_str(), (unsigned int)httpHeaders.size());
}

void HTTPServer::send501Error ( int connectionID )
{
  std::string httpHeaders;
  httpHeaders += "HTTP/1.1 501 Not Implemented\n\n";

  bz_sendNonPlayerData ( connectionID, httpHeaders.c_str(), (unsigned int)httpHeaders.size());
}

void HTTPServer::sendOptions ( int connectionID, bool p )
{
  std::string httpHeaders;
  httpHeaders += "HTTP/1.1 200 Ok\n";
  httpHeaders += "Allow: GET, HEAD, POST, OPTIONS";
  if (p)
    httpHeaders += ", PUT";
  httpHeaders += "\n\n";

  bz_sendNonPlayerData ( connectionID, httpHeaders.c_str(), (unsigned int)httpHeaders.size());
}

void parseParams ( std::map<std::string, std::string> &params, const std::string &text, size_t offset )
{
  std::vector<std::string> items = tokenize(text,"&",0,false,offset);

  for (size_t i = 0; i < items.size(); i++)
  {
    std::string &item = items[i];

   std::vector<std::string> t = tokenize(item,"=",0,false);
   if (t.size() > 1)
      params[ t[0] ] = t[1];
   else
     params[item] = "";
  }
}

HTTPRequest::HTTPRequest(HTTPConnection &connection)
{
  request = connection.request;
  vdir = connection.vdir;
  resource = connection.vdir;
  header = connection.header;

  if (request == eGet)
  {
    // parse out the paramaters from the resource
    size_t q = resource.find_first_of('?');
    if (q != std::string::npos)
      parseParams(paramaters,resource,q+1);
  }
  else if (request == ePost && connection.contentSize > 0)
    parseParams(paramaters,connection.body,0);
  else if (request == ePut && connection.contentSize > 0)
    body = connection.body;
}

void HTTPConnection::update ( void )
{
  // hit the processings, then the pendings
  std::vector<size_t> killList;

  for (size_t i = 0; i < processingTasks.size(); i++ )
  {
    if (processingTasks[i].update(connectionID))
      killList.push_back(i);
  }

  std::vector<size_t>::reverse_iterator itr = killList.rbegin();
  while (itr != killList.rend())
  {
    size_t offset = *itr;
    processingTasks.erase(processingTasks.begin()+offset);
    itr++;
  }
}

const char* getMimeType ( HTTPReply::DocumentType docType )
{
  switch(docType) 
  {
    case HTTPReply::eOctetStream:
      return "application/octet-stream";

    case HTTPReply::eBinary:
      return "application/binary";

    case HTTPReply::eHTML:
      return "text/html";

    default:
      break;
  }
  return "text/plain";
}


HTTPConnection::HTTPTask::HTTPTask(HTTPReply& r):reply(r), pos(0)
{
  // start a new one
  page += "HTTP/1.1";
  
  switch(reply.returnCode)
  {
    case HTTPReply::e200OK:
      page += " 200 OK\n";
      break;

    case HTTPReply::e301Redirect:
      if (reply.redirectLoc.size())
      {
	page += " 301 Moved Permanently\n";
	page += "Location: " + reply.redirectLoc + "\n";
      }
      else
	page += " 500 Server Error\n";
      break;

    case HTTPReply::e500ServerError:
      page += " 500 Server Error\n";
      break;

    case HTTPReply::e404NotFound:
      page += " 404 Not Found\n";
      break;

    case HTTPReply::e403Forbiden:
      page += " 500 Forbidden\n";
      break;
  }

  if (reply.returnCode == HTTPReply::e200OK)
    page += "Connection: close\n";
  else
    page += "Connection: close\n";

  if (reply.body.size())
  {
    page += format("Content-Length: %d\n", reply.body.size());

    page += "Content-Type: ";
    page += getMimeType(reply.docType);
    page += "\n";
  }

  page += "\n";

  if (reply.body.size())
    page += reply.body;

  generated = reply.body.size();
}

bool HTTPConnection::HTTPTask::update ( int connectionID )
{
  // find out how much to write

  if (pos >= page.size())
    return true;

  size_t write = page.size();
  size_t left = write-pos;

  if ( left > 1000)
    write = pos + 1000;

  if ( !bz_sendNonPlayerData (connectionID, page.c_str()+pos, (unsigned int)write))
    return true;

  pos += write;

  return pos >= page.size();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
