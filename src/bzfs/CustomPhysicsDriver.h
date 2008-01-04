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

#ifndef __CUSTOM_PHYSICS_DRIVER_H__
#define __CUSTOM_PHYSICS_DRIVER_H__

/* interface header */
#include "WorldFileObject.h"

/* common headers */
#include "PhysicsDriver.h"

class CustomPhysicsDriver : public WorldFileObject {
  public:
    CustomPhysicsDriver();
    ~CustomPhysicsDriver();
    virtual bool read(const char *cmd, std::istream& input);
    virtual void writeToManager() const;
    bool usesManager() { return true; }

  private:
    mutable PhysicsDriver* driver;
};

#endif  /* __CUSTOM_PHYSICS_DRIVER_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
