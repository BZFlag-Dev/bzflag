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

// a series of utilitys for bzfs plugins to use with permision groups
#ifndef _PLUGIN_GROUPS_H_
#define _PLUGIN_GROUPS_H_

#include "bzfsAPI.h"

// return a list of groups that have ALL the perms in the list passed in
std::vector<std::string> findGroupsWithPerms( const std::vector<std::string> &perms, bool skipLocal = true );

// return a list of groups that have the perm passed in
std::vector<std::string> findGroupsWithPerm( const std::string &perm, bool skipLocal = true );
std::vector<std::string> findGroupsWithPerm( const char* perm, bool skipLocal = true );

// return a list of groups that have kick and ban
std::vector<std::string> findGroupsWithAdmin( bool skipLocal = true );

#endif //_PLUGIN_GROUPS_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
