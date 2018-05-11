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

#include "plugin_groups.h"
#include "plugin_utils.h"

bool permInGroup ( const std::string &perm, bz_APIStringList* groupPerms )
{
  for (unsigned int i = 0; i < groupPerms->size(); i++)
  {
    if (strcasecmp(perm.c_str(),groupPerms->get(i).c_str()) == 0)
      return true;
  }
  return false;
}

std::vector<std::string> findGroupsWithPerms( const std::vector<std::string> &perms, bool skipLocal )
{
  std::vector<std::string> groupsWithPerms;

  bz_APIStringList* groupList = bz_getGroupList();

  if (groupList)
  {
    for ( unsigned int i = 0; i < groupList->size();i++)
    {
      std::string groupName = groupList->get(i).c_str();

      if (skipLocal && compare_nocase(groupName,"LOCAL.ADMIN") == 0)
	continue;

      bz_APIStringList *groupPerms = bz_getGroupPerms(groupName.c_str());
      if (groupPerms)
      {
	// see if any of the perms are NOT in this group.
	bool hasOneWithNoPerm = false;
	for (size_t p =0; p < perms.size(); p++)
	{
	  if (!permInGroup(perms[p],groupPerms))
	    hasOneWithNoPerm = true;
	}
	bz_deleteStringList(groupPerms);

	if (!hasOneWithNoPerm)
	  groupsWithPerms.push_back(groupName);
      }
    }
    bz_deleteStringList(groupList);
  }

  return groupsWithPerms;
}

std::vector<std::string> findGroupsWithPerm( const char* perm, bool skipLocal )
{
  std::string name;
  if (perm)
    name = perm;
  return findGroupsWithPerm(name,skipLocal);
}

std::vector<std::string> findGroupsWithPerm( const std::string &perm, bool skipLocal )
{
  std::vector<std::string> groupsWithPerms;

  bz_APIStringList* groupList = bz_getGroupList();

  if (groupList)
  {
    for ( unsigned int i = 0; i < groupList->size();i++)
    {
      std::string groupName = groupList->get(i).c_str();

      if (skipLocal && compare_nocase(groupName,"LOCAL.ADMIN") == 0)
	continue;

      bz_APIStringList *groupPerms = bz_getGroupPerms(groupName.c_str());
      if (groupPerms)
      {
	if (permInGroup(perm,groupPerms))
	  groupsWithPerms.push_back(groupName);

	bz_deleteStringList(groupPerms);
      }
    }
    bz_deleteStringList(groupList);
  }
  return groupsWithPerms;
}

std::vector<std::string> findGroupsWithAdmin( bool skipLocal )
{
  std::vector<std::string> perms;
  perms.push_back(bz_perm_kick);
  perms.push_back(bz_perm_ban);

  return findGroupsWithPerms(perms,skipLocal);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
