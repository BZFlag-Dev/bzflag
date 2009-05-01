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

#include "common.h"

/* interface header */
#include "CustomDynamicColor.h"

/* system implementation headers */
#include <iostream>
#include <sstream>
#include <vector>
#include <string.h>

/* common implementation headers */
#include "DynamicColor.h"
#include "ParseColor.h"


CustomDynamicColor::CustomDynamicColor()
{
  dyncol = new DynamicColor;
  return;
}


CustomDynamicColor::~CustomDynamicColor()
{
  delete dyncol;
  return;
}


bool CustomDynamicColor::read(const char *cmd, std::istream& input)
{
  if (strcasecmp ("varName", cmd) == 0) {
    std::string varName;
    if (!(input >> varName)) {
      std::cout << "missing variable name" << std::endl;
      return false;
    }
    dyncol->setVariableName(varName);
  }
  else if (strcasecmp ("varTime", cmd) == 0) {
    float varTime;
    if (!(input >> varTime)) {
      std::cout << "missing variable timing" << std::endl;
      return false;
    }
    dyncol->setVariableTiming(varTime);
  }
  else if (strcasecmp ("varNoAlpha", cmd) == 0) {
    dyncol->setVariableNoAlpha(true);
  }
  else if (strcasecmp("delay", cmd) == 0) {
    float delay;
    if (!(input >> delay)) {
      std::cout << "bad dyncol delay" << std::endl;
    }
    dyncol->setDelay(delay);
  }
  else if (strcasecmp("ramp", cmd) == 0) {
    float duration;
    if (!(input >> duration)) {
      std::cout << "bad dyncol ramp duration" << std::endl;
      return false;
    }
    fvec4 color;
    if (!parseColorStream(input, color)) {
      std::cout << "bad dyncol ramp color" << std::endl;
      return false;
    }
    dyncol->addState(duration, color);
  }
  else if (strcasecmp("level", cmd) == 0) {
    float duration;
    if (!(input >> duration)) {
      std::cout << "bad dyncol level duration" << std::endl;
      return false;
    }
    fvec4 color;
    if (!parseColorStream(input, color)) {
      std::cout << "bad dyncol level color" << std::endl;
      return false;
    }
    dyncol->addState(duration, color);
    dyncol->addState(0.0f,     color);
  }
  else {
    // NOTE: we don't use a WorldFileObstacle
    return WorldFileObject::read(cmd, input);
  }

  return true;
}


void CustomDynamicColor::writeToManager() const
{
  dyncol->setName(name);
  if ((name.size() > 0) && (DYNCOLORMGR.findColor(name) >= 0)) {
    std::cout << "WARNING: duplicate dynamic color"
	      << " (" << name << ")" << std::endl;
  }
  dyncol->finalize();
  DYNCOLORMGR.addColor(dyncol);
  dyncol = NULL;
  return;
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
