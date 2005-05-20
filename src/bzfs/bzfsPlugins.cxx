/* bzflag
* Copyright (c) 1993 - 2005 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <string>
#include <vector>
#include "bzfio.h"
#include "common.h"
#include "TextUtils.h"
#include <vector>

#include "commands.h"
#include "bzfsAPI.h"

typedef struct 
{
	std::string plugin;

#ifdef _WIN32
	HINSTANCE	handle;
#else
	void*		handle;
#endif 
}trPluginRecord;

std::vector<trPluginRecord>	vPluginList;

void unload1Plugin ( int iPluginID ); 

#ifdef _WIN32
#include <windows.h>

void loadPlugin ( std::string plugin, std::string config )
{
	int (*lpProc)(const char*);

	HINSTANCE	hLib = LoadLibrary(plugin.c_str());
	if (hLib)
	{
		lpProc = (int (__cdecl *)(const char*))GetProcAddress(hLib, "bz_Load");
		if (lpProc)
		{
			int ret =lpProc(config.c_str()); 
			DEBUG1("Plugin:%s loaded\n",plugin.c_str());

			trPluginRecord pluginRecord;
			pluginRecord.handle = hLib;
			pluginRecord.plugin = plugin;
			vPluginList.push_back(pluginRecord);
		}
		else
			DEBUG1("Plugin:%s found but does not contain bz_Load method\n",plugin.c_str());
	}
	else
		DEBUG1("Plugin:%s not found\n",plugin.c_str());
}

void unload1Plugin ( int iPluginID )
{
	int (*lpProc)(void);

	trPluginRecord &plugin = vPluginList[iPluginID];

	lpProc = (int (__cdecl *)(void))GetProcAddress(plugin.handle, "bz_Unload");
	if (lpProc)
		int ret =lpProc(); 
	else
		DEBUG1("Plugin does not contain bz_UnLoad method\n");

	FreeLibrary(plugin.handle);
	plugin.handle = NULL;
}
#else

#include <dlfcn.h>
std::vector<void*>	vLibHandles;

void loadPlugin ( std::string plugin, std::string config )
{
	int (*lpProc)(const char*);

	void*	hLib = dlopen(plugin.c_str(),RTLD_LAZY);
	if (hLib)
	{
		*(void**) &lpProc = dlsym(hLib,"bz_Load");
		if (lpProc)
		{
			(*lpProc)(config.c_str());
			DEBUG1("Plugin:%s loaded\n",plugin.c_str());
			trPluginRecord pluginRecord;
			pluginRecord.handle = hLib;
			pluginRecord.plugin = plugin;
			vPluginList.push_back(pluginRecord);
		}
		else
			DEBUG1("Plugin:%s found but does not contain bz_Load method, error %s\n",plugin.c_str(),dlerror());
	}
	else
		DEBUG1("Plugin:%s not found, error %s\n",plugin.c_str(), dlerror());
}

void unload1Plugin ( int iPluginID )
{
	int (*lpProc)(void);
	trPluginRecord &plugin = vPluginList[iPluginID];

	*(void**) &lpProc = dlsym(plugin.handle, "bz_Unload");
	if (lpProc)
		(*lpProc)(); 
	else
		DEBUG1("Plugin does not contain bz_UnLoad method, error %s\n",dlerror());

	dlclose(plugin.handle);
	plugin.handle = NULL;
}
#endif 

void unloadPlugin ( std::string plugin )
{
	// unload the first one of the name we find
	for (unsigned int i = 0; i < vPluginList.size();i++)
	{
		if ( vPluginList[i].plugin == plugin )
		{
			unload1Plugin(i);
			vPluginList.erase(vPluginList.begin()+i);
			return;
		}
	}
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

class DynamicPluginCommands : public CustomSlashCommandHandler
{
public:
	virtual ~DynamicPluginCommands(){};
	virtual bool handle ( int playerID, std::string command, std::string message )
	{
		bz_PlayerRecord	record;

		bz_getPlayerByIndex(playerID,&record);

		if ( !record.admin )
		{
			bz_sendTextMessage(BZ_SERVER,playerID,"Permision denied: ADMIN");
			return true;
		}

		if ( !message.size() )
		{
			bz_sendTextMessage(BZ_SERVER,playerID,"Error: Command must have a plugin name");
			return true;
		}

		if ( TextUtils::tolower(command) == "loadplugin" )
		{
			std::vector<std::string> params = TextUtils::tokenize(message,std::string(","));

			std::string config;
			if ( params.size() >1)
				config = params[1];

			loadPlugin(params[0],config);

			bz_sendTextMessage(BZ_SERVER,playerID,"Plug-in loaded");
			return true;
		}

		if ( TextUtils::tolower(command) == "unloadplugin" )
		{
			unloadPlugin(message);

			bz_sendTextMessage(BZ_SERVER,playerID,"Plug-in unloaded");
			return true;
		}

		if ( TextUtils::tolower(command) == "listplugins" )
		{
			std::vector<std::string>	plugins = getPluginList();

			if (!plugins.size())
				bz_sendTextMessage(BZ_SERVER,playerID,"No Plug-ins loaded;");
			else
			{
				bz_sendTextMessage(BZ_SERVER,playerID,"Plug-isn loaded;");

				for ( unsigned int i = 0; i < plugins.size(); i++)
					bz_sendTextMessage(BZ_SERVER,playerID,plugins[i].c_str());
			}
			return true;
		}
		return true;
	}
};

DynamicPluginCommands	command;

void initPlugins ( void )
{
	registerCustomSlashCommand("loadplugin",&command);
	registerCustomSlashCommand("unloadplugin",&command);
	registerCustomSlashCommand("listplugins",&command);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
