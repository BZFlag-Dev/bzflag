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
  virtual void write(WorldInfo*) const;

  const QualifierList &getQualifiers() const;
  float getArea() const;
  void getRandomPoint(float *pt) const;
protected:
  QualifierList qualifiers;
};

inline const QualifierList& CustomZone::getQualifiers() const
{
  return qualifiers;
}

inline float CustomZone::getArea() const
{
  float h;
  if (size[2] < 1.0f)
    h = 1.0f;
  else
    h = size[2];
  return size[0] * size[1] * h;
}

#endif  /* __CUSTOMZONE_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
