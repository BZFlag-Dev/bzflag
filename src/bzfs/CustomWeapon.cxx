/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "CustomWeapon.h"


/* system headers */
#include <sstream>
#include <string>
#include <math.h>

/* local implementation headers */
#include "WorldWeapons.h"
#include "TextUtils.h"
#include "WorldEventManager.h"

TimeKeeper CustomWeapon::sync = TimeKeeper::getCurrent();

const float CustomWeapon::minWeaponDelay = 0.1f;

CustomWeapon::CustomWeapon()
{
  pos[0] = pos[1] = pos[2] = 0.0f;
  rotation = 0.0f;
  size[0] = size[1] = size[2] = 1.0f;
  tilt = 0.0f;
  initdelay = 10.0f;
  delay.push_back(10.0f);
  type = Flags::Null;

	eventTrigerd = false;
	teamCapTriger = false;
	capTeam = -1;
}


bool CustomWeapon::read(const char *cmd, std::istream& input) {
  if (strcmp(cmd, "initdelay") == 0) {
    input >> initdelay;
  }
  else if (strcmp(cmd, "delay") == 0) {
    std::string args;
    float d;

    delay.clear();
    std::getline(input, args);
    std::istringstream  parms(args);

    while (parms >> d) {
      if (d < minWeaponDelay) {
	std::cout << "skipping weapon delay of " << d << " seconds" << std::endl;
	continue;
      }
      else {
	delay.push_back(d);
      }
    }
    input.putback('\n');
    if (delay.size() == 0)
      return false;
  }
  else if (strcmp(cmd, "type") == 0) {
    std::string abbv;
    input >> abbv;
    type = Flag::getDescFromAbbreviation(abbv.c_str());
    if (type == NULL)
      return false;
  }
  else if (strcmp(cmd, "tilt") == 0) {
    if (!(input >> tilt)) {
      std::cout << "weapon tilt requires a value" << std::endl;
    }
    // convert to radians
    tilt = (float)(tilt * (M_PI / 180.0));
  }
  else if (!WorldFileLocation::read(cmd, input)) {
    return false;
  }
	else if (strcmp(cmd, "triger") == 0) 
	{
		std::string trigerType;
		input >> trigerType;

		eventTrigerd = true;

		TextUtils::tolower(trigerType);
		if ( trigerType == "flagcap")
			teamCapTriger = true;
		else
		{
			eventTrigerd = false;
			std::cout << "weapon triger type:" << trigerType << " unknown" << std::endl;
		}
	}
	else if (strcmp(cmd, "capteam") == 0) 
	{
		input >> capTeam;
	}

  return true;
}


void CustomWeapon::writeToWorld(WorldInfo* world) const
{
	if (!eventTrigerd)
		world->addWeapon(type, pos, rotation, tilt, initdelay, delay, sync);
	else if ( teamCapTriger )
		worldEventManager.addCapEvent(capTeam,new WorldWeaponGlobalEventHandaler(type, pos, rotation, tilt));
}


// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
