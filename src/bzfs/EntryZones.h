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

#ifndef __ENTRYZONES_H__
#define __ENTRYZONES_H__

#include <vector>
#include <map>
#include "WorldFileLocation.h"

class CustomZone;

typedef std::vector<WorldFileLocation> ZoneList;
typedef std::vector<std::pair<int,float> > QPairList;
typedef std::map<std::string, QPairList> QualifierMap;

class EntryZones
{
public:
  EntryZones();
  void addZone( const CustomZone *zone );
  void calculateQualifierLists();
  bool getZonePoint( const std::string &qualifier, float *pt ) const;
private:
  ZoneList zones;
  QualifierMap qmap;
};

#endif

