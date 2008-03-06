/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __ENTRYZONES_H__
#define __ENTRYZONES_H__

/* common header */
#include "common.h"

/* common interface headers */
#include "Flag.h"

// bzfs-specific headers
#include "CustomZone.h"


typedef std::vector<CustomZone> ZoneList;
typedef std::vector<std::pair<int,float> > QPairList;
typedef std::map<std::string, QPairList> QualifierMap;


class EntryZones
{
  public:
    EntryZones();

    void addZone( const CustomZone *zone );
    void addZoneFlag(int zone, int flagId);

    void calculateQualifierLists();

    bool getZonePoint(const std::string &qualifier, float *pt) const;
    bool getSafetyPoint(const std::string &qualifier,
			const float *pos, float *pt) const;

    bool getRandomPoint(const std::string &qual, float *pt) const;
    bool getClosePoint(const std::string &qual, const float pos[3],
		       float *pt) const;

    const ZoneList& getZoneList() const;

    int packSize() const;
    void *pack(void *buf) const;

  private:
    ZoneList zones;
    QualifierMap qmap;

    void makeSplitLists (int zone,
			 std::vector<FlagType*> &flags,
			 std::vector<TeamColor> &teams,
			 std::vector<TeamColor> &safety) const;
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

