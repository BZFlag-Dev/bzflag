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

#ifndef __CUSTOMWEAPON_H__
#define __CUSTOMWEAPON_H__

#include "common.h"

/* system interface headers */
#include <iostream>
#include <vector>

/* common interface headers */
#include "Flag.h"
#include "TimeKeeper.h"

/* local interface headers */
#include "WorldFileLocation.h"
#include "WorldInfo.h"


class CustomWeapon : public WorldFileLocation {
  public:
    CustomWeapon();
  virtual bool read(const char *cmd, std::istream&);
    virtual void write(WorldInfo*) const;

  protected:
    float initdelay;
    std::vector<float> delay;
    FlagType *type;
    static TimeKeeper sync;
};

#endif  /* __CUSTOMWEAPON_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
