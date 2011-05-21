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

// interface header
#include "ThirdPersonVars.h"

// common headers
#include "StateDatabase.h"


ThirdPersonVars::ThirdPersonVars()
  : b3rdPerson(false) {
  BZDB.addCallback("_forbid3rdPersonCam",         bzdbCallback, this);
  BZDB.addCallback("3rdPersonCam",                bzdbCallback, this);
  BZDB.addCallback("3rdPersonCamXYOffset",        bzdbCallback, this);
  BZDB.addCallback("3rdPersonCamZOffset",         bzdbCallback, this);
  BZDB.addCallback("3rdPersonCamTargetMult",      bzdbCallback, this);
  BZDB.addCallback("3rdPersonNearTargetDistance", bzdbCallback, this);
  BZDB.addCallback("3rdPersonNearTargetSize",     bzdbCallback, this);
  BZDB.addCallback("3rdPersonFarTargetDistance",  bzdbCallback, this);
  BZDB.addCallback("3rdPersonFarTargetSize",      bzdbCallback, this);
}


ThirdPersonVars::~ThirdPersonVars() {
  BZDB.removeCallback("_forbid3rdPersonCam",         bzdbCallback, this);
  BZDB.removeCallback("3rdPersonCam",                bzdbCallback, this);
  BZDB.removeCallback("3rdPersonCamXYOffset",        bzdbCallback, this);
  BZDB.removeCallback("3rdPersonCamZOffset",         bzdbCallback, this);
  BZDB.removeCallback("3rdPersonCamTargetMult",      bzdbCallback, this);
  BZDB.removeCallback("3rdPersonNearTargetDistance", bzdbCallback, this);
  BZDB.removeCallback("3rdPersonNearTargetSize",     bzdbCallback, this);
  BZDB.removeCallback("3rdPersonFarTargetDistance",  bzdbCallback, this);
  BZDB.removeCallback("3rdPersonFarTargetSize",      bzdbCallback, this);
}

void ThirdPersonVars::load(void) {
  b3rdPerson = !BZDB.isTrue(std::string("_forbid3rdPersonCam")) &&
               BZDB.isTrue(std::string("3rdPersonCam"));

  if (b3rdPerson) {
    cameraOffsetXY   = BZDB.eval(std::string("3rdPersonCamXYOffset"));
    cameraOffsetZ    = BZDB.eval(std::string("3rdPersonCamZOffset"));
    targetMultiplier = BZDB.eval(std::string("3rdPersonCamTargetMult"));

    nearTargetDistance = BZDB.eval(std::string("3rdPersonNearTargetDistance"));
    nearTargetSize     = BZDB.eval(std::string("3rdPersonNearTargetSize"));
    farTargetDistance  = BZDB.eval(std::string("3rdPersonFarTargetDistance"));
    farTargetSize      = BZDB.eval(std::string("3rdPersonFarTargetSize"));
  }
}

void ThirdPersonVars::clear(void) {
  b3rdPerson = false;
}

void ThirdPersonVars::bzdbCallback(const std::string& /*name*/, void* data) {
  ((ThirdPersonVars*)data)->load();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
