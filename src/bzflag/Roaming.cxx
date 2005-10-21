/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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
#include "Roaming.h"

/* common headers */
#include "BZDBCache.h"
#include "AnsiCodes.h"

/* local headers */
#include "ScoreboardRenderer.h"

// initialize the singleton
template <>
Roaming* Singleton<Roaming>::_instance = (Roaming*)0;

Roaming::Roaming() : view(roamViewDisabled),
		     targetManual(-1),
		     targetWinner(-1),
		     targetFlag(-1) {
  resetCamera();
}

void Roaming::resetCamera(void) {
  camera.pos[0] = 0.0f;
  camera.pos[1] = 0.0f;
  camera.pos[2] = BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT); 
  camera.theta = 0.0f;
  camera.zoom = 60.0f;
  camera.phi = -0.0f;
}

void Roaming::setCamera(RoamingCamera* newCam) {
  memcpy(&camera, newCam, sizeof(RoamingCamera));
}

void Roaming::setMode(RoamingView newView) {
  if (!(LocalPlayer::getMyTank() || devDriving)) {
    view = newView;
  } else if (LocalPlayer::getMyTank()->getTeam() == ObserverTeam) {
    // disallow disabling roaming in observer mode
    if ((newView == roamViewDisabled) && !devDriving)
      view = (RoamingView)(roamViewDisabled + 1);
    else
      view = newView;
  } else {
    // don't allow roaming for non-observers
    if (newView != roamViewDisabled)
      view = roamViewDisabled;
    else
      view = newView;
  }
  // make sure we have a valid target
  changeTarget(next);
  changeTarget(previous);
}

void Roaming::changeTarget(Roaming::RoamingTarget target, int explicitIndex) {
  bool found = false;

  World* world = World::getWorld();
  if (!world) {
    DEBUG4("Roaming::changeTarget() no world, switching to free roaming\n");
    view = roamViewFree;
    buildRoamingLabel();
    return;
  }

  if (view == roamViewFree || view == roamViewDisabled) {
    // do nothing
    found = true;
  } else if (view == roamViewFlag) {
    if (target == explicitSet) {
      targetFlag = explicitIndex;
      found = true;
    } else {
      int i, j;
      const int maxFlags = world->getMaxFlags();
      for (i = 1; i < maxFlags; ++i) {
	if (target == next) {
	  j = (targetFlag + i) % maxFlags;
	} else if (target == previous) {
	  j = (targetFlag - i + maxFlags) % maxFlags;
	}
	const Flag& flag = world->getFlag(j);
	if (flag.type->flagTeam != NoTeam) {
	  targetFlag = j;
	  found = true;
	  break;
	}
      }
    }
  } else {
    if (target == explicitSet) {
      targetManual = targetWinner = explicitIndex;
      found = true;
    } else {
      int i, j;
      for (i = 0; i < world->getCurMaxPlayers(); ++i) {
	if (target == next) {
	  j = (targetManual + i + 2) % (world->getCurMaxPlayers() + 1) - 1;
	} else if (target == previous) {
	  j = (targetManual - i + world->getCurMaxPlayers() + 1) % (world->getCurMaxPlayers() + 1) - 1;
	}
	if ((j == -1)
	    || (world->getPlayer(j)
		&& (world->getPlayer(j)->getTeam() != ObserverTeam)
		&& world->getPlayer(j)->isAlive())) {
	  targetManual = targetWinner = j;
	  found = true;
	  break;
	}
      }
    }
  }

  if (!found)
    view = roamViewFree;

  buildRoamingLabel();
}

void Roaming::buildRoamingLabel(void) {
  std::string playerString = "";

  World* world = World::getWorld();

  // follow the important tank
  if (targetManual == -1) {
    Player* top = NULL;
    if (world && world->allowRabbit()) {
      // follow the white rabbit
      top = world->getCurrentRabbit();
      if (top != NULL) {
	playerString = "Rabbit ";
	targetWinner = top->getId();
      }
    }
    if (top == NULL) {
      // find the leader
      top = ScoreboardRenderer::getLeader(&playerString);
      if (top == NULL) {
	targetWinner = 0;
      } else {
	targetWinner = top->getId();
      }
    }
  }

  Player* tracked = NULL;
  if (!devDriving) {
    if (world) {
      tracked = world->getPlayer(targetWinner);
    }
  } else {
    tracked = LocalPlayer::getMyTank();
  }

  if (world && tracked) {
    if (BZDBCache::colorful) {
      int color = tracked->getTeam();
      if (world->allowRabbit() && (color == RogueTeam)) {
	// hunters are orange (hack)
	color = OrangeColor;
      } else if (color < 0 || color > 4) {
	// non-teamed, rabbit are white (same as observer)
	color = WhiteColor;
      }
      playerString += ColorStrings[color];
    }
    playerString += tracked->getCallSign();

    const FlagType* flag = tracked->getFlag();
    if (flag != Flags::Null) {
      if (BZDBCache::colorful) {
	playerString += ColorStrings[CyanColor] + " / ";
	if (flag->flagTeam != NoTeam) {
	  playerString += ColorStrings[flag->flagTeam];
	} else {
	  playerString += ColorStrings[WhiteColor];
	}
      } else {
	playerString += " / ";
      }
      if (flag->endurance == FlagNormal) {
	playerString += flag->flagName;
      } else {
	playerString += flag->flagAbbv;
      }
    }

    switch (view) {
      case roamViewTrack: {
	roamingLabel = "Tracking " + playerString;
	break;
      }
      case roamViewFollow: {
	roamingLabel = "Following " + playerString;
	break;
      }
      case roamViewFP: {
	roamingLabel = "Driving with " + playerString;
	break;
      }
      case roamViewFlag: {
	roamingLabel = std::string("Tracking ")
	  + world->getFlag(targetFlag).type->flagName + " Flag";
	break;
      }
      default: {
	roamingLabel = "Roaming";
	break;
      }
    }
  } else {
    roamingLabel = "Roaming";
  }
}

