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
#ifndef _DRAWABLES_MANAGER_H
#define _DRAWABLES_MANAGER_H

#include "common.h"
#include "Singleton.h"
#include <map>
#include <string>
#include <vector>
#include "TextUtils.h"
#include "TextureManager.h"

// base class for all drawable effects
class BaseDrawable
{
public:
  // virtual bool draw ( int texture, int pass, int priority, void* param ){return false;}
  virtual bool draw (int, int, int, void*){return false;}
};

// items for sorted list
// this may be slow per frame
// TODO, staticly allocate some of the stuff
typedef struct
{
  void          *param;
  BaseDrawable  *item;
}drawableItem;

typedef std::vector<drawableItem> drawablesList;
typedef std::map<int, drawablesList> drawablesPriortiyList;
typedef std::map<int,drawablesPriortiyList> drawablesTextureList;
typedef std::map<int,drawablesTextureList> drawablesPassList;

class DrawablesManager : public Singleton<DrawablesManager>
{
public:
  void add ( BaseDrawable* item, int texture = -1, int pass = 0, int priority = 0, void* param = NULL );
  void drawAll ( void );
  void reset ( void );

  unsigned int getPasses ( void ){ return (unsigned int)list.size();}
protected:
  friend class Singleton<DrawablesManager>;

private:
  DrawablesManager();
  DrawablesManager(const DrawablesManager &dm);
  DrawablesManager& operator=(const DrawablesManager &dm);
  ~DrawablesManager();
  
  drawablesPassList list;
};

#endif//_DRAWABLES_MANAGER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
