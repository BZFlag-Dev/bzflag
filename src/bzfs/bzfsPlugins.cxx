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

#include "common.h"

/* system headers */
#include <string>
#include <vector>
#include <map>

/* common headers */
#include "bzfio.h"
#include "version.h"
#include "TextUtils.h"
#include "commands.h"
#include "bzfsAPI.h"
#include "DirectoryNames.h"
#include "bzfsPlugins.h"

#include "WorldEventManager.h"

#include "TextUtils.h"

// for HTTP
void InitHTTP();
void KillHTTP();

#ifdef _WIN32
std::string extension = ".dll";
std::string globalPluginDir = ".\\plugins\\";
#else
std::string extension = ".so";
std::string globalPluginDir = INSTALL_LIB_DIR;
#endif

typedef std::map<std::string, bz_APIPluginHandler*> tmCustomPluginMap;
tmCustomPluginMap customPluginMap;

std::string lastPluginDir;

typedef struct
{
	std::string name;
	std::string filename;
	bz_Plugin* plugin;
#ifdef _WIN32
	HINSTANCE	handle;
#else
	void*		handle;
#endif
}trPluginRecord;

std::string findPlugin ( std::string pluginName )
{
  // see if we can just open the bloody thing
  FILE	*fp = fopen(pluginName.c_str(),"rb");
  if (fp)
  {
	  fclose(fp);
	  return pluginName;
  }

  // now try it with the standard extension
  std::string name = pluginName + extension;
  fp = fopen(name.c_str(),"rb");
  if (fp)
  {
	  fclose(fp);
	  return name;
  }

  // check the local users plugins dir
  name = getConfigDirName(BZ_CONFIG_DIR_VERSION) + pluginName + extension;
  fp = fopen(name.c_str(),"rb");
  if (fp)
  {
	  fclose(fp);
	  return name;
  }

  // check the global plugins dir
  name = globalPluginDir + pluginName + extension;
  fp = fopen(name.c_str(),"rb");
  if (fp)
  {
	  fclose(fp);
	  return name;
  }

  return std::string("");
}

std::string getPluginPath ( const std::string & path )
{
  if ( path.find('/') == std::string::npos && path.find('\\') == std::string::npos )
  {
    return std::string ("./");
  }

  std::string newPath = path;
  size_t lastSlash = newPath.find_last_of('/');
  if (lastSlash != std::string::npos)
    newPath.erase(newPath.begin()+lastSlash+1,newPath.end());
  else
  {
    lastSlash = newPath.find_last_of('\\');
    if (lastSlash != std::string::npos)
      newPath.erase(newPath.begin()+lastSlash+1,newPath.end());
  }

  return newPath;
}

std::vector<trPluginRecord>	vPluginList;

void unload1Plugin ( int iPluginID );

bool PluginExists ( const char* n )
{
  std::string name = n;

  std::vector<trPluginRecord>::iterator itr = vPluginList.begin();
  while ( itr != vPluginList.end()) {
    if (itr->name == name)
      return true;
    ++itr;
  }
  return false;
}

bz_Plugin* getPlugin( const char* n )
{
  std::string name = n;

  std::vector<trPluginRecord>::iterator itr = vPluginList.begin();
  while ( itr != vPluginList.end()) {
    if (itr->name == name)
      return itr->plugin;
    ++itr;
  }
  return NULL;
}

#ifdef _WIN32
#  include <windows.h>

int getPluginVersion ( HINSTANCE hLib )
{
  int (*lpProc)(void);
  lpProc = (int (__cdecl *)(void))GetProcAddress(hLib, "bz_GetMinVersion");
  if (lpProc)
	  return lpProc();
  return 0;
}

