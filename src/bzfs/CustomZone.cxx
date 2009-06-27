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

// the common header
#include "common.h"

// interface header
#include "CustomZone.h"

// system headers
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sstream>
#include <math.h>

// common headers
#include "MeshFace.h"
#include "TextUtils.h"

// local headers
//#include "EntryZones.h"
#include "WorldInfo.h"
#include "Flag.h"
#include "Team.h"


CustomZone::CustomZone()
{
  pos = fvec3(0.0f, 0.0f, 0.0f);
  size = fvec3(1.0f, 1.0f, 1.0f);
  rotation = 0.0f;
  useCenter = 0.0f;
  face = NULL;
  faceHeight = 1.0f;
  faceWeight = 0.0f;
}


CustomZone::CustomZone(const MeshFace* f)
{
  face = f;
  pos = fvec3(0.0f, 0.0f, 0.0f);
  size = fvec3(1.0f, 1.0f, 1.0f);
  rotation = 0.0f;
}


// make a safety zone for all team flags
void CustomZone::addFlagSafety(float x, float y, WorldInfo* worldInfo)
{
  pos = fvec3(x, y, 0.0f);
  size = fvec3(0.0f, 0.0f, 0.0f);
  rotation = 0.0f;

  // add the qualifiers
  for (int team = 0; team < CtfTeams; team++) {
    std::string qual = getFlagSafetyQualifier(team);
    if (qual.size() > 0) {
      qualifiers.push_back(qual);
    }
  }

  worldInfo->addZone(this);

  return;
}


void CustomZone::addZoneFlagCount(FlagType* flagType, int count)
{
  ZoneFlagMap::iterator it = zoneFlagMap.find(flagType);
  if (it != zoneFlagMap.end()) {
    count += it->second;
  }
  if (count < 0) {
    count = 0;
  }
  zoneFlagMap[flagType] = count;
  return;
}


bool CustomZone::read(const char *cmd, std::istream& input)
{
  const std::string lower = TextUtils::tolower(cmd);
  if ((lower == "flag") ||
      (lower == "zoneflag") || (lower == "fixedflag") ||
      (lower == "team") || (lower == "safety") ||
      (lower == "height") || (lower == "weight") || (lower == "center")) {
    std::string line;
    std::getline(input, line);
    input.putback('\n');
    return readLine(cmd, line);
  }  
  else if (!face) {
    return WorldFileLocation::read(cmd, input);
  }
  else {
    return false;
  }
}

  
bool CustomZone::readLine(const std::string& cmd, const std::string& line)
{
  const std::string lower = TextUtils::tolower(cmd);

  std::istringstream parms(line);

  if (lower == "flag") {
    std::string flag;
    while (parms >> flag) {
      if (flag == "good") {
	FlagSet &fs = Flag::getGoodFlags();
	for (FlagSet::iterator it = fs.begin(); it != fs.end(); ++it) {
	  FlagType *f = *it;
	  if (f->endurance != FlagNormal) { // Null and Team flags
	    const std::string& qual = getFlagTypeQualifier(f);
	    if (qual.size() > 0) {
	      qualifiers.push_back(qual);
	    }
	  }
	}
      }
      else if (flag == "bad") {
	FlagSet &fs = Flag::getBadFlags();
	for (FlagSet::iterator it = fs.begin(); it != fs.end(); ++it) {
	  FlagType *f = *it;
	  if (f->endurance != FlagNormal) { // Null and Team flags
	    const std::string& qual = getFlagTypeQualifier(f);
	    if (qual.size() > 0) {
	      qualifiers.push_back(qual);
	    }
	  }
	}
      }
      else {
	FlagType* f = Flag::getDescFromAbbreviation(flag.c_str());
	if (f == Flags::Null) {
	  logDebugMessage(0, "WARNING: bad flag type: %s\n", flag.c_str());
	  return false;
	}
	if (f->endurance == FlagNormal) {
	  logDebugMessage(0, "WARNING: you probably want a safety: %s\n", flag.c_str());
	  return false;
	}
	const std::string& qual = getFlagTypeQualifier(f);
	if (qual.size() > 0) {
	  qualifiers.push_back(qual);
	}
      }
    }

    if (qualifiers.size() == 0) {
      return false;
    }
  }
  else if ((lower == "zoneflag") || (lower == "fixedflag")) {
    std::string flag;
    int count;

    if (!(parms >> flag)) {
      return false;
    }
    if (!(parms >> count)) {
      count = 1;
    }

    if (flag == "good") {
      FlagSet &fs = Flag::getGoodFlags();
      for (FlagSet::iterator it = fs.begin(); it != fs.end(); ++it) {
	FlagType *f = *it;
	if (f->endurance != FlagNormal) { // Null and Team flags
	  addZoneFlagCount(f, count);
	}
      }
    }
    else if (flag == "bad") {
      FlagSet &fs = Flag::getBadFlags();
      for (FlagSet::iterator it = fs.begin(); it != fs.end(); ++it) {
	FlagType *f = *it;
	if (f->endurance != FlagNormal) { // Null and Team flags
	  addZoneFlagCount(f, count);
	}
      }
    }
    else {
      FlagType *f = Flag::getDescFromAbbreviation(flag.c_str());
      if (f != Flags::Null) {
	addZoneFlagCount(f, count);
      } else {
	logDebugMessage(0, "WARNING: bad zoneflag type: %s\n", flag.c_str());
	return false;
      }
    }
  }
  else if ((lower == "team") || (lower == "safety")) {
    int color;
    const bool safety = (lower == "safety");

    while (parms >> color) {
      if ((color < 0) || (color >= CtfTeams)) {
	logDebugMessage(0, "WARNING: bad team number: %i\n", color);
	return false;
      }
      std::string qual;
      if (safety) {
	qual = getFlagSafetyQualifier(color);
      } else {
	qual = getPlayerTeamQualifier(color);
      }
      if (qual.size() > 0) {
	qualifiers.push_back(qual);
      }
    }
    if (qualifiers.size() == 0) {
      return false;
    }
  }
  else if ((face != NULL) && (lower == "height")) {
    if (!(parms >> faceHeight)) {
      logDebugMessage(0, "WARNING: invalid zone height\n");
      return false;
    }
  }
  else if ((face != NULL) && (lower == "weight")) {
    if (!(parms >> faceWeight)) {
      logDebugMessage(0, "WARNING: invalid zone weight\n");
      return false;
    }
  }
  else if (lower == "center") {
    useCenter = true;
  }

  return true;
}


