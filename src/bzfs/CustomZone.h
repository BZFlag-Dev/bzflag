/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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

/* local implementation headers */
#include "WorldInfo.h"
#include "EntryZones.h"

class CustomZone : public WorldFileLocation {
  public:
    CustomZone();

    virtual bool read(const char *cmd, std::istream&);
    virtual void writeToWorld(WorldInfo*) const;
    virtual bool usesGroupDef() { return false; }

    // make a safety zone for all team flags (on the ground)
    void addFlagSafety(float x, float y, WorldInfo* worldInfo);

    const QualifierList &getQualifiers() const;
    float getArea() const;
    void getRandomPoint(float *pt) const;
    float getDistToPoint (const float *pos) const;

  protected:
    QualifierList qualifiers;
};

inline const QualifierList& CustomZone::getQualifiers() const
{
  return qualifiers;
}

inline float CustomZone::getArea() const
{
  float x = 1.0f, y = 1.0f, z = 1.0f;

  if (size[0] > 1.0f) {
    x = size[0];
  }
  if (size[1] > 1.0f) {
    y = size[1];
  }
  if (size[2] > 1.0f) {
    z = size[2];
  }

  return (x * y * z);
}

#endif  /* __CUSTOMZONE_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