bool load1Plugin ( std::string plugin, std::string config )
{
	bz_Plugin* (*lpProc)(void);

	std::string realPluginName = findPlugin(plugin);

	HINSTANCE	hLib = LoadLibrary(realPluginName.c_str());
	if (hLib)
	{
		if (getPluginVersion(hLib) > BZ_API_VERSION)
		{
			logDebugMessage(1,"Plugin: %s found but needs a newer API version (%d), upgrade server\n",plugin.c_str(),getPluginVersion(hLib));
			FreeLibrary(hLib);
			return false;
		}
		else
		{
			lpProc = (bz_Plugin* (__cdecl *)(void))GetProcAddress(hLib, "bz_GetPlugin");
			if (lpProc)
			{
			    lastPluginDir = getPluginPath(realPluginName);

				bz_Plugin* p = lpProc();
				if (!p)
				  return false;

				std::string name  = p->Name();
				if (PluginExists(name.c_str()))
				{
				  FreeLibrary(hLib);
				  return false;
				}

				trPluginRecord pluginRecord;
				pluginRecord.name = name;
				pluginRecord.handle = hLib;
				pluginRecord.plugin = p;
				pluginRecord.filename = plugin;
				vPluginList.push_back(pluginRecord);

				p->Init(config.c_str());

				bz_PluginLoadUnloadEventData_V1 evt;
				evt.plugin = p;
				evt.eventType = bz_ePluginLoaded;
				worldEventManager.callEvents(&evt);

				logDebugMessage(1,"Plugin: %s loaded from %s\n",pluginRecord.name.c_str(),plugin.c_str());
				bz_debugMessagef(4,"%s plugin loaded",pluginRecord.name.c_str());
				return true;
			}
			else
			{
				logDebugMessage(1,"Plugin: %s found but does not contain bz_GetPlugin method\n",plugin.c_str());
				FreeLibrary(hLib);
				return false;
			}
		}
	}
	else
	{
		logDebugMessage(1,"Plugin: %s not found\n",plugin.c_str());
		return false;
	}
}

void unload1Plugin ( int iPluginID )
{
	void (*lpProc)(bz_Plugin*);

	trPluginRecord &plugin = vPluginList[iPluginID];

	bz_PluginLoadUnloadEventData_V1 evt;
	evt.plugin = plugin.plugin;
	evt.eventType = bz_ePluginUnloaded;
	worldEventManager.callEvents(&evt);

	FlushEvents(plugin.plugin);
	plugin.plugin->Cleanup();
	bz_debugMessagef(4,"%s plugin unloaded",plugin.plugin->Name());

	lpProc = (void (__cdecl *)(bz_Plugin*))GetProcAddress(plugin.handle, "bz_FreePlugin");
	if (lpProc)
	  lpProc(plugin.plugin);
	else
	  logDebugMessage(1,"Plugin: bz_FreePlugin method not used by number %d. Leaking memory.\n",iPluginID);

	FreeLibrary(plugin.handle);
	plugin.handle = NULL;
	plugin.plugin = NULL;
}
#else

#  include <dlfcn.h>
std::vector<void*>	vLibHandles;

int getPluginVersion ( void* hLib )
{
	int (*lpProc)(void);
	*(void**) &lpProc = dlsym(hLib,"bz_GetMinVersion");
	if (lpProc)
		return (*lpProc)();
	return 0;
}

bool load1Plugin ( std::string plugin, std::string config )
{
	bz_Plugin* (*lpProc)();

	std::string realPluginName = findPlugin(plugin);

	void *hLib = dlopen(realPluginName.c_str(), RTLD_LAZY | RTLD_GLOBAL);
	if (hLib)
	{
		if (dlsym(hLib, "bz_GetPlugin") == NULL) {
			logDebugMessage(1,"Plugin: %s found but does not contain bz_GetPlugin method, error %s\n",plugin.c_str(),dlerror());
			dlclose(hLib);
			return false;
		}

		int version = getPluginVersion(hLib);
		if (version > BZ_API_VERSION)
		{
			logDebugMessage(1,"Plugin: %s found but needs a newer API version (%d), upgrade server\n",plugin.c_str(), version);
			dlclose(hLib);
			return false;
		}
		else
		{
			*(void**) &lpProc = dlsym(hLib,"bz_GetPlugin");
			if (lpProc)
			{
				lastPluginDir = getPluginPath(realPluginName);
				bz_Plugin * p = (*lpProc)();
				if (!p)
				  return false;

				std::string name  = p->Name();
				if (PluginExists(name.c_str()))
				  return false;

				trPluginRecord pluginRecord;
				pluginRecord.handle = hLib;
				pluginRecord.name = name;
				pluginRecord.plugin = p;
				pluginRecord.filename = plugin;
				vPluginList.push_back(pluginRecord);
				logDebugMessage(1,"Plugin: %s loaded from %s\n",pluginRecord.name.c_str(),plugin.c_str());
				bz_debugMessagef(4,"%s plugin loaded",pluginRecord.name.c_str());

				p->Init(config.c_str());
				bz_PluginLoadUnloadEventData_V1 evt;
				evt.plugin = p;
				evt.eventType = bz_ePluginLoaded;
				worldEventManager.callEvents(&evt);
				return true;
			}
		}
	}
	else
	{
		logDebugMessage(1,"Plugin: %s not found, error %s\n",plugin.c_str(), dlerror());
		return false;
	}

	logDebugMessage(1,"Plugin: load1Plugin() coding error\n");
	return false;
}

