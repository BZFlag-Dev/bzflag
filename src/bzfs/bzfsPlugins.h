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

#ifndef _BZFS_PLUGINS_H_
#define _BZFS_PLUGINS_H_

#include "bzfsAPI.h"
#include "PlayerState.h"

void initPlugins ( void );

void loadPlugin ( std::string plugin, std::string config );
void unloadPlugin ( std::string plugin );
void unloadPlugins ( void );

std::vector<std::string> getPluginList ( void );

#endif //_BZFS_PLUGINS_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
