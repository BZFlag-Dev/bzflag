/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __CUSTOMWORLD_H__
#define __CUSTOMWORLD_H__

/* interface header */
#include "WorldFileObject.h"

/* system interface headers */
#include <iostream>

/* local interface headers */
#include "WorldInfo.h"


class CustomWorld : public WorldFileObject
{
public:
  CustomWorld();
  virtual bool read(const char *cmd, std::istream&);
  virtual void writeToWorld(WorldInfo*) const;
  virtual bool usesGroupDef() { return false; }

protected:
  bool createWalls;
  double _size;
  double _fHeight;
};


struct CustomObjectMapData {
  CustomObjectMapData() : handler(NULL) {}
  CustomObjectMapData(bz_CustomMapObjectHandler* h,
                      const std::string& end)
  : handler(h)
  , endToken(end)
  {}
  bz_CustomMapObjectHandler* handler;
  std::string endToken;
};
typedef std::map<std::string, CustomObjectMapData> CustomObjectMap;
extern CustomObjectMap customObjectMap;


bool registerCustomMapObject(const char* object, const char* end,
                             bz_CustomMapObjectHandler *handler);
bool removeCustomMapObject ( const char* object );


#endif  /* __CUSTOMWORLD_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
