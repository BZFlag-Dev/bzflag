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

class ConnectionEvent : public bz_EventHandler
{
public:
  ConnectionEvent()
  {
    plugin = NULL;
  }
  virtual void process ( bz_EventData *eventData )
  {
    NewHTTPConnection(eventData);
  }
};

ConnectionEvent con;

class HTTPConnectedPeer : bz_NonPlayerConnectionHandler
{
public:
  bool Killme;
  VDir *vDir;

  virtual void pending(int connectionID, void *data, unsigned int size)
  {

  }

  virtual void disconnect(int connectionID)
  {
    if (connectionID)
      Killme = true;
  }
};

std::map<int,HTTPConnectedPeer> HTTPPeers;

void InitHTTP()
{
  VDirs.clear();
  worldEventManager.addEvent(bz_eNewNonPlayerConnection,&con);
}

void KillHTTP()
{
  worldEventManager.removeEvent(bz_eNewNonPlayerConnection,&con);
  VDirs.clear();
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

  // figure out who gets it
  

  //connData->data
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
