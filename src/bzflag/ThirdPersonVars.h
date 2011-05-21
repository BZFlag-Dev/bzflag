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

#ifndef BZF_THIRDPERSONVARS_H
#define BZF_THIRDPERSONVARS_H

/* system interface headers */
#include <string>

struct ThirdPersonVars {
  ThirdPersonVars();
  ~ThirdPersonVars();

  void load();
  void clear();

  static void bzdbCallback(const std::string& name, void* data);

  bool b3rdPerson;
  float cameraOffsetXY;
  float cameraOffsetZ;
  float targetMultiplier;
  float nearTargetDistance;
  float nearTargetSize;
  float farTargetDistance;
  float farTargetSize;
};


#endif // BZF_THIRDPERSONVARS_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
