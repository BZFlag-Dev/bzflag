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

/* interface header */
#include "Shot.h"

Shot::Shot() {}

Shot::Shot(uint64_t _id) : id(_id)
{
}

Shot::Shot(PlayerId _plr, uint16_t _id)
{
  id = ((uint64_t)_plr << 16) + _id;
}

Shot::~Shot() {}

PlayerId Shot::getPlayerId(void) const
{
  return (PlayerId)(id >> 16);
}

uint16_t Shot::getShotId(void) const
{
  return id & 0xffff;
}

uint64_t Shot::getId(void) const
{
  return id;
}

void Shot::setId(uint64_t _id)
{
  id = _id;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