void unload1Plugin ( int iPluginID )
{
	void (*lpProc)(bz_Plugin*);
	trPluginRecord &plugin = vPluginList[iPluginID];

	bz_PluginLoadUnloadEventData_V1 evt;
	evt.plugin = plugin.plugin;
	evt.eventType = bz_ePluginUnloaded;
	worldEventManager.callEvents(&evt);

	FlushEvents(plugin.plugin);
	plugin.plugin->Cleanup();
	bz_debugMessagef(4,"%s plugin unloaded",plugin.plugin->Name());

	*(void**) &lpProc = dlsym(plugin.handle, "bz_FreePlugin");
	if (lpProc)
	  (*lpProc)(plugin.plugin);
	else
	  logDebugMessage(1,"Plugin: bz_FreePlugin method not used by number %d, error %s. Leaking memory.\n",iPluginID,dlerror());

	dlclose(plugin.handle);
	plugin.handle = NULL;
	plugin.plugin = NULL;
}
#endif


bool loadPlugin ( std::string plugin, std::string config )
{
  // check and see if it's an extension we have a handler for
  std::string ext;

  std::vector<std::string> parts = TextUtils::tokenize(plugin,std::string("."));
  ext = parts[parts.size()-1];

  tmCustomPluginMap::iterator itr = customPluginMap.find(TextUtils::tolower(ext));

  bool ret = false;

  if (itr != customPluginMap.end() && itr->second){
    bz_APIPluginHandler *handler = itr->second;
    ret =  handler->APIPlugin(plugin,config);
  }
  else
    ret =  load1Plugin(plugin,config);

  lastPluginDir = "";
  return ret;
}

bool unloadPlugin ( std::string plugin )
{
  // unload the first one of the name we find
  for (unsigned int i = 0; i < vPluginList.size();i++){
    if ( (vPluginList[i].name == plugin || vPluginList[i].filename == plugin) && vPluginList[i].plugin->Unloadable ){
      unload1Plugin(i);
      vPluginList.erase(vPluginList.begin()+i);
      return true;
    }
  }
  return false;
}

void unloadPlugins ( void )
{
  KillHTTP();

  for (unsigned int i = 0; i < vPluginList.size();i++)
    unload1Plugin(i);
  vPluginList.clear();

  removeCustomSlashCommand("loadplugin");
  removeCustomSlashCommand("unloadplugin");
  removeCustomSlashCommand("listplugins");
}

std::vector<std::string> getPluginList ( void )
{
  std::vector<std::string> plugins;
  for (unsigned int i = 0; i < vPluginList.size();i++)
    plugins.push_back(vPluginList[i].name);

  return plugins;
}

int pendingHTTPAuths = 0;

void PushPendingHTTPWait ()
{
	pendingHTTPAuths++;
}

void PopPendingHTTPWait ()
{
	pendingHTTPAuths--;
	if (pendingHTTPAuths < 0)
		pendingHTTPAuths = 0;
}

float getPluginMinWaitTime ( void )
{
  float maxTime = 1000.0;

  std::vector<std::string> plugins;
  for (unsigned int i = 0; i < vPluginList.size();i++)
  {
    if (vPluginList[i].plugin &&  (vPluginList[i].plugin->MaxWaitTime > 0) && (vPluginList[i].plugin->MaxWaitTime < maxTime))
      maxTime = vPluginList[i].plugin->MaxWaitTime;
  }
  if (pendingHTTPAuths > 0)
	  return 0;

  return maxTime;
}

