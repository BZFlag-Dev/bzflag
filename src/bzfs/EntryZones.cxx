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

#include "common.h"
#include "WorldInfo.h"
#include "EntryZones.h"
#include "CustomZone.h"
#include <string>

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

  float rnd = bzfrand();
  float total = 0.0f;
  QPairList::const_iterator vit;
  for (vit = qPairList.begin(); vit != qPairList.end(); ++vit) {
    total += vit->second;
    if (total > rnd)
      break;
  }

  if (vit == qPairList.end())
    return false; // ??

  int zoneIndex = vit->first;
  CustomZone *zone = ((CustomZone *) &zones[zoneIndex]);
  zone->getRandomPoint(pt);
  return true;
}
