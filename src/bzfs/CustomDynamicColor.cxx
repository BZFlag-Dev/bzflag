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

#include "common.h"

/* interface header */
#include "CustomDynamicColor.h"

/* system implementation headers */
#include <sstream>

/* common implementation headers */
#include "DynamicColor.h"


CustomDynamicColor::CustomDynamicColor()
{
  color = new DynamicColor;
  return;
}


CustomDynamicColor::~CustomDynamicColor()
{
  delete color;
  return;
}


bool CustomDynamicColor::read(const char *cmd, std::istream& input)
{
  int channel = -1;
  
  if (strcasecmp ("red", cmd) == 0) {
    channel = 0;
  }
  else if (strcasecmp ("green", cmd) == 0) {
    channel = 1;
  }
  else if (strcasecmp ("blue", cmd) == 0) {
    channel = 2;
  }
  else if (strcasecmp ("alpha", cmd) == 0) {
    channel = 3;
  }
  else {
    // NOTE: we don't use a WorldFileObstacle
    return WorldFileObject::read(cmd, input);
  }

  // in case WorldFileObject() at a later date  
  if (channel < 0) {
    std::cout << "unknown color channel" << std::endl;
    return false;
  }
  
  std::string args;
  std::string command;
  std::getline(input, args);
  std::istringstream parms(args);
  
  if (!(parms >> command)) {
    std::cout << "missing parameter type for " << cmd << " channel" << std::endl;
    return false;
  }
  
  if (command == "limits") {
    float min, max;
    if (!(parms >> min) || !(parms >> max)) {
      std::cout << "missing limits for " << cmd << " channel" << std::endl;
      return false;
    }
    color->setLimits(channel, min, max);
  }
  else if (command == "sinusoid") {
    float period, offset;
    if (!(parms >> period) || !(parms >> offset)) {
      std::cout << "missing sinusoid parameters for " << cmd << " channel" 
                << std::endl;
      return false;
    }
    color->setSinusoid(channel, period, offset);
  }
  else if (command == "clampup") {
    float period, offset, width;
    if (!(parms >> period) || !(parms >> offset) || !(parms >> width)) {
      std::cout << "missing clampup parameters for " << cmd << " channel"
                << std::endl;
      return false;
    }
    color->setClampUp(channel, period, offset, width);
  }
  else if (command == "clampdown") {
    float period, offset, width;
    if (!(parms >> period) || !(parms >> offset) || !(parms >> width)) {
      std::cout << "missing clampdown parameters for " << cmd << " channel"
                << std::endl;
      return false;
    }
    color->setClampDown(channel, period, offset, width);
  }
  else {
    return false;
  }

  input.putback('\n');

  return true;
}


void CustomDynamicColor::write(WorldInfo */*world*/) const
{
  color->finalize();
  DYNCOLORMGR.addColor (color);
  color = NULL;
  return;
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
