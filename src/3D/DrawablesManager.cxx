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

#include "DrawablesManager.h"

/*const int NO_VARIANT = (-1); */

// initialize the singleton
template <>
DrawablesManager* Singleton<DrawablesManager>::_instance = (DrawablesManager*)0;

DrawablesManager::DrawablesManager()
{
  list.clear();
}

DrawablesManager::DrawablesManager(const DrawablesManager &dm)
  : Singleton<DrawablesManager>()
{
  list = dm.list;
}

DrawablesManager& DrawablesManager::operator=(const DrawablesManager &dm)
{
  list = dm.list;
  return *this;
}

DrawablesManager::~DrawablesManager()
{
  list.clear();
}

// this is gonna be slow
void DrawablesManager::add (BaseDrawable* item, int texture, int pass,
			    int priority, void*)
{
  drawableItem  drawItem;
  drawItem.item = item;
  drawablesPassList::iterator pIt = list.find(pass);
  if (pIt == list.end())
  {
    drawablesTextureList  tList;
    list[priority] = tList;
    pIt = list.find(priority);
  }
  drawablesTextureList::iterator tIt = pIt->second.find(texture);
  if (tIt == pIt->second.end())
  {
    drawablesPriortiyList  oList;
    pIt->second[texture] = oList;
    tIt = pIt->second.find(priority);
  }
  drawablesPriortiyList::iterator oIt = tIt->second.find(priority);
  if (oIt == tIt->second.end())
  {
    drawablesList  iList;
    tIt->second[priority] = iList;
    oIt = tIt->second.find(priority);
  }
  oIt->second.push_back(drawItem);
}

void DrawablesManager::drawAll ( void )
{
  TextureManager  &tm = TextureManager::instance();

  drawablesPassList::iterator pIt = list.begin();
  while (pIt != list.end())
  {
    drawablesTextureList::iterator tIt = pIt->second.begin();
    while (tIt != pIt->second.end())
    {
      tm.bind(tIt->first);
      drawablesPriortiyList::iterator oIt = tIt->second.begin();
      while (oIt != tIt->second.end())
      {
	drawablesList::iterator iIt = oIt->second.begin();
	while (iIt != oIt->second.end())
	{
	  iIt->item->draw(tIt->first,pIt->first,oIt->first,iIt->param);
	  iIt++;
	}
	oIt++;
      }
      tIt++;
    }
    pIt++;
  }
}

void DrawablesManager::reset ( void )
{
  list.clear();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

