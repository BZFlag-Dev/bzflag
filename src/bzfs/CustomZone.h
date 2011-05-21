/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
class MeshFace;

typedef std::vector<std::string> QualifierList;
typedef std::map<FlagType*, int> ZoneFlagMap; // type, count


class CustomZone : public WorldFileLocation {
  public:
    CustomZone(const MeshFace* face = NULL);

    bool readLine(const std::string& cmd, const std::string& line);

    virtual bool read(const char* cmd, std::istream&);
    virtual void writeToWorld(WorldInfo*) const;
    virtual bool usesGroupDef() { return false; }

    // make a safety zone for all team flags (on the ground)
    void addFlagSafety(float x, float y, WorldInfo* worldInfo);

    const QualifierList& getQualifiers() const;
    const ZoneFlagMap& getZoneFlagMap() const;

    float getWeight() const;
    void getRandomPoint(fvec3& pt) const;
    float getDistToPoint(const fvec3& pos) const;

    inline bool faceZone() const { return (face != NULL); }

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

    bool useCenter;

    const MeshFace* face;
    float faceHeight; // equivalent to the Z size component
    float faceWeight; // direct value, area is not considered
};


inline const QualifierList& CustomZone::getQualifiers() const {
  return qualifiers;
}


inline const ZoneFlagMap& CustomZone::getZoneFlagMap() const {
  return zoneFlagMap;
}


#endif  /* __CUSTOMZONE_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
