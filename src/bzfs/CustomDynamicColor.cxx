/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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

  if (strcasecmp ("variable", cmd) == 0) {
    std::string varName;
    if (input >> varName) {
      color->setVariableName(varName);
      return true;
    } else {
      std::cout << "missing variable name" << std::endl;
      return false;
    }
  }
  else if (strcasecmp ("varTiming", cmd) == 0) {
    float varTiming;
    if (input >> varTiming) {
      color->setVariableTiming(varTiming);
      return true;
    } else {
      std::cout << "missing variable timing" << std::endl;
      return false;
    }
  }
  else if (strcasecmp ("varUseAlpha", cmd) == 0) {
    color->setVariableUseAlpha(true);
    return true;
  }
  
  
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
  input.putback('\n');

  if (!(parms >> command)) {
    std::cout << "missing parameter type for "
	      << cmd << " channel" << std::endl;
    return false;
  }

  if (strcasecmp (command.c_str(), "limits") == 0) {
    float min, max;
    if (!(parms >> min >> max)) {
      std::cout << "missing limits for " << cmd << " channel" << std::endl;
      return false;
    }
    color->setLimits(channel, min, max);
  }
  else if (strcasecmp (command.c_str(), "sinusoid") == 0) {
    float data[3];
    if (!(parms >> data[0] >> data[1] >> data[2])) {
      std::cout << "missing sinusoid parameters for " << cmd << " channel"
		<< std::endl;
      return false;
    }
    color->addSinusoid(channel, data);
  }
  else if (strcasecmp (command.c_str(), "clampup") == 0) {
    float data[3];
    if (!(parms >> data[0] >> data[1] >> data[2])) {
      std::cout << "missing clampup parameters for " << cmd << " channel"
		<< std::endl;
      return false;
    }
    color->addClampUp(channel, data);
  }
  else if (strcasecmp (command.c_str(), "clampdown") == 0) {
    float data[3];
    if (!(parms >> data[0] >> data[1] >> data[2])) {
      std::cout << "missing clampdown parameters for " << cmd << " channel"
		<< std::endl;
      return false;
    }
    color->addClampDown(channel, data);
  }
  else if (strcasecmp (command.c_str(), "sequence") == 0) {
    float period, offset;
    std::vector<char> list;
    if (!(parms >> period >> offset)) {
      std::cout << "missing sequence period for " << cmd << " channel"
		<< std::endl;
      return false;
    }
    int tmp;
    while (parms >> tmp) {
      list.push_back((char)tmp);
    }
    color->setSequence(channel, period, offset, list);
  }
  else {
    return false;
  }

  return true;
}


void CustomDynamicColor::writeToManager() const
{
  color->setName(name);
  if ((name.size() > 0) && (DYNCOLORMGR.findColor(name) >= 0)) {
    std::cout << "warning: duplicate dynamic color"
	      << " (" << name << ")" << std::endl;
  }
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
