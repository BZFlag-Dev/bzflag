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

#ifndef __DANGEROUSSPAWNPOLICY_H__
#define __DANGEROUSSPAWNPOLICY_H__

/* interface header */
#include "DefaultSpawnPolicy.h"


/** a DangerousSpawnPolicy is a SpawnPolicy that just generates a mostly
 *  dangerous (yet hopefully still valid) position and drops you there.
 */
class DangerousSpawnPolicy : public DefaultSpawnPolicy
{
public:
  DangerousSpawnPolicy();
  virtual ~DangerousSpawnPolicy();

  virtual const char *Name() const {
    static const char *name = "Dangerous";
    return name;
  }

  virtual void getPosition(float pos[3], int playerId, bool onGroundOnly, bool notNearEdges);
  virtual void getAzimuth(float &azimuth);

private:
  /* internal use */
  TeamColor   team;
  float       testPos[3];
};

#endif  /*__DANGEROUSSPAWNPOLICY_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
