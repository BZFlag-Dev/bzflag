/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include <iostream>
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
#include "OSFile.h"


#ifdef _WIN32
std::string extension = ".dll";
std::string globalPluginDir = ".\\plugins\\";
#else
std::string extension = ".so";
#  ifdef INSTALL_LIB_DIR
std::string globalPluginDir = INSTALL_LIB_DIR;
#  else
std::string globalPluginDir = ".";
#  endif
#endif

typedef std::map<std::string, bz_APIPluginHandler*> tmCustomPluginMap;
tmCustomPluginMap customPluginMap;

typedef struct
{
  std::string foundPath;
  std::string plugin;

#ifdef _WIN32
  HINSTANCE	handle;
#else
  void*		handle;
#endif
}trPluginRecord;

std::vector<trPluginRecord>	vPluginList;

typedef enum
  {
    eLoadFailedDupe = -1,
    eLoadFailedError = 0,
    eLoadFailedRuntime,
    eLoadComplete
  }PluginLoadReturn;

bool pluginExists ( std::string plugin )
{
  for ( int i = 0; i < (int)vPluginList.size(); i++ ) {
    if ( vPluginList[i].foundPath == plugin )
      return true;
  }
  return false;
}


std::string findPlugin ( std::string pluginName )
{
  // see if we can just open the bloody thing
  FILE	*fp = fopen(pluginName.c_str(),"rb");
  if (fp) {
    fclose(fp);
    return pluginName;
  }

  // now try it with the standard extension
  std::string name = pluginName + extension;
  fp = fopen(name.c_str(),"rb");
  if (fp) {
    fclose(fp);
    return name;
  }

  // check the local users plugins dir
  name = getConfigDirName(BZ_CONFIG_DIR_VERSION) + pluginName + extension;
  fp = fopen(name.c_str(),"rb");
  if (fp) {
    fclose(fp);
    return name;
  }

  // check the global plugins dir
  name = globalPluginDir + pluginName + extension;
  fp = fopen(name.c_str(),"rb");
  if (fp) {
    fclose(fp);
    return name;
  }

  return std::string("");
}

void unload1Plugin ( int iPluginID );

#ifdef _WIN32
#  include <windows.h>

int getPluginVersion ( HINSTANCE hLib )
{
  int (*lpProc)(void);
  lpProc = (int (__cdecl *)(void))GetProcAddress(hLib, "bz_GetVersion");
  if (lpProc)
    return lpProc();
  return 0;
}

PluginLoadReturn load1Plugin ( std::string plugin, std::string config )
{
  int (*lpProc)(const char*);

  std::string realPluginName = findPlugin(plugin);
  if (pluginExists(realPluginName)) {
    logDebugMessage(1,"LoadPlugin failed: %s is already loaded\n",realPluginName.c_str());
    return eLoadFailedDupe;
  }

  HINSTANCE	hLib = LoadLibrary(realPluginName.c_str());
  if (hLib) {
    if (getPluginVersion(hLib) > BZ_API_VERSION) {
      logDebugMessage(1,"Plugin:%s found but expects an newer API version (%d), upgrade your bzfs\n",plugin.c_str(),getPluginVersion(hLib));
      FreeLibrary(hLib);
      return eLoadFailedError;
    } else {
      lpProc = (int (__cdecl *)(const char*))GetProcAddress(hLib, "bz_Load");
      if (lpProc) {
	if (lpProc(config.c_str())!= 0) {
	  logDebugMessage(1,"Plugin:%s found but bz_Load returned an error\n",plugin.c_str());
	  FreeLibrary(hLib);
	  return eLoadFailedRuntime;
	}
	logDebugMessage(1,"Plugin:%s loaded\n",plugin.c_str());

	trPluginRecord pluginRecord;
	pluginRecord.foundPath = realPluginName;
	pluginRecord.handle = hLib;
	pluginRecord.plugin = plugin;
	vPluginList.push_back(pluginRecord);
      } else {
	logDebugMessage(1,"Plugin:%s found but does not contain bz_Load method\n",plugin.c_str());
	FreeLibrary(hLib);
	return eLoadFailedError;
      }
    }
  } else {
    logDebugMessage(1,"Plugin:%s not found\n",plugin.c_str());
    return eLoadFailedError;
  }

  return eLoadComplete;
}

