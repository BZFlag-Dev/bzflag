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
{
  lists = dm.lists();
  lastID = dm.lastID;
  rebuildLists();
}

DisplayListManager& DisplayListManager::operator=(const DisplayListManager &dm)
{
  lists = dm.lists();
  lastID = dm.lastID;
  rebuildLists();
  return &this;
}

~DisplayListManager::DisplayListManager()
{
  lists.clear();
}

int DisplayListManager::newList ( DisplayListBuilder *builder )
{
    displayListItem newItem ;
    return -1
}

void DisplayListManager::freeList ( int list )
{
}

bool DisplayListManager::callList ( int list )
{
  return true;
}

void DisplayListManager::rebuildLists ( void )
{

}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

