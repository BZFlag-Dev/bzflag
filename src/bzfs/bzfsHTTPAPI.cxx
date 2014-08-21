/* bzflag
 * Copyright (c) 1993-2013 Tim Riker
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
#include <cstdlib>

// implementation wrapers for all the bzf_ API functions
#include "bzfsHTTPAPI.h"
#include "WorldEventManager.h"
#include "TextUtils.h"
#include "TimeKeeper.h"
#include "base64.h"
#include "Permissions.h"

// only include this if we're going to need plugins
#ifdef BZ_PLUGINS
#include "bzfsPlugins.h"
#endif

std::string ServerVersion;
std::string ServerHostname;
std::string BaseURL;

#ifdef _WIN32
#define _DirDelim '\\'
#else
#define _DirDelim '/'
#endif

#define SESSION_COOKIE "BZFS_SESSION_ID"

std::string trimLeadingWhitespace(const char* t)
{
  std::string text;
  while(*t) {
    if (!TextUtils::isWhitespace(*t))
    {
     text = t;
     break;
    }
    t++;
  }
  return text;
}

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

//-- session data

class bzhttp_SessionData_Data
{
public:
  std::map<std::string,std::string> GlobalData;
  std::map<std::string,std::map<std::string,std::string> > PrivateData;
  std::string CurrentVdir;
  double LastUpdateTime;

  std::string IP;

  bzhttp_SessionData_Data()
  {
    LastUpdateTime = 0;
  }
};

#define VDIR_SESSION_PTR (static_cast<bzhttp_SessionData_Data*>(pimple))
#define VDIR_SESSION(n) bzhttp_SessionData_Data *n = VDIR_SESSION_PTR
#define VDIR_SESSION_CLASS(c,n) bzhttp_SessionData_Data *n = ((bzhttp_SessionData_Data*)(c))

bzhttp_SessionData::bzhttp_SessionData()
{
  pimple = new bzhttp_SessionData_Data();

  VDIR_SESSION(d);
  d->PrivateData[std::string("")] = std::map<std::string,std::string>();
}

bzhttp_SessionData::~bzhttp_SessionData()
{
  delete VDIR_SESSION_PTR;
}
std::map<std::string,std::string> emptyMap;

std::map<std::string,std::string> &GetPrivateData ( bzhttp_SessionData_Data * data )
{
  if (!data || data->CurrentVdir == "" || data->PrivateData.find(data->CurrentVdir) == data->PrivateData.end())
    return emptyMap;

  return data->PrivateData[data->CurrentVdir];
}

std::map<std::string,std::string> & GetServerSessionData ( bzhttp_SessionData &d )
{
  VDIR_SESSION_CLASS(d.pimple,p);
  return p->PrivateData[std::string("")];
}

const char* bzhttp_SessionData::GetPrivateItem ( const char* name )
{
  std::map<std::string,std::string> &items = GetPrivateData((bzhttp_SessionData_Data *)pimple);

  std::map<std::string,std::string>::iterator itr = items.find(TextUtils::toupper(name));
  if (itr != items.end())
    return itr->second.c_str();
  return NULL;
}

void bzhttp_SessionData::SetPrivateItem ( const char * n, const char* value )
{
  std::map<std::string,std::string> &items = GetPrivateData((bzhttp_SessionData_Data *)pimple);
  std::string name = TextUtils::toupper(n);

  std::map<std::string,std::string>::iterator itr = items.find(name);
  if (itr != items.end())
    return;

  items[name] = std::string(value);
}

void bzhttp_SessionData::ClearPrivateItem ( const char * name )
{
  std::map<std::string,std::string> &items = GetPrivateData((bzhttp_SessionData_Data *)pimple);
  std::map<std::string,std::string>::iterator itr = items.find(TextUtils::toupper(name));
  if (itr != items.end())
    items.erase(itr);
}

void bzhttp_SessionData::FlushPrivateItems ( void )
{
  std::map<std::string,std::string> &items = GetPrivateData((bzhttp_SessionData_Data *)pimple);
  items.clear();
}

const char* bzhttp_SessionData::GetGlobalItem ( const char* name )
{
  VDIR_SESSION(data);
  std::map<std::string,std::string>::iterator itr = data->GlobalData.find(TextUtils::toupper(name));
  if (itr != data->GlobalData.end())
    return itr->second.c_str();
  return NULL;
}

void bzhttp_SessionData::SetGlobalItem ( const char * n, const char* value )
{
  VDIR_SESSION(data);
  std::string name = TextUtils::toupper(n);

  std::map<std::string,std::string>::iterator itr = data->GlobalData.find(name);
  if (itr != data->GlobalData.end())
    return;

  data->GlobalData[name] = std::string(value);
}

void bzhttp_SessionData::ClearGlobalItem ( const char * name )
{
  VDIR_SESSION(data);
  std::map<std::string,std::string>::iterator itr = data->GlobalData.find(TextUtils::toupper(name));
  if (itr != data->GlobalData.end())
    data->GlobalData.erase(itr);
}

std::map<int,bzhttp_SessionData*> Sessions;

bzhttp_SessionData& GetSession ( unsigned int id, const char* ip )
{
  if (id < 10 || Sessions.find(id) == Sessions.end())
  {
    bzhttp_SessionData *newSession = new bzhttp_SessionData();
    unsigned int newID = rand();
    while (Sessions.find(newID) != Sessions.end() && newID < 10)
      newID = rand();

    newSession->SessionID = newID;
    VDIR_SESSION_CLASS(newSession->pimple,data);
    data->LastUpdateTime = TimeKeeper::getCurrent().getSeconds();
    data->IP = ip;
    Sessions[newID] = newSession;

    return *Sessions[newID];
  }
  else
  {
    VDIR_SESSION_CLASS(Sessions[id]->pimple,data);
    data->LastUpdateTime = TimeKeeper::getCurrent().getSeconds();
    data->IP = ip;
    return *Sessions[id];
  }
}

bzhttp_SessionData& GetSessionByID ( const char *_id, const char* ip )
{
  if (!_id || !ip)
    return GetSession(0,ip);

  unsigned int id = atoi(_id);

  if (id < 10 || Sessions.find(id) == Sessions.end())
    return GetSession(0,ip);

  VDIR_SESSION_CLASS(Sessions[id]->pimple,data);
  if (data->IP == ip)
  {
    data->LastUpdateTime = TimeKeeper::getCurrent().getSeconds();
    data->IP = ip;
    return *Sessions[id];
  }
  return GetSession(0,ip);
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
  RequiredAuthentiction = eNoAuth;
  CacheAuthentication = false;
  MaxRequestSize = -1;
  MaxRequestBody = -1;
}

bzhttp_VDir::~bzhttp_VDir()
{
  delete(VDIR_DATA_PTR);
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

  dir.name =  TextUtils::toupper(vdir->VDirName());
  if (VDirs.find(dir.name) != VDirs.end())
    return false;

  dir.plugin = plugin;
  dir.vdir = vdir;

  std::string vdirURL = BaseURL + vdir->VDirName();
  vdirURL += "/";

  vdir->BaseURL = vdirURL;

  VDirs[dir.name] = dir;
  return true;
}

BZF_API bool bzhttp_RemoveVDir (bz_Plugin* plugin, bzhttp_VDir *vdir )
{
  if (!plugin || !vdir)
    return false;
  VDir dir;

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

  if (names.empty())
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

class TickEvent : public bz_EventHandler
{
public:
  TickEvent()
  {
    plugin = NULL;
  }
  virtual void process (bz_EventData *)
  {
      CheckForZombies();
  }
};

ConnectionEvent *con = NULL;
TickEvent *tick = NULL;

//---- Response----
class bzhttp_Response_Data
{
public:
  std::map<std::string,std::string> Headers;
  std::map<std::string,std::string> Cookies;
  std::string Body;
};

#define RESPONSE_DATA_PTR ((bzhttp_Response_Data*)pimple)
#define RESPONSE_DATA(n) bzhttp_Response_Data *n = ((bzhttp_Response_Data*)pimple)
#define RESPONSE_DATA_CLASS(c,n) bzhttp_Response_Data *n = ((bzhttp_Response_Data*)(c))

bzhttp_Response::bzhttp_Response()
{
  Version = 1;
  pimple = new bzhttp_Response_Data;
  ReturnCode = e404NotFound;
  DocumentType = eHTML;
  ForceNoCache = false;
}

bzhttp_Response::~bzhttp_Response()
{
  delete(RESPONSE_DATA_PTR);
}

void bzhttp_Response::AddHeader ( const char* n, const char* v)
{
  RESPONSE_DATA(data);

  std::string name = n;
  data->Headers[name] = std::string(v);
}

void bzhttp_Response::AddCookies ( const char* n, const char* v)
{
  RESPONSE_DATA(data);

  std::string name = n;
   data->Cookies[name] = std::string(v);
}

void bzhttp_Response::AddBodyData ( const char* v)
{
  RESPONSE_DATA(data);

  if (v)
    data->Body += v;
}

void bzhttp_Response::AddBodyData ( const void* v, size_t size)
{
  RESPONSE_DATA(data);

  if (v && size)
    data->Body += std::string((const char*)v,size);
}

//---- Request----
class bzhttp_Request_Data
{
public:
  std::map<std::string,std::string> Headers;
  std::map<std::string,std::string> Cookies;
  std::map<std::string,std::string> Paramaters;
};

#define REQUEST_DATA_PTR ((bzhttp_Request_Data*)pimple)
#define REQUEST_DATA(n) bzhttp_Request_Data *n = ((bzhttp_Request_Data*)pimple)

bzhttp_Request::bzhttp_Request(): Session(NULL)
{
  Version = 1;
  pimple = new bzhttp_Request_Data;
}

bzhttp_Request::~bzhttp_Request()
{
  delete(REQUEST_DATA_PTR);
}

bool bzhttp_Request::UserHasPerm ( const char* perm ) const
{
  if (!BZID.size() || !BZIDCallsign.size() || !BZIDGroups.size() || !perm)
    return false;

  std::string permName = TextUtils::toupper(perm);
  PlayerAccessInfo::AccessPerm realPerm =  permFromName(permName);

  for( size_t i = 0; i < BZIDGroups.size(); i++ )
  {
    std::string groupName = BZIDGroups.get(i).c_str();
    groupName = TextUtils::toupper(groupName);
    PlayerAccessMap::iterator itr = groupAccess.find(groupName);
    if (itr == groupAccess.end())
      continue;

    if (itr->second.explicitAllows.test(realPerm) && !itr->second.explicitDenys.test(realPerm) )
      return true;

    for(unsigned int c = 0; c < itr->second.customPerms.size(); c++)
    {
      if (permName == TextUtils::toupper(itr->second.customPerms[c]))
	return true;
    }
  }

  return false;
}

void bzhttp_Request::AddHeader ( const char* n, const char* v)
{
  REQUEST_DATA(data);

  std::string name = TextUtils::toupper(n);
  if (data->Headers.find(name) == data->Headers.end())
    data->Headers[name] = std::string(v);
}

const char* bzhttp_Request::GetHeader ( const char* n) const
{
  REQUEST_DATA(data);

  std::string name = TextUtils::toupper(n);
  if (data->Headers.find(name) == data->Headers.end())
    return NULL;

  return data->Headers[name].c_str();
}

const char* bzhttp_Request::GetHeader ( size_t index ) const
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
    ++itr;
  }
  return NULL;
}

size_t bzhttp_Request::GetHeaderCount () const
{
  REQUEST_DATA(data);
  return data->Headers.size();
}

void bzhttp_Request::AddCookie ( const char* n, const char* v)
{
  REQUEST_DATA(data);

  std::string name = TextUtils::toupper(n);
  if (data->Headers.find(name) == data->Headers.end())
    data->Headers[name] = std::string(v);
}

const char* bzhttp_Request::GetCookie ( const char* n) const
{
  REQUEST_DATA(data);

  std::string name = TextUtils::toupper(n);
  if (data->Cookies.find(name) == data->Cookies.end())
    return NULL;

  return data->Cookies[name].c_str();
}

const char* bzhttp_Request::GetCookie ( size_t index ) const
{
  REQUEST_DATA(data);
  if (index >= data->Cookies.size())
    return NULL;

  size_t i =0;
  std::map<std::string,std::string>::iterator itr = data->Cookies.begin();
  while (itr != data->Cookies.end())
  {
    if (index == i)
      return itr->second.c_str();
    i++;
    ++itr;
  }
  return NULL;
}

size_t bzhttp_Request::GetCookieCount () const
{
  REQUEST_DATA(data);
  return data->Cookies.size();
}

void bzhttp_Request::AddParamater ( const char* n, const char* v)
{
  REQUEST_DATA(data);

  std::string name = TextUtils::toupper(n);
  if (data->Paramaters.find(name) == data->Paramaters.end())
    data->Paramaters[name] = std::string(v);
}

const char* bzhttp_Request::GetParamater ( const char* n) const
{
  REQUEST_DATA(data);

  std::string name = TextUtils::toupper(n);
  if (data->Paramaters.find(name) == data->Paramaters.end())
    return NULL;

  return data->Paramaters[name].c_str();
}

const char* bzhttp_Request::GetParamater ( size_t index ) const
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
    ++itr;
  }
  return NULL;
}

size_t bzhttp_Request::GetParamaterCount () const
{
  REQUEST_DATA(data);
  return data->Paramaters.size();
}

class HTTPConnectedPeer : public bz_NonPlayerConnectionHandler , public bz_BaseURLHandler
{
public:
  static HTTPConnectedPeer* Current;

  bool Killme;
  bzhttp_VDir *vDir;

  std::string Resource;
  std::string HTTPVersion;

  size_t ContentSize,HeaderSize;

  bool RequestComplete;

  bzhttp_Request  Request;
  bzhttp_Response Response;

  bool Authenticated;

  std::string bzIDAuthURL;
  std::string token;
  std::string callsign;
  std::string bzID;
  std::vector<std::string> bzGroups;

  std::string bzAuthReturnData;

  bool bzIDAuthFailed;
  bool bzIDAuthComplete;
  bool firedOffBZIDTokenCheck;

  HTTPConnectedPeer() : bz_NonPlayerConnectionHandler() ,bz_BaseURLHandler()
  {
    firedOffBZIDTokenCheck = false;
    bzIDAuthFailed = false;
	bzIDAuthComplete = false;
    Authenticated = false;
    RequestComplete = false;
    Killme = false;
    vDir = NULL;
    Request.RequestType = eHTTPUnknown;
    ContentSize = 0;
    HeaderSize = 0;
  }

  std::string GetAuthSessionData( const char* name )
  {
    if (!vDir || !name || !Request.Session)
      return std::string();

    std::string n = TextUtils::format("%s_%s",vDir->VDirName(),name);

    std::map<std::string,std::string> &list = GetServerSessionData(*Request.Session);
    std::map<std::string,std::string>::iterator itr = list.find(n);
    if (itr == list.end())
      return std::string();

    return itr->second;
  }

  void SetAuthSessionData( const char* name, const char* d )
  {
    if (!vDir || !name || !Request.Session)
      return;

    std::string n = TextUtils::format("%s_%s",vDir->VDirName(),name);

    std::map<std::string,std::string> &list = GetServerSessionData(*Request.Session);
    if (!d)
    {
      std::map<std::string,std::string>::iterator itr = list.find(n);
      if (itr != list.end())
	list.erase(itr);
    }
    else
      list[n] = std::string(d);
  }

  virtual void URLDone (const char*, const void *data, unsigned int size, bool complete)
  {
	  if (data && size)
		  bzAuthReturnData += std::string((const char*)data,size);

	  if (complete)
	  {
		  bzIDAuthComplete = true;
		  bzIDAuthURL.clear();

		  bzAuthReturnData = TextUtils::replace_all(bzAuthReturnData,"\r","");

		  std::vector<std::string> lines = TextUtils::tokenize(bzAuthReturnData,"\n");

		  if (lines.size() < 2)
			  bzIDAuthFailed = true;
		  else
		  {
			  std::vector<std::string> toks = TextUtils::tokenize(lines[1],":",2);
			  if (toks.size() > 1 && toks[0] == "TOKGOOD")
			  {
				  Authenticated = true;
				  SetAuthSessionData("bzidauthstatus","complete");

				  std::string groupList;
				  std::vector<std::string> groupToks = TextUtils::tokenize(lines[0]," ");
				  if ( groupToks.size() > 5)
				  {
					  for (size_t i = 5; i < groupToks.size(); i++)
					  {
						  std::vector<std::string> groups = TextUtils::tokenize(groupToks[i],"=");
						  bzGroups.push_back(groups[1]);
						  groupList += groups[1] + " ";

					  }
					  SetAuthSessionData("bzidauthgroups",groupList.c_str());

				  }
				  bzID = "";
				  // get the BZID
				  std::vector<std::string> chunks = TextUtils::tokenize(lines[2]," ");
				  if (chunks.size() > 1 && chunks[0] == "BZID:")
					  bzID = chunks[1];
				  else
					  bzIDAuthFailed = true;

				  SetAuthSessionData("bzid",bzID.c_str());
				  SetAuthSessionData("bzidcallsign",callsign.c_str());
			  }
			  else
				  bzIDAuthFailed = true;
		  }
	  }
  }

  virtual void URLTimeout ( const char* UNUSED(URL), int UNUSED(errorCode) )
  {
    bzIDAuthComplete = true;
    bzIDAuthFailed = true;
  }

  virtual void URLError ( const char* UNUSED(URL), int UNUSED(errorCode), const char * UNUSED(errorString) )
  {
	bzIDAuthComplete = true;
    bzIDAuthFailed = true;
  }

  virtual void pending(int connectionID, void *data, unsigned int size)
  {
	  RequestData.append(static_cast<char const*>(data), size);

	  // know our limits
	  size_t maxContentSize( 1024*1536 );
	  size_t maxBufferSize( 1024*2048 );

	  if (bz_BZDBItemHasValue("_MaxHTTPContentSize"))
		  maxBufferSize = bz_getBZDBInt("_MaxHTTPContentSize");

	  if (vDir && vDir->MaxRequestBody > (int)maxContentSize)
		  maxContentSize = vDir->MaxRequestBody;

	  if (bz_BZDBItemHasValue("_MaxHTTPRequestSize"))
		  maxBufferSize = bz_getBZDBInt("_MaxHTTPRequestSize");

	  if (maxBufferSize < maxContentSize)
		  maxBufferSize = maxContentSize;

	  if (vDir && vDir->MaxRequestSize > (int)maxBufferSize)
		  maxBufferSize = vDir->MaxRequestSize;

	  // check to see if we have too much data
	  if (RequestData.size() > maxBufferSize)
	  {
		  send501Error(connectionID);
		  return;
	  }

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

	unsigned int sessionID = 0;
	if (HeaderSize && !Request.GetHeaderCount())
	{
		std::string headerSubStir = RequestData.substr(0,HeaderSize);
		std::vector<std::string> headers = TextUtils::tokenize(headerSubStir,"\r\n");

		for ( size_t i = 1; i < headers.size(); i++ )
		{
			std::vector<std::string> headerParts = TextUtils::tokenize(headers[i],":",2);
			if (headerParts.size() > 1)
			{
				Request.AddHeader(headerParts[0].c_str(),headerParts[1].c_str());
				std::string name = TextUtils::toupper(headerParts[0]);
				if (name == "CONTENT-LENGTH") {
					int size1 = strtol(headerParts[1].c_str(), NULL, 0);
					ContentSize = (unsigned int)size1;
					if (ContentSize > maxContentSize) {
						send501Error(connectionID);
						return;
					}
				}
				if (name == "COOKIE")
				{
					std::vector<std::string> cookieParts = TextUtils::tokenize(headerParts[1],"=",2);
					if (cookieParts.size() > 1)
					{
						Request.AddCookie(cookieParts[0].c_str(),cookieParts[1].c_str());
						if (cookieParts[0] == SESSION_COOKIE)
							sessionID = atoi(cookieParts[1].c_str());
					}
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
		std::string IP = bz_getNonPlayerConnectionIP(connectionID);

		Request.URL = Resource;
		Request.Resource = resource;

		Request.RequesterIP = IP.c_str();
		if (bz_getNonPlayerConnectionHost(connectionID))
			Request.RequesterHost = bz_getNonPlayerConnectionHost(connectionID);

		if (Request.RequestType == eHTTPPost)
		{
			std::string body = RequestData.substr(HeaderSize,ContentSize);
			Request.Body = body;

			// parse the body for params
			parseParams(body);
		}
		else if (Request.RequestType == eHTTPPut)
			Request.Body = RequestData.substr(HeaderSize,ContentSize);


		if (sessionID == 0 && Request.GetParamater("SSID") != NULL)
			Request.Session = &GetSessionByID(Request.GetParamater("SSID"),IP.c_str());
		else
			Request.Session = &GetSession(sessionID,IP.c_str());

		// check and see if we have authentication to do

		bzhttp_eAuthenticationStatus authStatus = eAuthFail;

		if (vDir && vDir->RequiredAuthentiction != eNoAuth)
		{
			Authenticated = false;

			// check and see if we are cached
			if (vDir->CacheAuthentication)
			{
				if (vDir->RequiredAuthentiction == eBZID)
				{
					std::string id = GetAuthSessionData("bzid");
					if (id.size())
					{
						Authenticated = true;
						bzID = id;
						callsign = GetAuthSessionData("bzidcallsign");

						std::vector<std::string> groups = TextUtils::tokenize(GetAuthSessionData("bzidauthgroups")," ");
						for (size_t i = 0; i < groups.size(); i++)
						{
							if (groups[i] != callsign)
								bzGroups.push_back(groups[i]);
						}
					}
				}
				else
				{
					std::string status = GetAuthSessionData("authstatus");
					if (status == "complete")
						Authenticated = true;
				}
			}

			if (!Authenticated)
			{
				if (vDir->RequiredAuthentiction == eBZID)
				{
					std::string status = GetAuthSessionData("bzidauthstatus");
					if (status == "redired")
					{
						// see if they've got the stuff
						const char *t = Request.GetParamater("bzauthtoken");
						const char *c = Request.GetParamater("bzauthcallsign");

						if (t && c)
						{
							SetAuthSessionData("bzidauthstatus","requested");
							token = t;
							callsign = c;

							bzIDAuthURL = "http://my.bzflag.org/db/";
							if (bz_BZDBItemHasValue("_WebAuthCheckURL"))
								bzIDAuthURL = bz_getBZDBString("_WebAuthCheckURL").c_str();

							bzIDAuthURL += "?action=CHECKTOKENS&checktokens=";
							bzIDAuthURL += TextUtils::url_encode(callsign);
							bzIDAuthURL += "%3D";
							bzIDAuthURL += TextUtils::url_encode(token);

							std::string groups;
							// get the groups list
							PlayerAccessMap::iterator itr = groupAccess.begin();
							while (itr != groupAccess.end())
							{
								groups += TextUtils::url_encode(itr->first) + "%0D%0A";
								++itr;
							}

							for (size_t i = 0; i < vDir->BZIDAuthenicationGroups.size(); i++)
								groups += TextUtils::url_encode(vDir->BZIDAuthenicationGroups.get(i).c_str()) + "%0D%0A";

							if (groups.size())
							{
								groups.erase(groups.size()-6,6);
								bzIDAuthURL += "&groups=" + groups;
							}
							firedOffBZIDTokenCheck = true;
							bzIDAuthComplete = false;
#ifdef BZ_PLUGINS
							PushPendingHTTPWait();
#endif
							bz_setNonPlayerDisconnectOnSend(connectionID,false);
							bz_setNonPlayerInactivityTimeout(connectionID,120);
							bzAuthReturnData = "";
							bz_addURLJob(bzIDAuthURL.c_str(),this);

							//bz_startu
							authStatus = eNotAuthedYet;
						}
					}
					else
					{
						std::string authURL = "http://my.bzflag.org/weblogin.php";
						if (bz_BZDBItemHasValue("_WebAuthURL"))
							authURL = bz_getBZDBString("_WebAuthURL").c_str();

						authURL += "?action=weblogin&url=";
						std::string redirURL = vDir->BaseURL.c_str();

						std::string vname = "/";
						vname += vDir->VDirName();
						vname += "/";

						if (Resource.size() > vname.size())
							redirURL += Resource.c_str()+vname.size();

						if (redirURL.find_last_of('?') == std::string::npos)
							redirURL += "?";

						redirURL += TextUtils::format("bzauthtoken=%%TOKEN%%&bzauthcallsign=%%USERNAME%%&SSID=%d",Request.Session->SessionID);

						authURL += TextUtils::url_encode(redirURL);

						// authStatus = eNotAuthedYet;
						Response.ReturnCode = e302Found;
						Response.RedirectLocation = authURL.c_str();
						SetAuthSessionData("bzidauthstatus","redired");
					}

					if (Authenticated && vDir->CacheAuthentication)
						SetAuthSessionData("bzid", Request.BZID.c_str());
				}
				else
				{
					const char* authHeader = Request.GetHeader("Authorization");
					if (authHeader || vDir->RequiredAuthentiction == eHTTPOther)
					{
						if (authHeader)
						{
							std::vector<std::string> parts = TextUtils::tokenize(base64_decode(trimLeadingWhitespace(authHeader)),":",2);
							if (parts.size() > 1)
							{
								authStatus = vDir->AuthenticateHTTPUser(bz_getNonPlayerConnectionIP(connectionID),parts[0].c_str(),parts[1].c_str(),Request);
								if (authStatus == eAuthOK)
									Authenticated = true;
							}
							else
							{
								authStatus = vDir->AuthenticateHTTPUser(bz_getNonPlayerConnectionIP(connectionID),NULL,NULL,Request);
								if (authStatus == eAuthOK)
									Authenticated = true;
							}
						}

						if (!Authenticated)
						{
							if (!vDir->GenerateNoAuthPage(Request,Response) && authStatus == eAuthFail)
								Response.ReturnCode = e403Forbiden;
						}

						if (Authenticated && vDir->CacheAuthentication)
							SetAuthSessionData("authstatus", "complete");
					}
					else
						Response.ReturnCode = e401Unauthorized;
				}
			}

			if (!Authenticated && authStatus == eAuthFail)
			{
				GenerateResponse(connectionID);
				Killme = true;
				return;
			}
		}
		else
			Authenticated = true;

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

    VDIR_SESSION_CLASS(Request.Session->pimple,session);
    session->CurrentVdir = vDir->VDirName();

    // load up the auth data
    Request.BZID = bzID.c_str();
    Request.BZIDCallsign = callsign;

    for (size_t i =0; i < bzGroups.size(); i++)
      Request.BZIDGroups.push_back(bzGroups[i]);

    bzhttp_ePageGenStatus status = vDir->GeneratePage(Request,Response);

    session->CurrentVdir = "";

    return status;
  }

  void Think ( int connectionID )
  {
	  HTTPConnectedPeer::Current = this;
	  if (RequestComplete && !Killme)
	  {
		  if (vDir && !Authenticated)
		  {
			  if (vDir->RequiredAuthentiction == eHTTPOther)
			  {
				  bzhttp_eAuthenticationStatus authStatus = vDir->AuthenticateHTTPUser(bz_getNonPlayerConnectionIP(connectionID),NULL,NULL,Request);
				  if (authStatus == eAuthOK)
				  {
					  Authenticated = true;
					  if (vDir->CacheAuthentication)
						  SetAuthSessionData("authstatus", "complete");
				  }
				  else
				  {
					  if (!vDir->GenerateNoAuthPage(Request,Response) && authStatus == eAuthFail)
					  {
						  Response.ReturnCode = e403Forbiden;
						  GenerateResponse(connectionID);
						  Killme = true;
					  }
				  }
			  }
			  else if (vDir->RequiredAuthentiction == eBZID && bzIDAuthComplete) // check to see if we have authed yet or not
			  {
				  // if we failed then we suck
				  if (bzIDAuthFailed)
				  {
					  if (!vDir->GenerateNoAuthPage(Request,Response))
						  Response.ReturnCode = e403Forbiden;
					  GenerateResponse(connectionID);
					  Killme = true;
				  }
			  }
		  }

		  if (firedOffBZIDTokenCheck && bzIDAuthComplete)
		  {
#ifdef BZ_PLUGINS
			   PopPendingHTTPWait();
#endif
			   firedOffBZIDTokenCheck = false;
		  }

		  if (!vDir || Authenticated)
		  {
			  bzhttp_ePageGenStatus status = GetPage();

			  if (status == eNoPage)
				  send404Error(connectionID);
			  else if (status == ePageDone)
				  GenerateResponse(connectionID);
		  }
	  }
	  HTTPConnectedPeer::Current = NULL;

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

  void GenerateResponse (int connectionID)
  {
    RESPONSE_DATA_CLASS(Response.pimple,data);

    // set the session cookie
    Response.AddCookies(SESSION_COOKIE,TextUtils::format("%d",Request.Session->SessionID).c_str());

    std::string pageBuffer = "HTTP/1.1";

    bool redirected = false;

    switch (Response.ReturnCode)
    {
      case e200OK:
	pageBuffer += " 200 OK\n";
	break;
      case e301Redirect:
	if (Response.RedirectLocation.size())
	{
	  redirected = true;
	  pageBuffer += " 301 Moved Permanently\n";
	  pageBuffer += "Location: " + std::string(Response.RedirectLocation.c_str()) + "\n";
	}
	else
	  pageBuffer += " 500 Server Error\n";
	pageBuffer += "Host: " + ServerHostname + "\n";
	break;

      case e302Found:
	if (Response.RedirectLocation.size())
	{
	  redirected = true;
	  pageBuffer += " 302 Found\n";
	  pageBuffer += "Location: " + std::string(Response.RedirectLocation.c_str()) + "\n";
	}
	else
	  pageBuffer += " 500 Server Error\n";

	pageBuffer += "Host: " + ServerHostname + "\n";
	break;

      case e401Unauthorized:
	if (!vDir || vDir->RequiredAuthentiction == eBZID)
	  pageBuffer += " 403 Forbidden\n";
	else
	{
	  pageBuffer += " 401 Unauthorized\n";
	  pageBuffer += "WWW-Authenticate: ";

	  if (vDir->RequiredAuthentiction == eHTTPOther && vDir->OtherAuthenicationMethod.size())
	    pageBuffer += vDir->OtherAuthenicationMethod.c_str();
	  else
	    pageBuffer += "Basic";

	  pageBuffer += " realm=\"";
	  if (vDir->HTTPAuthenicationRelalm.size())
	    pageBuffer += vDir->HTTPAuthenicationRelalm.c_str();
	  else
	    pageBuffer += ServerHostname;
	  pageBuffer += "\"\n";
	}
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
      if (Response.ReturnCode == e200OK)
      {
	if (Response.DocumentType == eOther && Response.MimeType.size())
	  pageBuffer += Response.MimeType.c_str();
	else
	  pageBuffer += getMimeType(Response.DocumentType);
      }
      else
	pageBuffer += getMimeType(eHTML);

      pageBuffer += "\n";
    }

    if (Response.ForceNoCache)
      pageBuffer += "Cache-Control: no-cache\n";

    if (Response.MD5Hash.size())
      pageBuffer += "Content-MD5: " + std::string(Response.MD5Hash.c_str()) + "\n";

    pageBuffer += "Server: " + ServerVersion + "\n";

    bool didDate = false;

    std::map<std::string,std::string>::iterator itr = data->Headers.begin();
    while (itr != data->Headers.end())
    {
      pageBuffer += itr->first + ":" + itr->second + "\n";
      if (itr->first == "Date")
	didDate = true;
      ++itr;
    }

    if (!didDate)
    {
      bz_Time ts;
      bz_getUTCtime (&ts);
      pageBuffer += "Date: ";
      pageBuffer += printTime(&ts,"UTC");
      pageBuffer += "\n";
    }

    std::string cookieDomain;
    std::string cookiePath;

    if (Response.CookieDomain.size())
      cookieDomain = Response.CookieDomain.c_str();
    else
      cookieDomain = ServerHostname;

    if (Response.CookiePath.size())
      cookiePath = Response.CookiePath.c_str();
    else
    {
      if (vDir)
	cookiePath = TextUtils::format("/%s",vDir->VDirName());
      else
	cookiePath = "/";
    }


  //  if (Response.ReturnCode == e200OK)
    {
      itr = data->Cookies.begin();
      while (itr != data->Cookies.end())
      {
	std::string d,p;

	if (itr->first == SESSION_COOKIE)
	{

	  d = ServerHostname;
	  p = "/";
	}
	else
	{
	  d = cookieDomain;
	  p = cookiePath;
	}

	pageBuffer +=  TextUtils::format("Set-Cookie: %s=%s; Domain=%s; Path=%s; Max-Age=3600; HttpOnly\n",itr->first.c_str(),itr->second.c_str(),d.c_str(),p.c_str());
	++itr;
      }
    }

     pageBuffer += "\n";

     if (Request.RequestType != eHTTPHead && data->Body.size())
      pageBuffer += data->Body;

     pageBuffer += "\n\n";

     bz_setNonPlayerDisconnectOnSend(connectionID,redirected);

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
	if (!t.empty())
	  Request.AddParamater(TextUtils::url_decode(t[0]).c_str(), t.size() > 1 ? TextUtils::url_decode(t[1]).c_str() : "");
      }
    }
  }

  virtual void disconnect(int connectionID)
  {
    if (!connectionID)
      return;
    if (bzIDAuthURL.size())
      bz_removeURLJob(bzIDAuthURL.c_str());

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
    size_t item_read = fread(p,size,1,fp);
    fclose(fp);

    Response.MD5Hash = bz_MD5(p,size);

    if (Request.RequestType != eHTTPHead)
      Response.AddBodyData(p,size);
    free(p);

    if (item_read != 1)
      return eNoPage;

    Response.ReturnCode = e200OK;
    Response.DocumentType = eOther;
    Response.MimeType = Mime.c_str();

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
    if (bz_BZDBItemHasValue("_HTTPIndexResourceDir"))
      ResourceDirs.push_back(bz_getBZDBString("_HTTPIndexResourceDir"));

    AddStandardTypes();
  }

  virtual const char* VDirName(){return "INDEX";}

  virtual bzhttp_ePageGenStatus GeneratePage (const bzhttp_Request &, bzhttp_Response &response)
  {
    response.ReturnCode = e200OK;
    response.DocumentType = eHTML;
    response.AddBodyData("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"><html><head>");
    response.AddBodyData("<title>Index page for ");
    response.AddBodyData(ServerHostname.c_str());
    response.AddBodyData("</title></head><body>");

    if (VDirs.empty()) {
     response.AddBodyData("No HTTP Services are running on this server");
    } else {
      std::map<std::string,VDir>::iterator itr = VDirs.begin();
      {
	std::string vdirName = itr->second.vdir->VDirName();
	std::string vDirDescription = itr->second.vdir->VDirDescription();
	std::string line =  "<a href=\"/" + vdirName + "/\">" + vdirName +"</a>&nbsp;" +vDirDescription +"<br/>";
	response.AddBodyData(line.c_str());
      }
    }

    response.AddBodyData("</body></html>");

    return ePageDone;
  }

  virtual bool AllowResourceDownloads ( void )
  {
    return ResourceDirs.size() > 0;
  }
};

HTTPIndexHandler *indexHandler;

HTTPConnectedPeer* HTTPConnectedPeer::Current = NULL;

void InitHTTP()
{
  indexHandler = new HTTPIndexHandler();
  VDirs.clear();
  if (con || tick){  
    worldEventManager.removeEvent(bz_eTickEvent,tick);
    worldEventManager.removeEvent(bz_eNewNonPlayerConnection,con);
    delete(con);
    delete(tick);
  }
  con = new ConnectionEvent();
  tick = new TickEvent();

  worldEventManager.addEvent(bz_eNewNonPlayerConnection,con);
  worldEventManager.addEvent(bz_eTickEvent,tick);

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
    ++itr;
  }

  HTTPPeers.clear();
  if (con || tick)
  {
    worldEventManager.removeHandler(tick);
    worldEventManager.removeHandler(con);
    delete(con);
    con = NULL;
    delete(tick);
    tick = NULL;
 }
  VDirs.clear();
  delete(indexHandler);
}

void NewHTTPConnection ( bz_EventData *eventData )
{
  if (VDirs.empty())
    return;

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
	  std::string path = TextUtils::url_decode(resource.substr(0,dot));
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
  std::map<int,HTTPConnectedPeer*>::iterator itr = HTTPPeers.begin();
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

	itr++;
  }

  for (size_t i = 0; i < toKill.size(); i++)
    HTTPPeers.erase(HTTPPeers.find(toKill[i]));

  toKill.clear();

  double sessionTimeOut = 10*60;
  double rightNow = TimeKeeper::getCurrent().getSeconds();
  std::map<int,bzhttp_SessionData*>::iterator sessionItr = Sessions.begin();
  while(sessionItr != Sessions.end())
  {
    VDIR_SESSION_CLASS(sessionItr->second->pimple,data);
    if (data->LastUpdateTime + sessionTimeOut < rightNow)
    {
      delete (sessionItr->second);
      toKill.push_back(sessionItr->first);
    }
    ++sessionItr;
  }

  for (size_t i = 0; i < toKill.size(); i++)
    Sessions.erase(Sessions.find(toKill[i]));
}

// templates

class bzhttp_TMD_Data
{
public:
  std::map< std::string, std::vector<std::string> > Items;
};

#define TMD_DATA_PTR ((bzhttp_TMD_Data*)pimple)
#define TMD_DATA(n) bzhttp_TMD_Data *n = ((bzhttp_TMD_Data*)pimple)
#define TMD_DATA_CLASS(c,n) bzhttp_TMD_Data *n = ((bzhttp_TMD_Data*)(c))

bzhttp_TemplateMetaData::bzhttp_TemplateMetaData()
{
  pimple = new bzhttp_TMD_Data();
}

bzhttp_TemplateMetaData::bzhttp_TemplateMetaData( const bzhttp_TemplateMetaData& r)
{
  pimple = new bzhttp_TMD_Data();

  TMD_DATA(data);

  TMD_DATA_CLASS(r.pimple,otherData);
  data->Items = otherData->Items;
}

bzhttp_TemplateMetaData::~bzhttp_TemplateMetaData()
{
  delete(TMD_DATA_PTR);
}

const char * bzhttp_TemplateMetaData::Get ( const char* key, unsigned int index )
{
  if (!key)
    return NULL;

  TMD_DATA(data);
  std::map< std::string, std::vector<std::string> >::iterator itr = data->Items.find(key);
  if (itr == data->Items.end())
    return NULL;
  if (index >= itr->second.size())
    return NULL;
  return itr->second[index].c_str();
}

unsigned int bzhttp_TemplateMetaData::Count ( const char* key )
{
  if (!key)
    return 0;

  TMD_DATA(data);
  std::map< std::string, std::vector<std::string> >::iterator itr = data->Items.find(key);
  if (itr == data->Items.end())
    return 0;
  return itr->second.size();
}

void bzhttp_TemplateMetaData::Add ( const char* key, const char* val )
{
  if (!key || !val)
    return;

  TMD_DATA(data);
  std::map< std::string, std::vector<std::string> >::iterator itr = data->Items.find(key);
  if (itr == data->Items.end())
    data->Items[key] = std::vector<std::string>();

   data->Items[key].push_back(val);
}


class TemplateInfo
{
public:
  bzhttp_TemplateCallback* Callback;
  std::string PathSet;
  std::string RootPath;
};

std::string processTemplate ( const std::string &templateText, TemplateInfo& info );

std::string getStringRange ( const std::string &find, size_t start, size_t end )
{
  std::string ret;

  if (end <= start || start > find.size() || end > find.size())
    return ret;

  for ( size_t p = start; p <= end; p++)
    ret += find[p];

  return ret;
}

std::string readFileText ( const char* file )
{
  FILE *fp = fopen(file,"rb");
  if (!fp)
    return std::string();

  fseek(fp,0,SEEK_END);
  size_t s = ftell(fp);
  fseek(fp,0,SEEK_SET);

  char* m = (char*)malloc(s+1);
  size_t item_read = fread(m, s, 1, fp);
  m[s] = 0;
  fclose(fp);

  std::string ret(m);
  free(m);

  if (item_read != 1)
    return std::string();

  if (ret.find_first_of('\n') != std::string::npos)
    return ret;

  // HTTP likes /r/n not just /r
  return TextUtils::replace_all(ret,"\r","\r\n");
}

void GetTemplateMetaData( const std::string &templateText, bzhttp_TemplateMetaData& metaData)
{
  size_t pos = 0;
  while ( pos < templateText.size() && pos != std::string::npos)
  {
    pos = TextUtils::find_first_substr(templateText,std::string("[#"),pos);
    if ( pos < templateText.size() && pos != std::string::npos )
    {
      size_t start = pos;

      pos = TextUtils::find_first_substr(templateText,std::string("]"),pos);
      if ( pos < templateText.size() && pos != std::string::npos )
      {
	std::string dataKey = getStringRange(templateText,start+2,pos-1);

	std::vector<std::string> chunks = TextUtils::tokenize(dataKey,std::string(":"),0,true);
	if (chunks.size() > 1)
	  metaData.Add(chunks[0].c_str(),chunks[1].c_str());
      }
    }
  }
}

void makelower ( std::string &str)
{
  str = TextUtils::tolower(str);
}

std::string getDirName ( const std::string& file )
{
  std::string f = file;
  size_t s = f.find_last_of(_DirDelim);
  if (s != std::string::npos)
    f.erase(s+1,f.size()-s-1);

  return f;
}

std::string::const_iterator readKey ( std::string &key, std::string::const_iterator inItr, const std::string &str )
{
  std::string::const_iterator itr = inItr;

  while ( itr != str.end() ) {
    if (*itr != ']') {
      key += *itr;
      ++itr;
    } else {
      // go past the code
      ++itr;
      key = TextUtils::tolower(key);
      return itr;
    }
  }
  return itr;
}


std::string::const_iterator findNextTag ( const std::vector<std::string> &keys, std::string &endKey, std::string &code, std::string::const_iterator inItr, const std::string &str )
{
  if (keys.empty())
    return inItr;

  std::string::const_iterator itr = inItr;

  while (1) {
    itr = std::find(itr,str.end(),'[');
    if (itr == str.end())
      return itr;

    // save off the itr in case this is the one, so we can copy to this point
    std::string::const_iterator keyStartItr = itr;

    ++itr;
    if (itr == str.end())
      return itr;

    std::string key;
    itr = readKey(key,itr,str);

    for ( size_t i = 0; i < keys.size(); i++ ) {
      if ( key == keys[i]) {
	endKey = key;
	code.resize(keyStartItr - inItr);
	std::copy(inItr,keyStartItr,code.begin());
	return itr;
      }
    }
  }
  return itr;
}

double startTime;

const char* CallKey ( std::string& key, bzhttp_TemplateCallback* callback)
{
  if (key == "date") {
    bz_Time time;
    bz_getLocaltime(&time);
    return TextUtils::format("%d/%d/%d",time.month,time.day,time.year).c_str();
  } else if (key == "time") {
    bz_Time time;
    bz_getLocaltime(&time);
    TextUtils::format("%d:%d:%d",time.hour,time.minute,time.second);
  } else if (key == "hostname") {
    std::string data = bz_getPublicAddr().c_str();
    if (!data.size())
     return TextUtils::format("localhost:%d",bz_getPublicPort()).c_str();
  } else if (key == "pagetime") {
    return TextUtils::format("%.3f",bz_getCurrentTime()-startTime).c_str();
  } else if (key == "baseurl") {
    return HTTPConnectedPeer::Current->vDir->BaseURL.c_str();
  } else if (key == "pluginname") {
    return VDirs[HTTPConnectedPeer::Current->vDir->VDirName()].plugin ? VDirs[HTTPConnectedPeer::Current->vDir->VDirName()].plugin->Name() : "";
  }
  else if (callback)
    return callback->GetTemplateKey(key.c_str());

  return "";
}

void replaceVar ( std::string &code, std::string::const_iterator &itr, const std::string &str, TemplateInfo& info )
{
  // find the end of the ]]
  std::string key;

  itr = readKey(key,itr,str);

  if (itr != str.end()) {
    std::string lowerKey(TextUtils::tolower(key));
    code += CallKey(lowerKey,info.Callback);
  }
}

void processLoop ( std::string &code, std::string::const_iterator &inItr, const std::string &str, TemplateInfo& info )
{
  std::string key,loopSection,emptySection,param;

  // read the rest of the key
  std::string::const_iterator itr = readKey(key,inItr,str);

  std::vector<std::string> commandParts = TextUtils::tokenize(key,std::string(" "),0,0);
  if (commandParts.size() < 2)
  {
    inItr = itr;
    return;
  }

  // check the params
  makelower(commandParts[0]);
  makelower(commandParts[1]);

  if (commandParts.size() > 2)
    param = commandParts[2];

  if ( commandParts[0] != "start" )
  {
    inItr = itr;
    return;
  }

  std::vector<std::string> checkKeys;
  checkKeys.push_back(TextUtils::format("*end %s",commandParts[1].c_str()));

  std::string keyFound;
  itr = findNextTag(checkKeys,keyFound,loopSection,itr,str);

  if (itr == str.end())
  {
    inItr = itr;
    return;
  }

  // do the empty section
  // loops have to have both
  checkKeys.clear();
  checkKeys.push_back(TextUtils::format("*empty %s",commandParts[1].c_str()));
  itr = findNextTag(checkKeys,keyFound,emptySection,itr,str);

  if (info.Callback && info.Callback->GetTemplateLoop(commandParts[1].c_str(),param.c_str()))
  {
    code += processTemplate(loopSection,info);

    while(info.Callback && info.Callback->GetTemplateLoop(commandParts[1].c_str(),param.c_str()))
      code += processTemplate(loopSection,info);
  }
  else
    code += processTemplate(emptySection,info);
  inItr = itr;
}

bool CallIF ( const std::string &key, const std::string &param, bzhttp_TemplateCallback* callback )
{
  if (key == "public")
    return bz_getPublic();

  if (callback)
    return callback->GetTemplateIF(key.c_str(),param.c_str());
  return false;
}

void processIF ( std::string &code, std::string::const_iterator &inItr, const std::string &str, TemplateInfo& info )
{
  std::string key;

  // read the rest of the key
  std::string::const_iterator itr = readKey(key,inItr,str);

  std::vector<std::string> commandParts = TextUtils::tokenize(key,std::string(" "),0,0);
  if (commandParts.size() < 2) {
    inItr = itr;
    return;
  }

  // check the params
  makelower(commandParts[0]);
  makelower(commandParts[1]);

  if ( commandParts[0] != "if" ) {
    inItr = itr;
    return;
  }

  std::string param;
  if (commandParts.size() > 2)
    param = commandParts[2];

  // now get the code for the next section
  std::string trueSection,elseSection;

  std::vector<std::string> checkKeys;
  checkKeys.push_back(TextUtils::format("?else %s",commandParts[1].c_str()));
  checkKeys.push_back(TextUtils::format("?end %s",commandParts[1].c_str()));

  std::string keyFound;
  itr = findNextTag(checkKeys,keyFound,trueSection,itr,str);

  if (keyFound == checkKeys[0]) { // we hit an else, so we need to check for it
    // it was the else, so go and find the end too
    if (itr == str.end()) {
      inItr = itr;
      return;
    }

    checkKeys.erase(checkKeys.begin());// kill the else, find the end
    itr = findNextTag(checkKeys,keyFound,elseSection,itr,str);
  }

  // test the if, stuff that dosn't exist is false
  if (CallIF(commandParts[1],param,info.Callback))
    code += processTemplate(trueSection,info);
  else
    code += processTemplate(elseSection,info);

  inItr = itr;
}

void processComment ( std::string & UNUSED(code), std::string::const_iterator &inItr, const std::string &str,  TemplateInfo& UNUSED(info) )
{
  std::string key;
  inItr = readKey(key,inItr,str);
}

void processInclude ( std::string &code, std::string::const_iterator &inItr, const std::string &str, TemplateInfo& info )
{
  std::string key;
  inItr = readKey(key,inItr,str);

  // check the search paths for the include file

  std::string templatePath;

  if (info.RootPath.size())
  {
    std::string temp = concatPaths(info.RootPath.c_str(),key.c_str());
    if (fileExits(temp.c_str()))
      templatePath = temp;
  }
  if (!templatePath.size() || info.PathSet.size())
  {
    const char* t = bzhttp_FindFile(info.PathSet.c_str(),key.c_str());
    if (t)
      templatePath = t;
  }

  if (!templatePath.size() || fileExits(key.c_str()))
    templatePath = key;

  if (!templatePath.size())
    return;

  bzhttp_TemplateMetaData *oldMeta = NULL;
  bzhttp_TemplateMetaData *newMeta = NULL;

  std::string templateContents = readFileText(templatePath.c_str());

  if (info.Callback)
  {
    oldMeta = info.Callback->MetaData;
    newMeta = new bzhttp_TemplateMetaData(*oldMeta);
    GetTemplateMetaData(templateContents,*newMeta);
    info.Callback->MetaData = newMeta;
  }

  TemplateInfo includeInfo = info;
  includeInfo.RootPath = getDirName(templatePath);

  code += processTemplate(templateContents,includeInfo);

  if (info.Callback)
  {
    info.Callback->MetaData = oldMeta;
    delete(newMeta);
  }
}

std::string processTemplate ( const std::string &templateText, TemplateInfo& info )
{
  std::string code;

  std::string::const_iterator templateItr = templateText.begin();

  while ( templateItr != templateText.end() ) {
    if ( *templateItr != '[' ) {
      code += *templateItr;
      ++templateItr;
    } else {
      ++templateItr;

      if (templateItr == templateText.end()) {
	code += '[';
      } else {
	switch(*templateItr) {
	default: // it's not a code, so just let the next loop hit it and output it
	  break;

	case '$':
	  replaceVar(code,++templateItr,templateText,info);
	  break;

	case '*':
	  processLoop(code,++templateItr,templateText,info);
	  break;

	case '?':
	  processIF(code,++templateItr,templateText,info);
	  break;
	case '-':
	case '#': // treat metadata as comments when parsing
	  processComment(code,++templateItr,templateText,info);
	  break;
	case '!':
	  processInclude(code,++templateItr,templateText,info);
	  break;
	}
      }
    }
  }
  return code;
}

BZF_API bz_ApiString bzhttp_RenderTemplate ( const char* file, bzhttp_TemplateCallback* callback, const char *pathSet)
{
  if (!file)
    return bz_ApiString();

  std::string templateText = readFileText(file);

  TemplateInfo info;
  info.Callback = callback;
  if (pathSet)
    info.PathSet = pathSet;
  info.RootPath = getDirName(std::string(file));

  startTime = TimeKeeper::getCurrent().getSeconds();

  bzhttp_TemplateMetaData meta;
  GetTemplateMetaData(templateText,meta);
  if (callback)
    callback->MetaData = &meta;

  std::string code = processTemplate(templateText,info);

  if (callback)
    callback->MetaData = NULL;

  return bz_ApiString(code);
}


BZF_API bz_ApiString bzhttp_RenderTemplateFromText ( const char* text, bzhttp_TemplateCallback* callback, const char* pathSet )
{
  if (!text || !*text)
    return bz_ApiString();

  TemplateInfo info;
  info.Callback = callback;
  if (pathSet)
	info.PathSet = pathSet;

  startTime = TimeKeeper::getCurrent().getSeconds();

  std::string templateText = text;

  bzhttp_TemplateMetaData meta;
  GetTemplateMetaData(templateText,meta);
  if (callback)
    callback->MetaData = &meta;

  std::string code = processTemplate(templateText,info);

  if (callback)
    callback->MetaData = NULL;

  return bz_ApiString(code);
}

BZF_API bzhttp_TemplateMetaData bzhttp_GetTemplateMetaData( const char* file )
{
  bzhttp_TemplateMetaData data;
  GetTemplateMetaData(readFileText(file),data);
  return data;
}

// path utils
std::map<std::string,std::vector<std::string> > PathSets;

BZF_API bool bzhttp_AddSearchPath ( const char* pathSet, const char* path )
{
  if (!pathSet || !path)
    return false;

  std::string name = std::string(pathSet);

  if (PathSets.find(name) == PathSets.end())
    PathSets[name] = std::vector<std::string>();

  PathSets[name].push_back(std::string(path));
  return true;
}

BZF_API const char* bzhttp_FindFile ( const char* pathSet, const char* filename )
{
  if (!pathSet || !filename)
    return NULL;

  std::string name = std::string(pathSet);

  if (PathSets.find(name) == PathSets.end())
    return NULL;

  std::vector<std::string> &list = PathSets[name];
  for ( size_t i = 0; i < list.size(); i++)
  {
    static const std::string path = concatPaths(list[i].c_str(),filename);
    if (fileExits(path.c_str()))
      return path.c_str();
  }
  return NULL;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