void unload1Plugin ( int iPluginID )
{
  int (*lpProc)(void);

  trPluginRecord &plugin = vPluginList[iPluginID];
  if (!plugin.handle)
    return;

  lpProc = (int (__cdecl *)(void))GetProcAddress(plugin.handle, "bz_Unload");
  if (lpProc)
    lpProc();
  else
    logDebugMessage(1,"Plugin does not contain bz_UnLoad method\n");

  FreeLibrary(plugin.handle);
  plugin.handle = NULL;
  plugin.foundPath = "";
  plugin.plugin = "";
}
#else

#  include <dlfcn.h>
std::vector<void*>	vLibHandles;

int getPluginVersion ( void* hLib )
{
  int (*lpProc)(void);

  lpProc = force_cast<int (*)(void)>(dlsym(hLib,"bz_GetVersion"));
  if (lpProc)
    return (*lpProc)();
  return 0;
}

PluginLoadReturn load1Plugin ( std::string plugin, std::string config )
{
  int (*lpProc)(const char*);

  std::string realPluginName = findPlugin(plugin);

  if (pluginExists(realPluginName)) {
    logDebugMessage(1,"LoadPlugin failed: %s is already loaded\n",realPluginName.c_str());
    return eLoadFailedDupe;
  }

  void *hLib = dlopen(realPluginName.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  if (hLib) {
    if (dlsym(hLib, "bz_Load") == NULL) {
      logDebugMessage(1,"Plugin:%s found but does not contain bz_Load method, error %s\n",plugin.c_str(),dlerror());
      dlclose(hLib);
      return eLoadFailedError;
    }

    int version = getPluginVersion(hLib);
    if (version < BZ_API_VERSION) {
      logDebugMessage(1,"Plugin:%s found but expects an older API version (%d), upgrade it\n", plugin.c_str(), version);
      dlclose(hLib);
      return eLoadFailedError;
    } else {
      lpProc = force_cast<int (*)(const char*)>(dlsym(hLib,"bz_Load"));
      if (lpProc) {
	if((*lpProc)(config.c_str())) {
	  logDebugMessage(1,"Plugin:%s found but bz_Load returned an error\n",plugin.c_str());
	  return eLoadFailedRuntime;
	}
	logDebugMessage(1,"Plugin:%s loaded\n",plugin.c_str());
	trPluginRecord pluginRecord;
	pluginRecord.handle = hLib;
	pluginRecord.plugin = plugin;
	vPluginList.push_back(pluginRecord);
	return eLoadComplete;
      }
    }
  } else {
    logDebugMessage(1,"Plugin:%s not found, error %s\n",plugin.c_str(), dlerror());
    return eLoadFailedError;
  }

  logDebugMessage(1,"If you see this, there is something terribly wrong.\n");
  return eLoadFailedError;
}

void unload1Plugin ( int iPluginID )
{
  int (*lpProc)(void);
  trPluginRecord &plugin = vPluginList[iPluginID];

  if(!plugin.handle)
    return;

  lpProc = force_cast<int (*)(void)>(dlsym(plugin.handle, "bz_Unload"));
  if (lpProc)
    (*lpProc)();
  else
    logDebugMessage(1,"Plugin does not contain bz_UnLoad method, error %s\n",dlerror());

  dlclose(plugin.handle);
  plugin.handle = NULL;
  plugin.foundPath = "";
  plugin.plugin = "";

}
#endif


bool loadPlugin ( std::string plugin, std::string config )
{
  // check and see if it's an extension we have a handler for
  std::string ext;

  std::vector<std::string> parts = TextUtils::tokenize(plugin,std::string("."));
  ext = parts[parts.size()-1];

  tmCustomPluginMap::iterator itr = customPluginMap.find(TextUtils::tolower(ext));

  if (itr != customPluginMap.end() && itr->second) {
    bz_APIPluginHandler *handler = itr->second;
    return handler->handle(plugin,config);
  }
  else
    return load1Plugin(plugin,config) == eLoadComplete;
}

bool unloadPlugin ( std::string plugin )
{
  // unload the first one of the name we find
  for (unsigned int i = 0; i < vPluginList.size();i++) {
    if ( vPluginList[i].plugin == plugin ) {
      unload1Plugin(i);
      vPluginList.erase(vPluginList.begin()+i);
      return true;
    }
  }
  return false;
}

void unloadPlugins ( void )
{
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
    plugins.push_back(vPluginList[i].plugin);

  return plugins;
}

void parseServerCommand(const char *message, int dstPlayerId);

class DynamicPluginCommands : public bz_CustomSlashCommandHandler
{
public:
  virtual ~DynamicPluginCommands() {};
  virtual bool handle ( int playerID, bz_ApiString _command, bz_ApiString _message, bz_APIStringList *params )
  {
    bz_BasePlayerRecord	record;

    std::string command = _command.c_str();
    std::string message = _message.c_str();

    bz_BasePlayerRecord	*p = bz_getPlayerByIndex(playerID);
    if (!p)
      return false;

    record = *p;

    bz_freePlayerRecord(p);

    // list needs listPlugins permission
    if ( TextUtils::tolower(command) == "listplugins" ) {
      if (!bz_hasPerm(playerID, "listPlugins")) {
	bz_sendTextMessage(BZ_SERVER,playerID,"You do not have permission to run the /listplugins command");
	return true;
      } else {
	std::vector<std::string>	plugins = getPluginList();

	if (!plugins.size())
	  bz_sendTextMessage(BZ_SERVER,playerID,"No Plug-ins loaded.");
	else {
	  bz_sendTextMessage(BZ_SERVER,playerID,"Plug-ins loaded:");

	  for ( unsigned int i = 0; i < plugins.size(); i++)
	    bz_sendTextMessage(BZ_SERVER,playerID,plugins[i].c_str());
	}
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

      if ( unloadPlugin(std::string(params->get(0).c_str())) )
	bz_sendTextMessage(BZ_SERVER,playerID,"Plug-in unloaded.");

      return true;
    }
    return true;
  }
};

DynamicPluginCommands	command;


// auto load plugin dir

std::string getAutoLoadDir ( void )
{
  if (BZDB.isSet("PlugInAutoLoadDir"))
    return BZDB.get("PlugInAutoLoadDir");
#if (defined(_WIN32) || defined(WIN32))
  char exePath[MAX_PATH];
  GetModuleFileName(NULL,exePath,MAX_PATH);
  char* last = strrchr(exePath,'\\');
  if (last)
    *last = '\0';
  strcat(exePath,"\\plugins");
  return std::string(exePath);
#else
  return std::string("");
#endif
}

void initPlugins ( void )
{
  customPluginMap.clear();

  registerCustomSlashCommand("loadplugin",&command);
  registerCustomSlashCommand("unloadplugin",&command);
  registerCustomSlashCommand("listplugins",&command);

#ifdef _WIN32
#ifdef _DEBUG 
  OSDir	dir;
  std::string path = getAutoLoadDir();
  if (getAutoLoadDir().size()) {
    dir.setOSDir(getAutoLoadDir());

    OSFile file;
    while(dir.getNextFile(file,"*.dll",false) )
      loadPlugin(file.getOSName(),std::string(""));
  }
#endif //_DEBUG
#endif
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
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
