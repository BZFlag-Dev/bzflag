/* bzflag
* Copyright (c) 1993 - 2004 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifdef _MSC_VER
#pragma warning( 4: 4786 )
#endif

#include "DisplayListManager.h"

/*const int NO_VARIANT = (-1); */

// initialize the singleton
template <>
DisplayListManager* Singleton<DisplayListManager>::_instance = (DisplayListManager*)0;

DisplayListManager::DisplayListManager()
{
  lists.clear();
  lastID = -1;
}

DisplayListManager::DisplayListManager(const DisplayListManager &dm)
  : Singleton<DisplayListManager>()
{
  release();
  lists = dm.lists;
  lastID = dm.lastID;
  acquire();
}

DisplayListManager& DisplayListManager::operator=(const DisplayListManager &dm)
{
  release();
  lists = dm.lists;
  lastID = dm.lastID;
  acquire();
  return *this;
}

DisplayListManager::~DisplayListManager()
{
  lists.clear();
}

int DisplayListManager::newList ( )
{

  displayListItem newItem;
  newItem.list = glGenLists(1);
  int id = ++lastID;
  lists[id] = newItem;
  glNewList(newItem.list,GL_COMPILE);
  glEndList();
  return id;
}

void DisplayListManager::freeList ( int list )
{
  displayListMap::iterator  it = lists.find(list);
  if (it == lists.end())
    return;

  glDeleteLists(it->second.list,1);
  lists.erase(it);
}

bool DisplayListManager::callList ( int list )
{
  displayListMap::iterator  it = lists.find(list);
  if (it == lists.end())
    return false;

  if (it->second.list != _GL_INVALID_ID)
    glCallList(it->second.list);
  else
    return false;
  return true;
}

void DisplayListManager::release ( void )
{
  displayListMap::iterator  it = lists.begin();

  while (it != lists.end())
  {
    glDeleteLists(it->second.list,1);
    it->second.list = _GL_INVALID_ID;
    it++;
  }
}

void DisplayListManager::acquire ( void )
{
  displayListMap::iterator  it = lists.begin();

  while (it != lists.end())
  {
    it->second.list = glGenLists(1);
    glNewList(it->second.list,GL_COMPILE);
    glEndList();
    it++;
  }
}

void DisplayListManager::rebuildLists()
{
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

