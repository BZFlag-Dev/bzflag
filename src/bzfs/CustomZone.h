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
#ifndef __CUSTOMZONE_H__
#define __CUSTOMZONE_H__

/* interface header */
#include "WorldFileLocation.h"

/* system headers */
#include <vector>
#include <string>
#include <map>

/* local implementation headers */
//#include "WorldInfo.h"
class WorldInfo;
class FlagType;


typedef std::vector<std::string> QualifierList;
typedef std::map<FlagType*, int> ZoneFlagMap; // type, count


class CustomZone : public WorldFileLocation
{
  public:
    CustomZone();

    virtual bool read(const char *cmd, std::istream&);
    virtual void writeToWorld(WorldInfo*) const;
    virtual bool usesGroupDef() { return false; }

    // make a safety zone for all team flags (on the ground)
    void addFlagSafety(float x, float y, WorldInfo* worldInfo);

    const QualifierList &getQualifiers() const;
    const ZoneFlagMap& getZoneFlagMap() const;

    float getArea() const;
    void getRandomPoint(fvec3& pt) const;
    float getDistToPoint(const fvec3& pos) const;

  public:
    static const std::string& getFlagIdQualifier(int flagId);
    static int getFlagIdFromQualifier(const std::string&);

    static const std::string& getFlagTypeQualifier(FlagType* flagType);
    static FlagType* getFlagTypeFromQualifier(const std::string&);

    static const std::string& getFlagSafetyQualifier(int team);
    static int getFlagSafetyFromQualifier(const std::string&);

    static const std::string& getPlayerTeamQualifier(int team);
    static int getPlayerTeamFromQualifier(const std::string&);

  private:
    void addZoneFlagCount(FlagType* flagType, int count);

  private:
    ZoneFlagMap zoneFlagMap;
    QualifierList qualifiers;
};


inline const QualifierList& CustomZone::getQualifiers() const
{
  return qualifiers;
}


inline const ZoneFlagMap& CustomZone::getZoneFlagMap() const
{
  return zoneFlagMap;
}


inline float CustomZone::getArea() const
{
  const float x = (size.x >= 1.0f) ? size.x : 1.0f;
  const float y = (size.y >= 1.0f) ? size.y : 1.0f;
  const float z = (size.z >= 1.0f) ? size.z : 1.0f;
  return (x * y * z);
}


#endif  /* __CUSTOMZONE_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
