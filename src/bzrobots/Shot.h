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

#ifndef __SHOT_H__
#define __SHOT_H__

#include "common.h"
#include "Address.h"

class Shot {
public:
  Shot();
  Shot(uint64_t _id);
  Shot(PlayerId _plr, uint16_t _sid);
  virtual ~Shot();

  PlayerId getPlayerId(void) const;
  uint16_t getShotId(void) const;

  uint64_t getId(void) const;
  void setId(uint64_t id);

  mutable double x, y, z;
  mutable double vx, vy, vz;

protected:
  uint64_t id;
};

#else

class Shot;

#endif /* __SHOT_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
