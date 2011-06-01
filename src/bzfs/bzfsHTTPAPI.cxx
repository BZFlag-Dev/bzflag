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

std::string ServerVersion;
std::string ServerHostname;
std::string BaseURL;

#ifdef _WIN32
#define _DirDelim '\\'
#else
#define _DirDelim '/'
#endif

// ensures all the delims are constant
std::string convertPathToDelims(const char* file)
{
  if (!file)
    return std::string();

  std::string delim;
  delim += _DirDelim;
  return TextUtils::replace_all(TextUtils::replace_all(file,"/",delim),"\\",delim);
}

std::string getPathForOS(const char* file)
{
  return convertPathToDelims(file);
}

std::string concatPaths ( const char* path1, const char* path2 )
{
  std::string ret = getPathForOS(path1);
  ret += getPathForOS(path2);

  return ret;
}

bool fileExits ( const char* c)
{
  if (!c)
    return false;

  FILE* fp = fopen(c,"rb");
  if (!fp)
    return false;
  fclose(fp);
  return true;
}

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

//----bzhttp_VDir

class bzhttp_VDir_Data
{
public:
  std::map<std::string,std::string> MimeTypes;
};

#define VDIR_DATA_PTR ((bzhttp_VDir_Data*)pimple)
#define VDIR_DATA(n) bzhttp_VDir_Data *n = ((bzhttp_VDir_Data*)pimple)
#define VDIR_DATA_CLASS(c,n) bzhttp_VDir_Data *n = ((bzhttp_VDir_Data*)(c))

bzhttp_VDir::bzhttp_VDir()
{
  pimple = new bzhttp_VDir_Data;
}

bzhttp_VDir::~bzhttp_VDir()
{
  delete(pimple);
}

void bzhttp_VDir::AddMimeType(const char* e, const char* m )
{
  VDIR_DATA(data);

  std::string ext = TextUtils::toupper(e);

  if (data->MimeTypes.find(ext) == data->MimeTypes.end())
    data->MimeTypes[ext] = std::string(m);
}

void bzhttp_VDir::AddStandardTypes ()
{
  AddMimeType("htm","text/html");
  AddMimeType("html","text/html");
  AddMimeType("txt","text/plain");
  AddMimeType("png","image/png");
  AddMimeType("ico","image/vnd.microsoft.icon");
  AddMimeType("jpg","image/jpeg");
  AddMimeType("jpeg","image/jpeg");
  AddMimeType("gif","image/gif");
  AddMimeType("js","application/javascript");
  AddMimeType("json","application/json");
}

//----VDir
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

  vdir->BaseURL = BaseURL + dir.name + "/";

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
  std::map<std::string,std::string> Cookies;
  std::string Body;
};

#define RESPONCE_DATA_PTR ((bzhttp_Responce_Data*)pimple)

#define RESPONCE_DATA(n) bzhttp_Responce_Data *n = ((bzhttp_Responce_Data*)pimple)

#define RESPONCE_DATA_CLASS(c,n) bzhttp_Responce_Data *n = ((bzhttp_Responce_Data*)(c))

bzhttp_Responce::bzhttp_Responce()
{
  pimple = new bzhttp_Responce_Data;
  ReturnCode = e404NotFound;
  DocumentType = eHTML;
  ForceNoCache = false;
}

bzhttp_Responce::~bzhttp_Responce()
{
  delete(pimple);
}

void bzhttp_Responce::AddHeader ( const char* n, const char* v)
{
  RESPONCE_DATA(data);

  std::string name = n; 
  if (data->Headers.find(name) != data->Headers.end())
    data->Headers[name] = std::string(v);
}

void bzhttp_Responce::AddCookies ( const char* n, const char* v)
{
  RESPONCE_DATA(data);

  std::string name = n; 
  if (data->Cookies.find(name) != data->Cookies.end())
    data->Cookies[name] = std::string(v);
}

void bzhttp_Responce::AddBodyData ( const char* v)
{
  RESPONCE_DATA(data);

  if (v)
    data->Body += v;
}

