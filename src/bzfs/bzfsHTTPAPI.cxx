/* bzflag
* Copyright (c) 1993-2011 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

// common-interface headers
#include "global.h"

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <time.h>

// implementation wrapers for all the bzf_ API functions
#include "bzfsHTTPAPI.h"
#include "WorldEventManager.h"
#include "TextUtils.h"

bzhttp_eRequestType LineIsHTTPRequest ( const std::string & str )
{
  if (str == "GET")
    return eHTTPGet;
  else  if (str == "POST")
    return eHTTPPost;
  else if (str == "HEAD")
    return eHTTPHead;
  else if (str == "PUT")
    return eHTTPPut;
  else if (str == "DELETE")
    return eHTTPDelete;
  else if (str == "TRACE")
    return eHTTPTrace;
  else if (str == "OPTIONS")
    return eHTTPOptions;
  else  if (str == "CONNECT")
    return eHTTPConnect;

  return eHTTPUnknown;
}

class VDir
{
public:
  bz_Plugin *plugin;
  bzhttp_VDir *vdir;
  std::string name;
};

std::map<std::string,VDir> VDirs;

BZF_API bool bzhttp_RegisterVDir (bz_Plugin* plugin, bzhttp_VDir *vdir )
{
  if (!plugin || !vdir)
    return false;
  VDir dir;

  dir.name =  TextUtils::toupper(vdir->Name());
  if (VDirs.find(dir.name) != VDirs.end())
    return false;

  dir.plugin = plugin;
  dir.vdir = vdir;

  VDirs[dir.name] = dir;
  return true;
}

BZF_API bool bzhttp_RemoveVDir (bz_Plugin* plugin, bzhttp_VDir *vdir )
{
  if (!plugin || !vdir)
    return false;
  VDir dir;

  std::string name =  TextUtils::toupper(vdir->Name());

  std::map<std::string,VDir>::iterator itr = VDirs.find(dir.name);
  if (itr == VDirs.end())
    return false;

  if (itr->second.plugin != plugin)
    return false;

  VDirs.erase(itr);
  return true;
}

BZF_API bool bzhttp_RemoveAllVdirs (bz_Plugin* plugin )
{
  std::vector<std::string> names;
  std::map<std::string,VDir>::iterator itr = VDirs.begin();
  while (itr != VDirs.end())
  {
    if (itr->second.plugin == plugin)
      names.push_back(itr->second.name);
  }

  if (names.size() == 0)
    return false;

  for( size_t i = 0; i < names.size(); i++)
    VDirs.erase(VDirs.find(names[i]));

  return true;
}

void NewHTTPConnection ( bz_EventData *eventData );
void CheckForZombies  ( void );

class ConnectionEvent : public bz_EventHandler
{
public:
  ConnectionEvent()
  {
    plugin = NULL;
  }
  virtual void process ( bz_EventData *eventData )
  {
    if (eventData->eventType == bz_eTickEvent)
      CheckForZombies();
    else
      NewHTTPConnection(eventData);
  }
};

ConnectionEvent con;

//---- Responce----
class bzhttp_Responce_Data
{
public:
  std::map<std::string,std::string> Headers;
};

#define RESPONCE_DATA_PTR ((bzhttp_Responce_Data*)pimple)

#define RESPONCE_DATA(n) bzhttp_Responce_Data *n = ((bzhttp_Responce_Data*)pimple)


bzhttp_Responce::bzhttp_Responce()
{
  pimple = new bzhttp_Responce_Data;
}

bzhttp_Responce::~bzhttp_Responce()
{
  delete(pimple);
}


//---- Request----
class bzhttp_Request_Data
{
public:
  std::map<std::string,std::string> Headers;
  std::map<std::string,std::string> Paramaters;
};

#define REQUEST_DATA_PTR ((bzhttp_Request_Data*)pimple)

#define REQUEST_DATA(n) bzhttp_Request_Data *n = ((bzhttp_Request_Data*)pimple)

bzhttp_Request::bzhttp_Request()
{
  pimple = new bzhttp_Request_Data;
}

bzhttp_Request::~bzhttp_Request()
{
  delete(pimple);
}

void bzhttp_Request::AddHeader ( const char* n, const char* v)
{
  REQUEST_DATA(data);

  std::string name = TextUtils::toupper(n); 
  if (data->Headers.find(name) != data->Headers.end())
    data->Headers[name] = std::string(v);
}

const char* bzhttp_Request::GetHeader ( const char* n)
{
  REQUEST_DATA(data);

  std::string name = TextUtils::toupper(n); 
  if (data->Headers.find(name) == data->Headers.end())
    return NULL;

  return data->Headers[name].c_str();
}

const char* bzhttp_Request::GetHeader ( size_t index )
{
  REQUEST_DATA(data);
  if (index >= data->Headers.size())
    return NULL;

  size_t i =0;
  std::map<std::string,std::string>::iterator itr = data->Headers.begin();
  while (itr != data->Headers.end())
  {
    if (index == i)
      return itr->second.c_str();
    i++;
    itr++;
  }
  return NULL;
}

size_t bzhttp_Request::GetHeaderCount ()
{
  REQUEST_DATA(data);
  return data->Headers.size();
}

void bzhttp_Request::AddParamater ( const char* n, const char* v)
{
  REQUEST_DATA(data);

  std::string name = TextUtils::toupper(n); 
  if (data->Paramaters.find(name) != data->Paramaters.end())
    data->Paramaters[name] = std::string(v);
}

const char* bzhttp_Request::GetParamater ( const char* n)
{
  REQUEST_DATA(data);

  std::string name = TextUtils::toupper(n); 
  if (data->Paramaters.find(name) == data->Paramaters.end())
    return NULL;

  return data->Paramaters[name].c_str();
}

const char* bzhttp_Request::GetParamater ( size_t index )
{
  REQUEST_DATA(data);
  if (index >= data->Paramaters.size())
    return NULL;

  size_t i =0;
  std::map<std::string,std::string>::iterator itr = data->Paramaters.begin();
  while (itr != data->Paramaters.end())
  {
    if (index == i)
      return itr->second.c_str();
    i++;
    itr++;
  }
  return NULL;
}

size_t bzhttp_Request::GetParamaterCount ()
{
  REQUEST_DATA(data);
  return data->Paramaters.size();
}

class HTTPConnectedPeer : public bz_NonPlayerConnectionHandler
{
public:
  bool Killme;
  bzhttp_VDir *vDir;

  std::string Resource;
  std::string HTTPVersion;

  size_t ContentSize,HeaderSize;

  bzhttp_Request  Request;

  HTTPConnectedPeer() : bz_NonPlayerConnectionHandler()
  {
    Killme = false;
    vDir = NULL;
    Request.RequestType = eHTTPUnknown;
    ContentSize = 0;
    HeaderSize = 0;
  }

  virtual void pending(int connectionID, void *data, unsigned int size)
  {
    if (!vDir)
      return;

    char *d = (char*)malloc(size+1);
    memcpy(d,data,size);
    d[size] = 0;
    RequestData += d;
    free(d);

    if (Request.RequestType == eHTTPUnknown)
    {
      std::stringstream stream(RequestData);

      std::string request, httpVersion;
      stream >> request >> Resource >> httpVersion;

      Request.RequestType = LineIsHTTPRequest(request);
    }

    // easy outs
    if (Request.RequestType == eHTTPDelete || Request.RequestType == eHTTPConnect || (Request.RequestType == eHTTPPut && !vDir->SupportPut()))
    {
      send501Error(connectionID);
      return;
    }

    if (Request.RequestType == eHTTPOptions)
    {
      sendOptions(connectionID,vDir->SupportPut());
      return;
    }

    // see if we want to get it

    bool requestComplete = false;
    if (HeaderSize == 0)
    {
      size_t headerEnd = TextUtils::find_first_substr(RequestData,"\r\n\r\n");
      if (headerEnd != std::string::npos)
	HeaderSize = headerEnd + 4;
    }

    if (HeaderSize && !Request.GetHeaderCount())
    {
      std::string headerSubStir = RequestData.substr(0,HeaderSize);
      std::vector<std::string> headers = TextUtils::tokenize(headerSubStir,"\r\n");

      if (headers.size() > 0)
      {
	std::string browserInfo = headers[0];
      }

      for ( size_t i = 1; i < headers.size(); i++ )
      {
	std::vector<std::string> headerParts = TextUtils::tokenize(headers[i],":",2);
	if (headerParts.size() > 1)
	{
	  Request.AddHeader(headerParts[0].c_str(),headerParts[1].c_str());
	  std::string name = TextUtils::toupper(headerParts[0]);
	  if (name == "CONTENT-LENGTH")
	  {
	    int size = atoi(headerParts[1].c_str());
	    if (size > 0)
	      ContentSize = (unsigned int)size;
	    else
	      ContentSize = 0;
	  }
	}
      }
    }

    if (HeaderSize && (Request.RequestType == eHTTPGet || Request.RequestType == eHTTPHead))
      requestComplete = true;
    else
    {
      if (ContentSize)
      {
	if (RequestData.size() >= (ContentSize+HeaderSize))
	  requestComplete = true;
      }
    }

    if (requestComplete)
    {
      // parse up the paramaters;

      size_t question = Resource.find_first_of('?');

      std::string resource = Resource;

      if (question != std::string::npos)
      {
	parseParams(Resource.substr(question+1,Resource.length()-(question+1)));
	resource = Resource.substr(0,question);
      }

      Request.URL = Resource;
      Request.Resource = resource;

      if (Request.RequestType == eHTTPPost)
      {
	std::string body = RequestData.substr(HeaderSize,ContentSize);
	Request.Body = body;

	// parse the body for params
	parseParams(body);
      }
      else if (Request.RequestType == eHTTPPut)
      {
	Request.Body = RequestData.substr(HeaderSize,ContentSize);
      }

      bzhttp_Responce responce;

      // let the vdir do it's thing
      if (!vDir->GeneratePage(Request,responce))
	send404Error(connectionID);

      Killme = true;
    }
    else
    {
      if (Request.RequestType == eHTTPPost || Request.RequestType == eHTTPPut)
	send100Continue(connectionID);
    }
  }

  void parseParams ( const std::string & str )
  {
    std::vector<std::string> v = TextUtils::tokenize(str,"&");
    for ( size_t i = 0; i < v.size(); i++ )
    {
      if (v[i].size())
      {
	std::vector<std::string> t = TextUtils::tokenize(v[i],"=");
	if (t.size())
	  Request.AddParamater(TextUtils::url_decode(t[0]).c_str(), t.size() > 1 ? TextUtils::url_decode(t[1]).c_str() : "");
      }
    }
  }

  virtual void disconnect(int connectionID)
  {
    if (connectionID)
      Killme = true;
  }

  void send100Continue(int connectionID)
  {
    std::string httpHeaders;
    httpHeaders += "HTTP/1.1 100 Continue\n\n";

    bz_sendNonPlayerData(connectionID, httpHeaders.c_str(), (unsigned int)httpHeaders.size());
  }

  void send404Error(int connectionID)
  {
    std::string httpHeaders;
    httpHeaders += "HTTP/1.1 404 Not Found\n\n";

    bz_sendNonPlayerData(connectionID, httpHeaders.c_str(), (unsigned int)httpHeaders.size());
  }

  void send501Error(int connectionID)
  {
    std::string httpHeaders;
    httpHeaders += "HTTP/1.1 501 Not Implemented\n\n";

    bz_sendNonPlayerData(connectionID, httpHeaders.c_str(), (unsigned int)httpHeaders.size());
    Killme = true;
  }

  void sendOptions(int connectionID, bool put)
  {
    std::string httpHeaders;
    httpHeaders += "HTTP/1.1 200 Ok\n";
    httpHeaders += "Allow: GET, HEAD, POST, OPTIONS";
    if (put)
      httpHeaders += ", PUT";
    httpHeaders += "\n\n";

    bz_sendNonPlayerData(connectionID, httpHeaders.c_str(), (unsigned int)httpHeaders.size());
    Killme = true;
  }

protected:
  std::string RequestData;
};

std::map<int,HTTPConnectedPeer*> HTTPPeers;

// index handler
class HTTPIndexHandler: public bzhttp_VDir
{
public:
  virtual const char* Name(){return "INDEX";}

  virtual bool GeneratePage ( const bzhttp_Request& request, bzhttp_Responce &responce )
  {
    return true;
  }
};

HTTPIndexHandler *indexHandler;

void InitHTTP()
{
  indexHandler = new HTTPIndexHandler();
  VDirs.clear();
  worldEventManager.addEvent(bz_eNewNonPlayerConnection,&con);
  worldEventManager.addEvent(bz_eTickEvent,&con);

}

void KillHTTP()
{
  std::map<int,HTTPConnectedPeer*>::iterator itr = HTTPPeers.begin();
  while (itr != HTTPPeers.end())
  {
    bz_disconnectNonPlayerConnection(itr->first);
    delete(itr->second);
    itr++;
  }

  HTTPPeers.clear();
  worldEventManager.removeEvent(bz_eTickEvent,&con);
  worldEventManager.removeEvent(bz_eNewNonPlayerConnection,&con);
 VDirs.clear();
  delete(indexHandler);
}

void NewHTTPConnection ( bz_EventData *eventData )
{
//  if (VDirs.size() == 0)
//    return;

  bz_NewNonPlayerConnectionEventData_V1 *connData = (bz_NewNonPlayerConnectionEventData_V1*)eventData;

  if (!connData->size)
    return;

  const char* data = (char*)connData->data;
  std::string sentData = data;

  std::stringstream stream(sentData);

  std::string request, resource, httpVersion;
  stream >> request >> resource >> httpVersion;

  if (!request.size() || !resource.size() || !httpVersion.size())
    return;

  bzhttp_eRequestType requestType = LineIsHTTPRequest(TextUtils::toupper(request));

  if (requestType == eHTTPUnknown)
    return;

  if (httpVersion != "HTTP/1.1" && httpVersion != "HTTP/1.0")
    return;

  // figure out who gets it
  // count the /s if there is more then one then the first part is the vdir
  // otherwise it's an index request
  bzhttp_VDir * dir = NULL;


  if (resource.size() > 0)
  {
    // trim it at the ? if it has one
    std::string subResource = resource;
    size_t question = subResource.find_first_of('?');
    if (question != std::string::npos)
      subResource = resource.substr(0,question);

    // find the first / ( really this should be at 0 )
    size_t firstSlash = subResource.find_first_of('/');
    if (firstSlash != std::string::npos && firstSlash == 0)
    {
      std::string dirName;

      size_t secondSlash = subResource.find_first_of('/',firstSlash+1);
      if (secondSlash != std::string::npos)
	dirName = subResource.substr(firstSlash+1,secondSlash-firstSlash-1);
      else
      {
	 if (firstSlash+1 < subResource.size())
	   dirName = subResource.substr(firstSlash+1,subResource.size()-firstSlash);
      }

      if (dirName.size())
      {
	std::map<std::string,VDir>::iterator itr = VDirs.find(TextUtils::toupper(dirName));
	if (itr != VDirs.end())
	{
	  dir = itr->second.vdir;
	}
      }
    }

    HTTPConnectedPeer *peer = new HTTPConnectedPeer();
    if (!dir)
      peer->vDir = indexHandler;
    else
      peer->vDir = dir;

    bz_registerNonPlayerConnectionHandler(connData->connectionID,peer);
    
    HTTPPeers[connData->connectionID] = peer;
    peer->pending(connData->connectionID,connData->data,connData->size);
  }
  //connData->data
}

void CheckForZombies ( void )
{
  std::map<int,HTTPConnectedPeer*>::iterator itr = HTTPPeers.end();
  std::vector<int> toKill;

  while (itr != HTTPPeers.end())
  {
    if (itr->second->Killme)
    {
      bz_disconnectNonPlayerConnection(itr->first);
      delete(itr->second);
      toKill.push_back(itr->first);
    }
  }

  for (size_t i = 0; i < toKill.size(); i++)
    HTTPPeers.erase(HTTPPeers.find(toKill[i]));
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
