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

#include "EntryZones.h"
#include "CustomZone.h"

#include <string>
#include <vector>

#include "common.h"
#include "global.h"
#include "Protocol.h"
#include "Flag.h"
#include "Team.h"

#include "WorldInfo.h"


EntryZones::EntryZones()
{
}

void EntryZones::addZone(const CustomZone *zone)
{
  //We're purposely slicing off part of the zone structure
  zones.push_back( *((WorldFileLocation *)zone) );

  QualifierList qualifiers = zone->getQualifiers();
  for (QualifierList::iterator it = qualifiers.begin(); it != qualifiers.end(); ++it) {
    std::string qualifier = *it;
    QPairList &qPairList = qmap[qualifier];
    qPairList.push_back(std::pair<int,float>( zones.size()-1, 0.0f));
  }
}

void EntryZones::calculateQualifierLists()
{
  for (QualifierMap::iterator mit = qmap.begin(); mit != qmap.end(); ++mit) {
    QPairList &qPairList = mit->second;
    float total = 0.0f;
    QPairList::iterator vit;
    for (vit = qPairList.begin(); vit != qPairList.end(); ++vit) {
      std::pair<int,float> &p = *vit;
      int zoneIndex = p.first;
      p.second = ((CustomZone *) &zones[zoneIndex])->getArea();
      total += p.second;
    }
    for (vit = qPairList.begin(); vit != qPairList.end(); ++vit) {
      std::pair<int,float> &p = *vit;
      p.second /= total;
    }
  }
}

bool EntryZones::getZonePoint(const std::string &qualifier, float *pt) const
{
  QualifierMap::const_iterator mit = qmap.find(qualifier);
  if (mit == qmap.end())
    return false;

  const QPairList &qPairList = mit->second;

  float rnd = (float)bzfrand();
  float total = 0.0f;
  QPairList::const_iterator vit;
  for (vit = qPairList.begin(); vit != qPairList.end(); ++vit) {
    total += vit->second;
    if (total > rnd)
      break;
  }

  if (vit == qPairList.end()) {
    return false; // ??
  }

  int zoneIndex = vit->first;
  CustomZone *zone = ((CustomZone *) &zones[zoneIndex]);
  zone->getRandomPoint(pt);
  return true;
}

bool EntryZones::getSafetyPoint( const std::string &qualifier,
                                 const float *pos, float *pt ) const
{
  std::string safetyString = EntryZones::getSafetyPrefix() + qualifier;

  QualifierMap::const_iterator mit = qmap.find(safetyString);
  if (mit == qmap.end())
    return false;

  const QPairList &qPairList = mit->second;

  int closest = -1;
  float minDist = +Infinity;
  QPairList::const_iterator vit;
  for (vit = qPairList.begin(); vit != qPairList.end(); ++vit) {
    int index = vit->first;
    CustomZone *zone = ((CustomZone *) &zones[index]);
    float dist = zone->getDistToPoint (pos);
    if (dist < minDist) {
      closest = index;
      minDist = dist;
    }
  }

  if (closest == -1) {
    return false;
  }

  CustomZone *zone = ((CustomZone *) &zones[closest]);
  zone->getRandomPoint(pt);

  return true;
}

const char * EntryZones::getSafetyPrefix ()
{
  return "$";
}


static int matchTeamColor(const char *teamText)
{
  for (int i = 0; i < CtfTeams; i++) {
    if (strcmp (teamText, Team::getName((TeamColor)i)) == 0) {
      return i;
    }
  }
  return -1;
}

void EntryZones::makeSplitLists (int zone,
                                 std::vector<FlagType*> &flags,
                                 std::vector<TeamColor> &teams,
                                 std::vector<TeamColor> &safety) const
{
  flags.clear();
  teams.clear();
  safety.clear();

  QualifierMap::const_iterator mit;
  for (mit = qmap.begin(); mit != qmap.end(); ++mit) {
    const QPairList &qPairList = mit->second;
    QPairList::const_iterator vit;
    for (vit = qPairList.begin(); vit != qPairList.end(); ++vit) {
      const std::pair<int,float> &p = *vit;
      int zoneIndex = p.first;
      if (zoneIndex == zone) {
        int team;
        FlagType *type = Flag::getDescFromAbbreviation(mit->first.c_str());
        if (type != Flags::Null) {
          flags.push_back (type);
        }
        else if ((team = matchTeamColor(mit->first.c_str())) != -1) {
          teams.push_back ((TeamColor)team);
        }
        else if ((mit->first.c_str()[0] == getSafetyPrefix()[0]) &&
                 ((team = matchTeamColor(mit->first.c_str()+1)) != -1)) {
          safety.push_back ((TeamColor)team);
        }
        else {
          printf ("EntryZones::makeSplitLists() ERROR on (%s)\n", mit->first.c_str());
        }
      }
    }
  }

  return;
}

int EntryZones::packSize() const
{
  int size = 0;
  for (unsigned int i=0; i < zones.size(); i++) {
    std::vector<FlagType*> flags;
    std::vector<TeamColor> teams;
    std::vector<TeamColor> safety;
    makeSplitLists (i, flags, teams, safety);
    size += 2 + 2 + WorldCodeZoneSize;
    size += FlagType::packSize * flags.size();
    size += 2 * teams.size();
    size += 2 * safety.size();
  }
  return size;
}

void * EntryZones::pack(void *buf) const
{
  for (unsigned int i=0 ; i < zones.size(); i++) {
    unsigned int j;
    const WorldFileLocation& z = (const WorldFileLocation) zones[i];
    std::vector<FlagType*> flags;
    std::vector<TeamColor> teams;
    std::vector<TeamColor> safety;
    makeSplitLists (i, flags, teams, safety);
    void *bufStart = buf;
    buf = nboPackUShort(buf, 0); // place-holder
    buf = nboPackUShort(buf, WorldCodeZone);
    buf = z.pack (buf);                         // 12 + 12 + 4
    buf = nboPackUShort(buf, flags.size());     // 30
    buf = nboPackUShort(buf, teams.size());     // 32
    buf = nboPackUShort(buf, safety.size());    // 34
    for (j = 0; j < flags.size(); j++) {
      buf = flags[j]->pack(buf);
    }
    for (j = 0; j < teams.size(); j++) {
      buf = nboPackUShort(buf, teams[j]);
    }
    for (j = 0; j < safety.size(); j++) {
      buf = nboPackUShort(buf, safety[j]);
    }
    uint16_t len = (char*)buf - (char*)bufStart;
    nboPackUShort (bufStart, len - (2 * sizeof(uint16_t)));
  }
  return buf;
}