void bzhttp_Responce::AddBodyData ( const void* v, size_t size)
{
  RESPONCE_DATA(data);

  if (v && size)
    data->Body += std::string((char*)v,size);
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

  bool RequestComplete;

  bzhttp_Request  Request;
  bzhttp_Responce Responce;

  HTTPConnectedPeer() : bz_NonPlayerConnectionHandler()
  {
    RequestComplete = false;
    Killme = false;
    vDir = NULL;
    Request.RequestType = eHTTPUnknown;
    ContentSize = 0;
    HeaderSize = 0;
  }

  virtual void pending(int connectionID, void *data, unsigned int size)
  {
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
    if (Request.RequestType == eHTTPDelete || Request.RequestType == eHTTPConnect || (Request.RequestType == eHTTPPut && (vDir && !vDir->SupportPut())))
    {
      send501Error(connectionID);
      return;
    }

    if (Request.RequestType == eHTTPOptions)
    {
      sendOptions(connectionID,vDir ? vDir->SupportPut() : false);
      return;
    }

    // see if we want to get it

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
      RequestComplete = true;
    else
    {
      if (ContentSize)
      {
	if (RequestData.size() >= (ContentSize+HeaderSize))
	  RequestComplete = true;
      }
    }

    if (RequestComplete)
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

      Think(connectionID);
    }
    else
    {
      if (Request.RequestType == eHTTPPost || Request.RequestType == eHTTPPut)
	send100Continue(connectionID);
    }
  }

  virtual bzhttp_ePageGenStatus GetPage () 
  {
    if(!vDir)
      return eNoPage;

    return vDir->GeneratePage(Request,Responce);
  }

  void Think ( int connectionID )
  {
    if (RequestComplete && !Killme)
    {
      bzhttp_ePageGenStatus status = GetPage();

      if (status == eNoPage)
	send404Error(connectionID);
      else if (status == ePageDone)
	GenerateResponce(connectionID,Responce);
    }
  }

  const char* getMimeType(bzhttp_eDocumentType docType)
  {
    switch(docType) {
      case eOctetStream:
	return "application/octet-stream";

      case eBinary:
	return "application/binary";

      case eHTML:
	return "text/html";

      case eCSS:
	return "text/css";

      case eXML:
	return "application/xml";

      case eJSON:
	return "application/json";

      default:
	break;
    }
    return "text/plain";
  }


  void appendTime(std::string & text, bz_Time *ts, const char* _zoneofTime)
  {
    switch(ts->dayofweek) {
  case 1:
    text += "Mon";
    break;
  case 2:
    text += "Tue";
    break;
  case 3:
    text += "Wed";
    break;
  case 4:
    text += "Thu";
    break;
  case 5:
    text += "Fri";
    break;
  case 6:
    text += "Sat";
    break;
  case 0:
    text += "Sun";
    break;
    }

    text += TextUtils::format(", %d ",ts->day);

    switch(ts->month) {
  case 0:
    text += "Jan";
    break;
  case 1:
    text += "Feb";
    break;
  case 2:
    text += "Mar";
    break;
  case 3:
    text += "Apr";
    break;
  case 4:
    text += "May";
    break;
  case 5:
    text += "Jun";
    break;
  case 6:
    text += "Jul";
    break;
  case 7:
    text += "Aug";
    break;
  case 8:
    text += "Sep";
    break;
  case 9:
    text += "Oct";
    break;
  case 10:
    text += "Nov";
    break;
  case 11:
    text += "Dec";
    break;
    }

    text += TextUtils::format(" %d %d:%d:%d ",ts->year,ts->hour,ts->minute,ts->second);
    if (_zoneofTime)
      text += _zoneofTime;
    else
      text += "GMT";
  }

  std::string printTime(bz_Time *ts, const char* _zoneofTime)
  {
    std::string time;
    appendTime(time,ts,_zoneofTime);
    return time;
  }

  void GenerateResponce (int connectionID, const bzhttp_Responce & responce )
  {
    RESPONCE_DATA_CLASS(Responce.pimple,data);

    std::string pageBuffer = "HTTP/1.1";

    switch (responce.ReturnCode)
    {
      case e200OK:
	pageBuffer += " 200 OK\n";
	break;
      case e301Redirect:
	if (responce.RedirectLocation.size())
	{
	  pageBuffer += " 301 Moved Permanently\n";
	  pageBuffer += "Location: " + std::string(responce.RedirectLocation.c_str()) + "\n";
	}
	else
	  pageBuffer += " 500 Server Error\n";
	pageBuffer += "Host: " + ServerHostname + "\n";
	break;

      case e302Found:
	if (responce.RedirectLocation.size())
	{
	  pageBuffer += " 302 Found\n";
	  pageBuffer += "Location: " + std::string(responce.RedirectLocation.c_str()) + "\n";
	}
	else
	  pageBuffer += " 500 Server Error\n";

	pageBuffer += "Host: " + ServerHostname + "\n";
	break;

      case e403Forbiden:
	pageBuffer += " 403 Forbidden\n";
	break;

      case e404NotFound:
	pageBuffer += " 404 Not Found\n";
	break;

      case e418IAmATeapot:
	pageBuffer += " 418 I Am A Teapot\n";
	break;

      case e500ServerError:
	pageBuffer += " 500 Server Error\n";
	break;
    }

    pageBuffer += "Connection: close\n";

    if (data->Body.size())
    {
      pageBuffer += TextUtils::format("Content-Length: %d\n", data->Body.size());

      pageBuffer += "Content-Type: ";
      if (responce.DocumentType == eOther && responce.MimeType.size())
	pageBuffer += responce.MimeType.c_str();
      else
	pageBuffer += getMimeType(responce.DocumentType);
      pageBuffer += "\n";
    }

    if (responce.ForceNoCache)
      pageBuffer += "Cache-Control: no-cache\n";

    if (responce.MD5Hash.size())
      pageBuffer += "Content-MD5: " + std::string(responce.MD5Hash.c_str()) + "\n";

    pageBuffer += "Server: " + ServerVersion + "\n";

    bz_Time ts;
    bz_getUTCtime (&ts);
    pageBuffer += "Date: ";
    pageBuffer += printTime(&ts,"UTC");
    pageBuffer += "\n";

    std::map<std::string,std::string>::iterator itr = data->Headers.begin();
    while (itr != data->Headers.end())
    {
      pageBuffer += itr->first + ":" + itr->second + "\n";
      itr++;
    }

    if (responce.ReturnCode == e200OK)
    {
      itr = data->Cookies.begin();
      while (itr != data->Cookies.end())
      {
	pageBuffer +=  "Set-Cookie: " + itr->first + ":" + itr->second + "\n";
	itr++;
      }
    }

     pageBuffer += "\n";

     if (Request.RequestType != eHTTPHead && data->Body.size())
      pageBuffer += data->Body;


     // build up the buffers
     size_t i = 0;
     size_t bufferSize = 1024;

     while ( i < pageBuffer.size())
     {
       size_t thisBuffer = bufferSize;
       if (i + thisBuffer >= pageBuffer.size())
	 thisBuffer = pageBuffer.size()-i;

       bz_sendNonPlayerData(connectionID, pageBuffer.c_str() + i, thisBuffer);
       i += thisBuffer;
     }

    Killme = true;
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

class ResourcePeer : public HTTPConnectedPeer
{
public:
  std::string File;
  std::string Mime;

  ResourcePeer() : HTTPConnectedPeer()
  {
    Mime = "application/octet-stream";
  }

  virtual bzhttp_ePageGenStatus GetPage () 
  {
    if (!File.size())
      return eNoPage;

    FILE* fp = fopen(File.c_str(),"rb");
    if (!fp)
      return eNoPage;

    size_t size = 0;
    fseek(fp,0,SEEK_END);
    size = ftell(fp);
    fseek(fp,0,SEEK_SET);
    void *p = malloc(size);
    fread(p,size,1,fp);
    fclose(fp);

    Responce.AddBodyData(p,size);
    free(p);
    Responce.ReturnCode = e200OK;
    Responce.DocumentType = eOther;
    Responce.MimeType = Mime.c_str();

    return ePageDone;
  }

  static std::string FindResourcePath ( const std::string &file, const bz_APIStringList &dirs )
  {
    for ( size_t i = 0; i < dirs.size(); i++)
    {
      std::string path = concatPaths(dirs.get(i).c_str(),file.c_str());
      if (fileExits(path.c_str()))
	return path;
    }
    return std::string();
  }
};

std::map<int,HTTPConnectedPeer*> HTTPPeers;

// index handler
class HTTPIndexHandler: public bzhttp_VDir
{
public:
  HTTPIndexHandler() : bzhttp_VDir()
  {
    if (bz_BZDBItemExists("_HTTPIndexResourceDir") && bz_getBZDBString("_HTTPIndexResourceDir").size())
      ResourceDirs.push_back(bz_getBZDBString("_HTTPIndexResourceDir"));

    AddStandardTypes();
  }

  virtual const char* Name(){return "INDEX";}

  virtual bzhttp_ePageGenStatus GeneratePage ( const bzhttp_Request& request, bzhttp_Responce &responce )
  {
    responce.AddBodyData("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"><html><head>");
    responce.AddBodyData("<title>Index page for ");
    responce.AddBodyData(ServerHostname.c_str());
    responce.AddBodyData("</title></head><body>");

    if (VDirs.size() == 0)
     responce.AddBodyData("No HTTP Services are running on this server");
    else
    {
      std::map<std::string,VDir>::iterator itr = VDirs.begin();
      {
	std::string vdirName = itr->second.vdir->Name();
	std::string vDirDescription = itr->second.vdir->Description();
	std::string line =  "<a href=\"/" + vdirName + "/\">" + vdirName +"</a>&nbsp;" +vDirDescription +"<br/>";
	responce.AddBodyData(line.c_str());
      }
    }

    responce.AddBodyData("</body></html>");

    return ePageDone;
  }

  virtual bool AllowResourceDownloads ( void )
  {
    return ResourceDirs.size() > 0;
  }
};

HTTPIndexHandler *indexHandler;

void InitHTTP()
{
  indexHandler = new HTTPIndexHandler();
  VDirs.clear();
  worldEventManager.addEvent(bz_eNewNonPlayerConnection,&con);
  worldEventManager.addEvent(bz_eTickEvent,&con);

  ServerHostname = "localhost";
  if (bz_getPublicAddr().size())
    ServerHostname = bz_getPublicAddr().c_str();

  // make sure it has the port
  if (strrchr(ServerHostname.c_str(),':') == NULL)
    ServerHostname += TextUtils::format(":%d",bz_getPublicPort());

  ServerVersion = bz_getServerVersion();

  BaseURL = "http://";
  BaseURL += ServerHostname +"/";

  indexHandler->BaseURL = BaseURL.c_str();
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
  //if (VDirs.size() == 0)
 //   return;

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
    size_t question = resource.find_first_of('?');
    if (question != std::string::npos)
      resource = resource.substr(0,question);

    // find the first / ( really this should be at 0 )
    size_t firstSlash = resource.find_first_of('/');
    if (firstSlash != std::string::npos && firstSlash == 0)
    {
      std::string dirName;

      size_t secondSlash = resource.find_first_of('/',firstSlash+1);
      if (secondSlash != std::string::npos)
	dirName = resource.substr(firstSlash+1,secondSlash-firstSlash-1);
      else
      {
	 if (firstSlash+1 < resource.size())
	   dirName = resource.substr(firstSlash+1,resource.size()-firstSlash);
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

    // check and see if it's a resource 

    if (peer->vDir->AllowResourceDownloads())
    {
      if (resource.size())
      {
	size_t dot = resource.find_last_of('.');
	if (dot != std::string::npos)
	{
	  std::string path = resource.substr(0,dot);
	  if (TextUtils::find_first_substr(path,"..") == std::string::npos) // don't do paths that have .. ANYWHERE
	  {
	    std::string ext = TextUtils::toupper(resource.substr(dot+1,resource.size()-dot-1));
	    VDIR_DATA_CLASS(peer->vDir->pimple,vdata);
	    if (vdata->MimeTypes.find(ext) != vdata->MimeTypes.end())
	    {
	      ResourcePeer *p = new ResourcePeer();
	      p->File = ResourcePeer::FindResourcePath(resource,peer->vDir->ResourceDirs);
	      p->Mime = vdata->MimeTypes[ext];
	      delete(peer);
	      peer = p;
	    }
	  }
	}
      }
    }

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
      if ( bz_getNonPlayerConnectionOutboundPacketCount(itr->first))
      {
	bz_disconnectNonPlayerConnection(itr->first);
	delete(itr->second);
	toKill.push_back(itr->first);
      }
    }
    else
      itr->second->Think(itr->first);
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