void CustomZone::writeToWorld(WorldInfo* worldInfo) const
{
  worldInfo->addZone( this );
}


void CustomZone::getRandomPoint(fvec3& pt) const
{
  if (face) {
    pt = useCenter ? face->calcCenter() : face->getRandomPoint();
    return;
  }

  if (useCenter) {
    pt = pos;
    return;
  }

  const float x = (float)((bzfrand() * (2.0f * size.x)) - size.x);
  const float y = (float)((bzfrand() * (2.0f * size.y)) - size.y);

  const float cos_val = cosf(rotation);
  const float sin_val = sinf(rotation);

  pt = pos;
  pt.x += (x * cos_val) - (y * sin_val);
  pt.y += (x * sin_val) + (y * cos_val);
}


float CustomZone::getDistToPoint (const fvec3& _pos) const
{
  if (face) {
    return (face->calcCenter() - _pos).length();
  }
  // FIXME - should use proper minimum distance from
  // the zone edge, and maybe -1.0f if its inside the zone
  return (pos - _pos).length();
}


static std::string makeIntQualifier(char prefix, int value)
{
  static std::string qual;
  char buffer[16];
  snprintf(buffer, 16, "%c%i", prefix, value);
  qual = buffer;
  return qual;
}

static int checkIntQualifier(char prefix, const std::string& qual)
{
  if (qual[0] == prefix) {
    const char* start = qual.c_str() + 1;
    char* end;
    int id = strtol(start, &end, 10);
    if (end != start) {
      return id;
    }
  }
  return -1;
}


const std::string& CustomZone::getFlagIdQualifier(int flagId)
{
  static std::string qual;
  qual = makeIntQualifier('#', flagId);
  return qual;
}

int CustomZone::getFlagIdFromQualifier(const std::string& qual)
{
  return checkIntQualifier('#', qual);
}


const std::string& CustomZone::getFlagTypeQualifier(FlagType* flagType)
{
  static std::string qual;
  if (flagType != NULL) {
    qual = "f";
    qual += flagType->flagAbbv;
  } else {
    qual = "";
  }
  return qual;
}

FlagType* CustomZone::getFlagTypeFromQualifier(const std::string& qual)
{
  if (qual[0] == 'f') {
    return Flag::getDescFromAbbreviation(qual.c_str() + 1);
  } else {
    return Flags::Null;
  }
}


const std::string& CustomZone::getFlagSafetyQualifier(int team)
{
  static std::string qual;
  if ((team > 0) && (team < CtfTeams)) {
    qual = makeIntQualifier('$', team);
  } else {
    qual = "";
  }
  return qual;
}


int CustomZone::getFlagSafetyFromQualifier(const std::string& qual)
{
  return checkIntQualifier('$', qual);
}


const std::string& CustomZone::getPlayerTeamQualifier(int team)
{
  static std::string qual;
  if ((team >= 0) && (team < CtfTeams)) {
    qual = makeIntQualifier('t', team);
  } else {
    qual = "";
  }
  return qual;
}


int CustomZone::getPlayerTeamFromQualifier(const std::string& qual)
{
  return checkIntQualifier('t', qual);
}


float CustomZone::getWeight() const
{
  if (face) {
    if (faceWeight > 0.0f) {
      return faceWeight;
    }
    float area = face->calcArea();
    area *= (faceHeight >= 1.0f) ? faceHeight : 1.0f;
    return (area >= 1.0f) ? area : 1.0f;
  }
  const float x = (size.x >= 1.0f) ? size.x : 1.0f;
  const float y = (size.y >= 1.0f) ? size.y : 1.0f;
  const float z = (size.z >= 1.0f) ? size.z : 1.0f;
  return (x * y * z);
}




// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
