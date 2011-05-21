/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/// HTTPServer.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"
#include <algorithm>
#include <sstream>
#include <time.h>

#define FORCE_CLOSE false

typedef std::map<std::string, BZFSHTTP*> VirtualDirs;

VirtualDirs virtualDirs;

bool registered = false;
int lastRequestID = 1;

BZ_GET_PLUGIN_VERSION

bool RegisterVDir(void* param) {
  BZFSHTTP* handler = (BZFSHTTP*)param;
  if (!handler) {
    return false;
  }

  std::string name = handler->getVDir();
  if (!name.size()) {
    return false;
  }

  if (virtualDirs.find(name) != virtualDirs.end()) {
    return false;
  }

  virtualDirs[name] = handler;
  return true;
}

bool RemoveVDir(void* param) {
  BZFSHTTP* handler = (BZFSHTTP*)param;
  if (!handler) {
    return false;
  }

  std::string name = handler->getVDir();
  if (!name.size()) {
    return false;
  }

  if (virtualDirs.find(name) == virtualDirs.end()) {
    return false;
  }

  virtualDirs.erase(virtualDirs.find(name));
  return true;
}

int generateSessionID(void) {
  short s[2];
  s[0] = rand();
  s[1] = rand();

  return *((int*)s);
}

class HTTPConnection {
  public:
    HTTPConnection() : connectionID(-1),
      requestComplete(false),
      headerComplete(false),
      contentSize(0),
      bodyEnd(0),
      request(eUnknown),
      sessionID(0) {};

    void flush(void) {
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

    bool update(void);

    int connectionID;

    // the current request as we process it
    std::string currentData;
    std::string body;
    size_t      contentSize;
    size_t      bodyEnd;
    bool        headerComplete;
    bool        requestComplete;

    int sessionID;

    HTTPRequestType   request;
    std::string     vdir;
    std::string     resource;
    std::string     host;
    std::map<std::string, std::string> header;

    void fillRequest(HTTPRequest& req);

    class HTTPTask {
      public:
        HTTPTask(HTTPReply& r, bool noBody);
        HTTPTask(const HTTPTask& t);

        virtual ~HTTPTask(void) {};

        void generateBody(HTTPReply& r, bool noBody);

        bool update(int connectionID);

        // std::string page;
        size_t pos;

        // a binary data management class
        class PageBuffer {
          public:
            virtual ~PageBuffer() {if (data) { free(data); }}
            PageBuffer() : bufferSize(0), data(NULL) {};

            PageBuffer(const char* str) {
              bufferSize = strlen(str);
              data = (char*)malloc(bufferSize);
              memcpy(data, str, bufferSize);
            }

            PageBuffer(const std::string& r) {
              bufferSize = r.size();
              data = (char*)malloc(bufferSize);
              memcpy(data, r.c_str(), bufferSize);
            }

            PageBuffer(const PageBuffer& r) {
              bufferSize = r.size();
              data = (char*)malloc(bufferSize);
              memcpy(data, r.data, bufferSize);
            }

            size_t append(const char* newData, size_t newSize) {
              if (!data) {
                data = (char*)malloc(newSize);
                memcpy(data, newData, newSize);
                bufferSize = newSize;
              }
              else {
                char* p = (char*)malloc(bufferSize + newSize);
                memcpy(p, data, bufferSize);
                memcpy(p + bufferSize, newData, newSize);
                bufferSize += newSize;
                free(data);
                data = p;
              }
              return bufferSize;
            }

            PageBuffer& operator += (const char* p) {
              if (p) {
                append(p, strlen(p));
              }
              return *this;
            }

            PageBuffer& operator += (const std::string& p) {
              if (p.size()) {
                append(p.c_str(), p.size());
              }
              return *this;
            }

            PageBuffer& operator = (const char* p) {
              if (data) {
                free(data);
              }
              data = NULL;

              if (p) {
                append(p, strlen(p));
              }
              return *this;
            }

            PageBuffer& operator = (const std::string& p) {
              if (data) {
                free(data);
              }
              data = NULL;

              if (p.size()) {
                append(p.c_str(), p.size());
              }
              return *this;
            }

            PageBuffer& operator = (const PageBuffer& p) {
              if (data) {
                free(data);
              }
              data = NULL;

              if (p.size()) {
                append(p.getData(), p.size());
              }
              return *this;
            }
            const size_t size(void)const {return bufferSize;}
            const char* getData(void) const { return data;}

          protected:
            size_t bufferSize;
            char* data;
        };
        PageBuffer pageBuffer;

        bool forceClose;
    };

    class PendingHTTPTask : public HTTPTask {
      public:
        PendingHTTPTask(HTTPReply& r, HTTPRequest& rq, bool noBody): HTTPTask(r, noBody), request(rq), reply(r) {};
        HTTPRequest request;
        HTTPReply reply;
    };

    std::vector<HTTPTask> processingTasks;  // tasks working
    std::vector<PendingHTTPTask> pendingTasks;    // tasks waiting
};

typedef std::map<int, HTTPConnection> HTTPConnectionMap;

class HTTPServer : public bz_EventHandler, bz_NonPlayerConnectionHandler {
  public:
    HTTPServer();
    virtual ~HTTPServer();

