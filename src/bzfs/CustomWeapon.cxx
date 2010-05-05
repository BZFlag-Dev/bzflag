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
#include "CustomWeapon.h"


// system headers
#include <iostream>
#include <sstream>
#include <string>
#include <string.h>
#include <math.h>

// common headers
#include "MeshObstacle.h"

// local headers
#include "WorldWeapons.h"
#include "TextUtils.h"

BzTime CustomWeapon::sync = BzTime::getCurrent();

const float CustomWeapon::minWeaponDelay = 0.1f;

CustomWeapon::CustomWeapon(const MeshObstacle* _mesh)
{
  pos = fvec3(0.0f, 0.0f, 0.0f);
  size = fvec3(1.0f, 1.0f, 1.0f);
  rotation = 0.0f;
  tilt = 0.0f;
  initdelay = 10.0f;
  delay.push_back(10.0f);
  type = Flags::Null;
  teamColor = RogueTeam;

  triggerType = bz_eNullEvent;
  eventTeam = -1;

  mesh = _mesh;
  posVertex = -1;
  dirNormal = -1;
}


bool CustomWeapon::read(const char *cmd, std::istream& input)
{
  const std::string lower = TextUtils::tolower(cmd);

  if ((lower == "type")      ||
      (lower == "color")     ||
      (lower == "tilt")      ||
      (lower == "initdelay") ||
      (lower == "delay")     ||
      (lower == "trigger")   ||
      (lower == "eventteam") ||
      (lower == "posvertex") ||
      (lower == "dirnormal")) {
    std::string line;
    std::getline(input, line);
    input.putback('\n');
    return readLine(cmd, line);
  }  
  else if (!mesh) {
    return WorldFileLocation::read(cmd, input);
  }

  return false;
}


bool CustomWeapon::readLine(const std::string& cmd, const std::string& line)
{
  const std::string lower = TextUtils::tolower(cmd);

  std::istringstream parms(line);

  if (lower == "initdelay") {
    parms >> initdelay;
  }
  else if (lower == "delay") {
    std::string args;
    float d;

    delay.clear();

    while (parms >> d) {
      if (d < minWeaponDelay) {
	std::cout << "skipping weapon delay of " << d << " seconds" << std::endl;
	continue;
      } else {
	delay.push_back(d);
      }
    }
    if (delay.empty()) {
      return false;
    }
  }
  else if (lower == "type") {
    std::string abbv;
    parms >> abbv;
    type = Flag::getDescFromAbbreviation(abbv.c_str());
    if (type == NULL)
      return false;
  }
  else if (lower == "color") {
    int team;
    if (!(parms >> team)) {
      std::cout << "weapon color requires a team number" << std::endl;
    } else {
      teamColor = (TeamColor)team;
    }
  }
  else if (lower == "tilt") {
    if (!(parms >> tilt)) {
      std::cout << "weapon tilt requires a value" << std::endl;
    }
    // convert to radians
    tilt *= DEG2RADf;
  }
  else if (lower == "trigger") {
    std::string triggerName;
    parms >> triggerName;

    triggerType = bz_eNullEvent;

    TextUtils::tolower(triggerName);
    if (triggerName == "oncap") {
      triggerType = bz_eCaptureEvent;
    } else if (triggerName == "onspawn") {
      triggerType = bz_ePlayerSpawnEvent;
    } else if (triggerName == "ondie") {
      triggerType = bz_ePlayerDieEvent;
    } else if (triggerName == "none") {
      triggerType = bz_eNullEvent;
    } else {
      std::cout << "weapon trigger type:" << triggerName << " unknown" << std::endl;
      return true;
    }
    logDebugMessage(4,"Adding world weapon triggered '%s'\n", triggerName.c_str());
  }
  else if (lower == "eventteam") {
    parms >> eventTeam;
  }
  else if (lower == "posvertex") {
    parms >> posVertex;
  }
  else if (lower == "dirnormal") {
    parms >> dirNormal;
  }
  else {
    return false;
  }

  return true;
}

void CustomWeapon::writeToWorld(WorldInfo* world) const
{
  fvec3 p = pos;
  float r = rotation;
  float t = tilt;

  if (mesh) {
    if ((posVertex >= 0) && (posVertex < mesh->getVertexCount())) {
      p = mesh->getVertices()[posVertex];
    }
    if ((dirNormal >= 0) && (dirNormal < mesh->getNormalCount())) {
      const fvec3& dir = mesh->getNormals()[dirNormal];
      r = atan2f(dir.y, dir.x);
      t = atan2f(dir.z, dir.xy().length());
    }
  }

  if (triggerType == bz_eNullEvent) {
    const bool fromMesh = (mesh != NULL);
    world->addWeapon(type, p, r, t, teamColor, initdelay, delay, sync, fromMesh);
  }
  else {
    WorldWeaponGlobalEventHandler* eventHandler =
      new WorldWeaponGlobalEventHandler(type, p, r, t, (TeamColor)eventTeam);
    worldEventManager.addEvent(triggerType, eventHandler);
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