void Roaming::updatePosition(RoamingCamera* dc, float dt) {
  World* world = World::getWorld();

  // are we tracking?
  bool tracking = false;
  const float* trackPos;
  if (view == roamViewTrack) {
    Player *target;
    if (!devDriving) {
      if (world && (targetWinner < world->getCurMaxPlayers())) {
	target = world->getPlayer(targetWinner);
      } else {
	target = NULL;
      }
    } else {
      target = LocalPlayer::getMyTank();
    }
    if (target != NULL) {
      trackPos = target->getPosition();
      tracking = true;
    }
  }
  else if (view == roamViewFlag) {
    if ((world != NULL) && (targetFlag < world->getMaxFlags())) {
      Flag &flag = world->getFlag(targetFlag);
      trackPos = flag.position;
      tracking = true;
    }
  }

  // modify X and Y coords
  if (!tracking) {
    const float c = cosf(camera.theta * (float)(M_PI / 180.0f));
    const float s = sinf(camera.theta * (float)(M_PI / 180.0f));
    camera.pos[0] += dt * (c * dc->pos[0] - s * dc->pos[1]);
    camera.pos[1] += dt * (c * dc->pos[1] + s * dc->pos[0]);
    camera.theta  += dt * dc->theta;
    camera.phi    += dt * dc->phi;
  } else {
    float dx = camera.pos[0] - trackPos[0];
    float dy = camera.pos[1] - trackPos[1];
    float dist = sqrtf((dx * dx) + (dy * dy));

    float nomDist = 4.0f * BZDBCache::tankSpeed;
    if (nomDist == 0.0f) {
      nomDist = 100.0f;
    }
    float distFactor =  (dist / nomDist);
    if (distFactor < 0.25f) {
      distFactor = 0.25f;
    }
    float newDist = dist - (dt * distFactor * dc->pos[0]);

    const float minDist = BZDBCache::tankLength * 0.5f;
    if (newDist < minDist) {
      if (dist >= minDist) {
	newDist = minDist;
      } else {
	newDist = dist;
      }
    }
    float scale = 0.0f;
    if (dist > 0.0f) {
      scale = newDist / dist;
    }
    dx = dx * scale;
    dy = dy * scale;
    if (fabsf(dx) < 0.001f) dx = 0.001f;
    if (fabsf(dy) < 0.001f) dy = 0.001f;
    const float dtheta = -(dt * dc->theta * (float)(M_PI / 180.0f));
    const float c = cosf(dtheta);
    const float s = sinf(dtheta);
    camera.pos[0] = trackPos[0] + ((c * dx) - (s * dy));
    camera.pos[1] = trackPos[1] + ((c * dy) + (s * dx));
    // setup so that free roam stays in the last state
    camera.theta = atan2f(trackPos[1] - camera.pos[1], trackPos[0] - camera.pos[0]);
    camera.theta *= (float)(180.0f / M_PI);
    camera.phi = atan2f(trackPos[2] - camera.pos[2], newDist);
    camera.phi *= (float)(180.0f / M_PI);
  }

  // modify Z coordinate
  camera.pos[2] += dt * dc->pos[2];
  float muzzleHeight = BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT);
  if (camera.pos[2] < muzzleHeight) {
    camera.pos[2] = muzzleHeight;
    dc->pos[2] = 0.0f;
  }

  // adjust zoom
  camera.zoom += dt * dc->zoom;
  if (camera.zoom < BZDB.eval("roamZoomMin")) {
    camera.zoom = BZDB.eval("roamZoomMin");
  }
  else if (camera.zoom > BZDB.eval("roamZoomMax")) {
    camera.zoom = BZDB.eval("roamZoomMax");
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
