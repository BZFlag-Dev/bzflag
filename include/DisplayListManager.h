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
#ifndef _DISPLAY_LIST_MANAGER_H
#define _DISPLAY_LIST_MANAGER_H

#include "common.h"
#include "Singleton.h"
#include <map>
#include "bzfgl.h"

class DisplayListBuilder
{
public:
  virtual bool build ( void ){return false;}
};

typedef struct
{
  GLuint  list;
  DisplayListBuilder *builder;
}displayListItem;

typedef std::map<int, displayListItem> displayListMap;

class DisplayListManager : public Singleton<DisplayListManager>
{
public:
  int newList ( DisplayListBuilder *builder );
  void freeList ( int list );
  bool callList ( int list );

  void release( void );
  void aquire ( void );
protected:
  friend class Singleton<DisplayListManager>;

private:
  DisplayListManager();
  DisplayListManager(const DisplayListManager &dm);
  DisplayListManager& operator=(const DisplayListManager &dm);
  ~DisplayListManager();
  
  int               lastID;
  displayListMap    lists;
};

#endif//_DISPLAY_LIST_MANAGER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