    // BZFS callback methods
    virtual void process(bz_EventData* eventData);
    virtual void pending(int connectionID, void* d, unsigned int s);
    virtual void disconnect(int connectionID);

  protected:
    void update(void);

    bool processRequest(HTTPRequest& request, int connectionID);

    HTTPConnectionMap liveConnections;

    std::string baseURL;

  private:
    void send100Continue(int connectionID);
    void send403Error(int connectionID);
    void send404Error(int connectionID);
    void send501Error(int connectionID);
    void sendOptions(int connectionID, bool p);

    void generateIndex(int connectionID, const HTTPRequest& request);
    bool generatePage(BZFSHTTP* vdir, int connectionID, HTTPRequest& request);
};

HTTPServer* server = NULL;

// some statics for speed
std::string serverVersion;
std::string serverHostname;

BZF_PLUGIN_CALL int bz_Load(const char* /*commandLine*/) {
  registered = bz_callbackExists("RegisterHTTPDVDir");

  if (!registered) {
    if (server) {
      delete(server);
    }
    server = new HTTPServer;

    srand((unsigned int)bz_getCurrentTime());

    bz_registerCallBack("RegisterHTTPDVDir", &RegisterVDir);
    bz_registerCallBack("RemoveHTTPDVDir", &RemoveVDir);

    bz_registerEvent(bz_eTickEvent, server);
    bz_registerEvent(bz_eNewNonPlayerConnection, server);

    serverVersion = bz_getServerVersion();

    registered = true;
    bz_debugMessage(4, "HTTPServer plug-in loaded");
  }
  else {
    bz_debugMessage(1, "HTTPServer *WARNING* plug-in loaded more then once, this instance will not be used");
  }

  return 0;
}

BZF_PLUGIN_CALL int bz_Unload(void) {
  if (registered) {
    bz_removeCallBack("RegisterHTTPDVDir", &RegisterVDir);
    bz_removeCallBack("RemoveHTTPDVDir", &RemoveVDir);

    bz_removeEvent(bz_eTickEvent, server);
    bz_removeEvent(bz_eNewNonPlayerConnection, server);
  }

  if (server) {
    delete(server);
  }

  server = NULL;
  bz_debugMessage(4, "HTTPServer plug-in unloaded");
  return 0;
}

HTTPServer::HTTPServer() {
  baseURL = "http://";
  serverHostname = "localhost";
  if (bz_getPublicAddr().size()) {
    serverHostname = bz_getPublicAddr().c_str();
  }

  // make sure it has the port
  if (strrchr(serverHostname.c_str(), ':') == NULL) {
    serverHostname += format(":%d", bz_getPublicPort());
  }

  baseURL += serverHostname + "/";
}

HTTPServer::~HTTPServer() {

}

void HTTPServer::process(bz_EventData* eventData) {
  if (eventData->eventType == bz_eTickEvent) {
    update();
  }
  else {
    bz_NewNonPlayerConnectionEventData_V1* connData = (bz_NewNonPlayerConnectionEventData_V1*)eventData;

    // log out the data if our level is high enough
    if (bz_getDebugLevel() >= 4) {
      char* temp = (char*)malloc(connData->size + 1);
      memcpy(temp, connData->data, connData->size);
      temp[connData->size] = 0;
      bz_debugMessagef(4, "Plug-in HTTPServer: Non ProtoConnection connection from %d with %s", connData->connectionID, temp);
      free(temp);
    }

    // we go an accept everyone so that we can see if they are going to give us an HTTP command
    if (bz_registerNonPlayerConnectionHandler(connData->connectionID, this)) {
      // record the connection
      HTTPConnection connection;
      connection.connectionID = connData->connectionID;
      connection.request = eUnknown;
      connection.sessionID = generateSessionID();  // new ID in case they don't have one

      HTTPConnectionMap::iterator itr = liveConnections.find(connection.connectionID);

      if (itr != liveConnections.end()) {
        liveConnections.erase(itr);  // something weird is happening here
      }

      liveConnections[connection.connectionID] = connection;

      // go and process any data they have and see what the deal is
      pending(connData->connectionID, (char*)connData->data, connData->size);
    }
  }
}

void HTTPServer::pending(int connectionID, void* d, unsigned int s) {
  HTTPConnectionMap::iterator itr = liveConnections.find(connectionID);

  if (itr == liveConnections.end()) {
    return;
  }

  HTTPConnection& connection = itr->second;

  // grab the current data
  if (d && s) {
    char* t = (char*)malloc(s + 1);
    memcpy(t, d, s);
    t[s] = 0;
    connection.currentData += t;
    free(t);
  }

  // see what our status is
  if (!connection.request) {
    std::stringstream stream(connection.currentData);

    std::string request, resource, httpVersion;
    stream >> request >> resource >> httpVersion;

    if (request.size() && resource.size() && httpVersion.size()) {
      if (compare_nocase(request, "get") == 0) {
        connection.request = eGet;
      }
      else if (compare_nocase(request, "head") == 0) {
        connection.request = eHead;
      }
      else if (compare_nocase(request, "post") == 0) {
        connection.request = ePost;
      }
      else if (compare_nocase(request, "put") == 0) {
        connection.request = ePut;
      }
      else if (compare_nocase(request, "delete") == 0) {
        connection.request = eDelete;
      }
      else if (compare_nocase(request, "trace") == 0) {
        connection.request = eTrace;
      }
      else if (compare_nocase(request, "options") == 0) {
        connection.request = eOptions;
      }
      else if (compare_nocase(request, "connect") == 0) {
        connection.request = eConnect;
      }
      else {
        connection.request = eOther;
      }

      if (httpVersion != "HTTP/1.1" && httpVersion != "HTTP/1.0") {
        bz_debugMessagef(1, "HTTPServer HTTP version of %s requested", httpVersion.c_str());
      }

      if (resource.size() > 1) {
        size_t p = resource.find_first_of('/');
        if (p != std::string::npos) {
          if (p == 0) {
            p = resource.find_first_of('/', p + 1);
          }

          if (p == std::string::npos) {
            // there is only one / so the stuff after the slash in the vdir and the resource is NULL
            connection.vdir.resize(resource.size() - 1);
            std::copy(resource.begin() + 1, resource.end(), connection.vdir.begin());
          }
          else {
            connection.vdir.resize(p - 1);
            std::copy(resource.begin() + 1, resource.begin() + p, connection.vdir.begin());

            connection.resource.resize(resource.size() - p - 1);
            std::copy(resource.begin() + p + 1, resource.end(), connection.resource.begin());
          }
        }
      }
    }
  }

  if (connection.request) {
    // we know the type, so we can get the rest of the data or bail out
    if (!connection.requestComplete &&
        (connection.request == ePost || connection.request == ePut)) { // if the request is a post, tell the client to send us the rest of the body
      send100Continue(connectionID);
    }

    size_t headerEnd = find_first_substr(connection.currentData, "\r\n\r\n");

    if (!connection.headerComplete && headerEnd != std::string::npos) {
      bool done = false;  // ok we have the header and we don't haven't processed it yet

      // read past the command
      size_t p = find_first_substr(connection.currentData, "\r\n");
      p += 2;

      while (p < headerEnd) {
        size_t p2 = find_first_substr(connection.currentData, "\r\n", p);

        std::string line(connection.currentData.substr(p, p2 - p));
        p = p2 + 2;

        trimLeadingWhitespace(line);
        std::vector<std::string> headerLine = tokenize(line, ":", 2, false);
        if (headerLine.size() > 1) {
          std::string& key = headerLine[0];
          trimLeadingWhitespace(headerLine[1]);
          if (compare_nocase(key, "Host") == 0) {
            connection.host = line.c_str() + key.size() + 2;
          }
          else if (compare_nocase(key, "Content-Length") == 0) {
            connection.contentSize = (size_t)atoi(headerLine[1].c_str());
          }
          else {
            connection.header[key] = headerLine[1];
          }
        }
      }
      connection.headerComplete = true;
    }

    if (connection.headerComplete && !connection.requestComplete) {
      connection.bodyEnd = headerEnd + 4;

      if (connection.request != ePost && connection.request != ePut) {
        connection.requestComplete = true; // there is nothing after the header we care about
      }
      else {
        if (connection.contentSize) {
          headerEnd += 4;
          if (connection.currentData.size() - headerEnd >= connection.contentSize) {
            // read in that body!
            connection.body.append(connection.currentData.substr(headerEnd));
            connection.requestComplete = true;

            connection.bodyEnd += connection.contentSize;
          }
        }
        else {
          connection.requestComplete = true;
        }
      }
    }
  }

  if (connection.requestComplete) {
    // special, if it's a trace, just fire it back to them
    if (connection.request == eTrace) {
      bz_sendNonPlayerData(connectionID, connection.currentData.c_str(), (unsigned int)connection.bodyEnd);

      connection.currentData.erase(0, connection.bodyEnd);
      connection.flush();
    }
    else {
      // parse it all UP and build up a complete request
      connection.currentData.erase(0, connection.bodyEnd);

      HTTPRequest request;
      connection.fillRequest(request);
      // rip off what we need for the request, and then flush
      if (processRequest(request, connectionID)) { // the request closed, then just kill it
        return;
      }
      connection.flush();
    }

    // if there are lines to read
    if (connection.currentData.size() && find_first_substr(connection.currentData, "\r\n") != std::string::npos) {
      pending(connectionID, NULL, 0);
    }
  }
}

void HTTPServer::disconnect(int connectionID) {
  HTTPConnectionMap::iterator itr = liveConnections.find(connectionID);

  if (itr != liveConnections.end()) {
    liveConnections.erase(itr);
  }
}

void HTTPServer::update(void) {
  HTTPConnectionMap::iterator itr = liveConnections.begin();

  while (itr != liveConnections.end()) {
    if (itr->second.update()) {
      HTTPConnectionMap::iterator i = itr;
      itr++;
      bz_removeNonPlayerConnectionHandler(i->first, this);
      bz_disconnectNonPlayerConnection(i->first);
    }
    else {
      itr++;
    }
  }

  if (liveConnections.size()) {
    bz_setMaxWaitTime(0.01f, "HTTPServer");
  }
  else {
    bz_clearMaxWaitTime("HTTPServer");
  }
}

void HTTPServer::generateIndex(int connectionID, const HTTPRequest& request) {
  HTTPConnectionMap::iterator itr = liveConnections.find(connectionID);

  if (itr == liveConnections.end()) {
    return;
  }

  HTTPConnection& connection = itr->second;

  HTTPReply reply;

  reply.docType = HTTPReply::eHTML;
  reply.returnCode = HTTPReply::e200OK;
  reply.body = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"><html><head><title>Index page for " + baseURL + "</title></head>";
  reply.body += "<body>";

  VirtualDirs::iterator dirItr = virtualDirs.begin();

  while (dirItr != virtualDirs.end()) {
    if (!dirItr->second->hide()) {
      std::string vdirName = dirItr->second->getVDir();
      std::string vDirDescription = dirItr->second->getDescription();
      reply.body += "<a href=\"/" + vdirName + "/\">" + vdirName + "</a>&nbsp;" + vDirDescription + "<br>";
      dirItr++;
    }
  }

  reply.body += "</body></html>";

  connection.processingTasks.push_back(HTTPConnection::HTTPTask(reply, request.request == eHead));
  connection.update();
}

bool HTTPServer::generatePage(BZFSHTTP* vdir, int connectionID, HTTPRequest& request) {
  HTTPConnectionMap::iterator itr = liveConnections.find(connectionID);

  if (itr == liveConnections.end()) {
    return false;
  }

  HTTPConnection& connection = itr->second;

  HTTPReply reply;

  request.baseURL = baseURL;
  request.baseURL += vdir->getVDir();
  request.baseURL += "/";
  request.requestID = lastRequestID++;
  request.sessionID = connection.sessionID;

  if (vdir->handleRequest(request, reply)) {
    reply.cookies["SessionID"] = format("%d", request.sessionID);
    connection.processingTasks.push_back(HTTPConnection::HTTPTask(reply, request.request == eHead));
  }
  else {
    connection.pendingTasks.push_back(HTTPConnection::PendingHTTPTask(reply, request, request.request == eHead));
  }

  if (connection.update()) {
    connection.flush();
    bz_removeNonPlayerConnectionHandler(connectionID, this);
    bz_disconnectNonPlayerConnection(connectionID);
    liveConnections.erase(itr);
    return true;
  }

  return false;
}

bool HTTPServer::processRequest(HTTPRequest& request, int connectionID) {
  // check the request to see if it'll have any thing we care to process

  // find the vdir handler

  BZFSHTTP* vdir = NULL;

  VirtualDirs::iterator itr = virtualDirs.find(request.vdir);

  if (itr != virtualDirs.end()) {
    vdir = itr->second;
  }

  switch (request.request) {
    case ePut:
      if (!vdir || !vdir->supportPut()) {
        send403Error(connectionID);
        break;
      }
    case eHead:
    case eGet:
    case ePost:
      if (!vdir) {
        generateIndex(connectionID, request);
      }
      else {
        return generatePage(vdir, connectionID, request);
      }
      break;

    case eOptions:
      sendOptions(connectionID, vdir ? vdir->supportPut() : false);
      break;

    case eDelete:
    case eConnect:
      send501Error(connectionID);
      break;
  }

  return false;
}

void HTTPServer::send100Continue(int connectionID) {
  std::string httpHeaders;
  httpHeaders += "HTTP/1.1 100 Continue\n\n";

  bz_sendNonPlayerData(connectionID, httpHeaders.c_str(), (unsigned int)httpHeaders.size());
}

void HTTPServer::send403Error(int connectionID) {
  std::string httpHeaders;
  httpHeaders += "HTTP/1.1 403 Forbidden\n\n";

  bz_sendNonPlayerData(connectionID, httpHeaders.c_str(), (unsigned int)httpHeaders.size());
}

void HTTPServer::send404Error(int connectionID) {
  std::string httpHeaders;
  httpHeaders += "HTTP/1.1 404 Not Found\n\n";

  bz_sendNonPlayerData(connectionID, httpHeaders.c_str(), (unsigned int)httpHeaders.size());
}

void HTTPServer::send501Error(int connectionID) {
  std::string httpHeaders;
  httpHeaders += "HTTP/1.1 501 Not Implemented\n\n";

  bz_sendNonPlayerData(connectionID, httpHeaders.c_str(), (unsigned int)httpHeaders.size());
}

void HTTPServer::sendOptions(int connectionID, bool p) {
  std::string httpHeaders;
  httpHeaders += "HTTP/1.1 200 Ok\n";
  httpHeaders += "Allow: GET, HEAD, POST, OPTIONS";
  if (p) {
    httpHeaders += ", PUT";
  }
  httpHeaders += "\n\n";

  bz_sendNonPlayerData(connectionID, httpHeaders.c_str(), (unsigned int)httpHeaders.size());
}

void parseParams(std::map<std::string, std::vector<std::string> > &params, const std::string& text, size_t offset) {
  std::vector<std::string> items = tokenize(text, "&", 0, false, offset);

  for (size_t i = 0; i < items.size(); i++) {
    std::string& item = items[i];

    std::string key, val;

    std::vector<std::string> t = tokenize(item, "=", 0, false);
    if (t.size() > 1) {
      key = t[0];
      val = t[1];
    }
    else {
      key = item;
      val = "";
    }

    if (params.find(key) == params.end()) {
      std::vector<std::string> tv;
      params[key] = tv;
    }
    params[key].push_back(val);

  }
}

void HTTPConnection::fillRequest(HTTPRequest& req) {
  req.request = request;
  req.vdir = url_decode(vdir);
  req.resource = url_decode(resource);
  req.headers = header;
  req.cookies.clear();

  req.ip = bz_getNonPlayerConnectionIP(connectionID);
  const char* hostmask = bz_getNonPlayerConnectionHost(connectionID);
  req.hostmask = hostmask ? hostmask : "";

  // parse the headers here for cookies
  std::map<std::string, std::string>::iterator itr = req.headers.begin();

  while (itr != req.headers.end()) {
    const std::string& key = itr->first;
    if (compare_nocase(key, "cookie") == 0) {
      std::vector<std::string> cookie = tokenize(itr->second, "=", 2, false);

      if (cookie.size() > 1) {
        req.cookies[cookie[0]] = cookie[1];

        // check for the magic sessionID cookie
        if (compare_nocase(cookie[0], "sessionid") == 0) {
          sessionID = atoi(cookie[1].c_str());
        }
      }
    }
    else if (compare_nocase(key, "authorization") == 0) {
      std::vector<std::string> auth = tokenize(itr->second, " ", 2, false);

      if (auth.size() > 1) {
        req.authType = auth[0];
        req.authCredentials = auth[1];

        if (compare_nocase(auth[0], "basic") == 0 && auth[1].size()) {
          std::string b64 = base64_decode(auth[1]);
          std::vector<std::string> uandp = tokenize(b64, ":", 2, false);
          if (uandp.size() == 2) {
            req.username = uandp[0];
            req.password = uandp[1];
          }
        }
      }
    }

    itr++;
  }

  if (req.request != ePost) {
    // parse out the parameters from the resource
    size_t q = req.resource.find_first_of('?');
    if (q != std::string::npos) {
      parseParams(req.parameters, req.resource, q + 1);
      req.resource.erase(req.resource.begin() + q, req.resource.end());
    }
  }

  if (req.request == ePost && contentSize > 0) {
    parseParams(req.parameters, body, 0);
  }
  else if (req.request == ePut && contentSize > 0) {
    req.body = body;
  }
}

bool HTTPConnection::update(void) {
  // hit the processings
  std::vector<size_t> killList;

  bool closeConnect = false;

  for (size_t i = 0; i < processingTasks.size(); i++) {
    if (processingTasks[i].update(connectionID)) {
      if (processingTasks[i].forceClose) {
        closeConnect = true;
      }
      killList.push_back(i);
    }
  }

  std::vector<size_t>::reverse_iterator itr = killList.rbegin();
  while (itr != killList.rend()) {
    size_t offset = *itr;
    processingTasks.erase(processingTasks.begin() + offset);
    itr++;
  }

  // check the pending to see if they should be restarted
  std::vector<PendingHTTPTask>::iterator pendingItr = pendingTasks.begin();
  while (pendingTasks.size() && pendingItr != pendingTasks.end()) {
    PendingHTTPTask& pendingTask = *pendingItr;

    BZFSHTTP* vdir = NULL;

    VirtualDirs::iterator itr = virtualDirs.find(pendingTask.request.vdir);

    if (itr != virtualDirs.end()) {
      vdir = itr->second;
    }

    if (!vdir) {
      std::vector<PendingHTTPTask>::iterator t = pendingItr;
      t++;
      pendingTasks.erase(pendingItr);
      pendingItr = t;
    }
    else {
      if (vdir->resumeTask(pendingTask.request.requestID)) {
        if (vdir->handleRequest(pendingTask.request, pendingTask.reply)) {
          // if it is done and fire if off
          pendingTask.reply.cookies["SessionID"] = format("%d", pendingTask.request.sessionID);
          pendingTask.generateBody(pendingTask.reply, pendingTask.request.request == eHead);
          processingTasks.push_back(HTTPTask(pendingTask));

          std::vector<PendingHTTPTask>::iterator t = pendingItr;
          t++;
          pendingTasks.erase(pendingItr);
          pendingItr = t;
        }
        else {
          pendingItr++;
        }
      }
      else {
        pendingItr++;
      }
    }
  }

  return closeConnect;
}

const char* getMimeType(HTTPReply::DocumentType docType) {
  switch (docType) {
    case HTTPReply::eOctetStream:
      return "application/octet-stream";

    case HTTPReply::eBinary:
      return "application/binary";

    case HTTPReply::eHTML:
      return "text/html";

    case HTTPReply::eCSS:
      return "text/css";

    case HTTPReply::eXML:
      return "application/xml";

    case HTTPReply::eJSON:
      return "application/json";

    default:
      break;
  }
  return "text/plain";
}

HTTPConnection::HTTPTask::HTTPTask(HTTPReply& r, bool noBody): pos(0) {
  generateBody(r, noBody);
}

void HTTPConnection::HTTPTask::generateBody(HTTPReply& r, bool noBody) {
  // start a new one
  pageBuffer = "HTTP/1.1";

  forceClose = true;

  switch (r.returnCode) {
    case HTTPReply::e200OK:
      pageBuffer += " 200 OK\n";
      forceClose = false;
      break;

    case HTTPReply::e301Redirect:
      if (r.redirectLoc.size()) {
        pageBuffer += " 301 Moved Permanently\n";
        pageBuffer += "Location: " + r.redirectLoc + "\n";
      }
      else {
        pageBuffer += " 500 Server Error\n";
      }
      pageBuffer += "Host: " + serverHostname + "\n";

      break;

    case HTTPReply::e302Found:
      if (r.redirectLoc.size()) {
        pageBuffer += " 302 Found\n";
        pageBuffer += "Location: " + r.redirectLoc + "\n";
      }
      else {
        pageBuffer += " 500 Server Error\n";
      }

      pageBuffer += "Host: " + serverHostname + "\n";
      break;

    case HTTPReply::e500ServerError:
      pageBuffer += " 500 Server Error\n";
      break;

    case HTTPReply::e401Unauthorized:
      pageBuffer += " 401 Unauthorized\n";
      pageBuffer += "WWW-Authenticate: ";

      if (r.authType.size()) {
        pageBuffer += r.authType;
      }
      else {
        pageBuffer += "Basic";
      }

      pageBuffer += " realm=\"";
      if (r.authRealm.size()) {
        pageBuffer += r.authRealm;
      }
      else {
        pageBuffer += serverHostname;
      }
      pageBuffer += "\"\n";
      forceClose = false;
      break;

    case HTTPReply::e404NotFound:
      pageBuffer += " 404 Not Found\n";
      forceClose = false;
      break;

    case HTTPReply::e403Forbiden:
      pageBuffer += " 403 Forbidden\n";
      forceClose = false;
      break;
  }

  if (FORCE_CLOSE) {
    forceClose = true;
  }

  if (forceClose) {
    pageBuffer += "Connection: close\n";
  }

  if (r.getBodySize()) {
    pageBuffer += format("Content-Length: %d\n", r.getBodySize());

    pageBuffer += "Content-Type: ";
    if (r.docType == HTTPReply::eOther && r.otherMimeType.size()) {
      pageBuffer += r.otherMimeType;
    }
    else {
      pageBuffer += getMimeType(r.docType);
    }
    pageBuffer += "\n";
  }

  // write the cache info
  if (r.forceNoCache) {
    pageBuffer += "Cache-Control: no-cache\n";
  }

  if (r.md5.size()) {
    pageBuffer += "Content-MD5: " + r.md5 + "\n";
  }

  // dump the basic stat block
  pageBuffer += "Server: " + serverVersion + "\n";

  bz_Time ts;
  bz_getUTCtime(&ts);
  pageBuffer += "Date: ";
  pageBuffer += printTime(&ts, "UTC");
  pageBuffer += "\n";

  // dump the headers
  std::map<std::string, std::string>::iterator itr = r.headers.begin();

  while (itr != r.headers.end()) {
    pageBuffer += itr->first + ": " + itr->second + "\n";
    itr++;
  }

  if (r.returnCode == HTTPReply::e200OK) {
    itr = r.cookies.begin();
    while (itr != r.cookies.end()) {
      pageBuffer += "Set-Cookie: " + itr->first + "=" + itr->second + "\n";
      itr++;
    }
  }

  pageBuffer += "\n";

  if (!noBody && r.getBodySize()) {
    if (r.body.size()) {
      pageBuffer += r.body;
    }
    else { // it's bin data
      pageBuffer.append(r.getBody(), r.getBodySize());
    }
  }
}

HTTPConnection::HTTPTask::HTTPTask(const HTTPTask& t) {
  pageBuffer = t.pageBuffer;
  pos = t.pos;
}

bool HTTPConnection::HTTPTask::update(int connectionID) {
  // find out how much to write
  if (pos >= pageBuffer.size()) {
    return true;
  }

  // only send out another message if the buffer is nearing being empty, so we don't flood it out and
  // waste a lot of extra memory.
  int pendingMessages =  bz_getNonPlayerConnectionOutboundPacketCount(connectionID);
  if (pendingMessages > 1) {
    return false;
  }

  size_t write = 1000;
  size_t left = pageBuffer.size() - pos;

  if (left <= 1000) {
    write = left;
  }

  if (!bz_sendNonPlayerData(connectionID, pageBuffer.getData() + pos, (unsigned int)write)) {
    return true;
  }

  pos += write;

  return pos >= pageBuffer.size();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
