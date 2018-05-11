/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _BZFS_PLUGINS_H_
#define _BZFS_PLUGINS_H_

#include "bzfsAPI.h"
#include "PlayerState.h"

void initPlugins ( void );

bool loadPlugin ( std::string plugin, std::string config );
bool unloadPlugin ( std::string plugin );
void unloadPlugins ( void );

bool registerCustomPluginHandler ( std::string extension, bz_APIPluginHandler *handler );
bool removeCustomPluginHandler ( std::string extension, bz_APIPluginHandler *handler );

std::vector<std::string> getPluginList ( void );

float getPluginMinWaitTime ( void );

void PushPendingHTTPWait ();
void PopPendingHTTPWait ();

bz_Plugin* getPlugin( const char* name );

extern std::string lastPluginDir;


#endif //_BZFS_PLUGINS_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