class DynamicPluginCommands : public bz_CustomSlashCommandHandler
{
  public:
    virtual ~DynamicPluginCommands(){};
    virtual bool SlashCommand ( int playerID, bz_ApiString _command, bz_ApiString _message, bz_APIStringList *params )
    {
      bz_BasePlayerRecord	record;

      std::string command = _command.c_str();
      std::string message = _message.c_str();

      bz_BasePlayerRecord	*p = bz_getPlayerByIndex(playerID);
      if (!p)
	return false;

      record = *p;

      bz_freePlayerRecord(p);

      if (TextUtils::tolower(command) == "listplugins" ) {
	if (record.hasPerm("LISTPLUGINS") || record.hasPerm("PLUGINS")) {
	  std::vector<std::string> plugins = getPluginList();

	  if (plugins.empty()) {
	    bz_sendTextMessage(BZ_SERVER,playerID,"No Plug-ins loaded.");
	  } else {
	    bz_sendTextMessage(BZ_SERVER,playerID,"Plug-ins loaded:");

	    for ( unsigned int i = 0; i < plugins.size(); i++) {
	      char tmp[256];
	      sprintf(tmp,"%d %s", i+1, plugins[i].c_str());
	      bz_sendTextMessage(BZ_SERVER,playerID,tmp);
	    }
	  }
	}
	else {
	  bz_sendTextMessage(BZ_SERVER,playerID,"You do not have permission to view loaded plug-ins.");
	}

	return true;
      }

      if (!record.hasPerm("PLUGINS")) {
	bz_sendTextMessage(BZ_SERVER,playerID,"You do not have permission to (un)load plug-ins.");
	return true;
      }

      if ( TextUtils::tolower(command) == "loadplugin" ) {
	if ( !params->size() ) {
	  bz_sendTextMessage(BZ_SERVER,playerID,"Usage: /loadplugin plug-in");
	  return true;
	}

	std::vector<std::string> subparams = TextUtils::tokenize(message,std::string(","));

	std::string config;
	if ( subparams.size() >1)
	  config = subparams[1];

	if (loadPlugin(subparams[0],config))
	  bz_sendTextMessage(BZ_SERVER,playerID,"Plug-in loaded.");
	else
	  bz_sendTextMessage(BZ_SERVER,playerID,"Plug-in load failed.");
	return true;
      }

      if ( TextUtils::tolower(command) == "unloadplugin" ) {
	if ( !params->size() ) {
	  bz_sendTextMessage(BZ_SERVER,playerID,"Usage: /unloadplugin plug-in");
	  return true;
	}

	std::string name;
	for(size_t i = 0; i < params->size(); i++) {
	  name += params->get(i).c_str();
	  if (i != params->size()-1)
	    name += " ";
	}
	if (TextUtils::isNumeric(name[0])) {
	  int index = atoi(name.c_str())-1;
	  std::vector<std::string>	plugins = getPluginList();
	  if (index > 0 && index < (int)plugins.size())
	    name = plugins[index];
	}
	if ( unloadPlugin(name) ) {
	  std::string msg = "Plug-In " + name + " unloaded.";
	  bz_sendTextMessage(BZ_SERVER,playerID,msg.c_str());
	}

	return true;
      }
      return true;
    }
};

DynamicPluginCommands	command;

void initPlugins ( void )
{
  customPluginMap.clear();

  registerCustomSlashCommand("loadplugin",&command);
  registerCustomSlashCommand("unloadplugin",&command);
  registerCustomSlashCommand("listplugins",&command);

  InitHTTP();
}

bool registerCustomPluginHandler ( std::string exte, bz_APIPluginHandler *handler )
{
  std::string ext = TextUtils::tolower(exte);
  customPluginMap[ext] = handler;
  return true;
}

bool removeCustomPluginHandler ( std::string ext, bz_APIPluginHandler *handler )
{
  tmCustomPluginMap::iterator itr = customPluginMap.find(TextUtils::tolower(ext));

  if (itr == customPluginMap.end() || itr->second != handler)
    return false;

  customPluginMap.erase(itr);
  return true;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
