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

// object that creates and contains a spawn position

#include "PlayerInfo.h"

class SpawnPosition {

public:
  SpawnPosition::SpawnPosition(int playerId, bool onGroundOnly,
			       bool notNearEdges);
  SpawnPosition::~SpawnPosition();

  float       pos[3];
  float	      azimuth;

private:
  const float enemyProximityCheck(float &enemyAngle) const;
  const float distanceFrom(const float* farPos) const;
  const bool  isImminentlyDangerous() const;
  const bool  isFacing(const float *enemyPos, const float enemyAzimuth, 
		       const float deviation) const;
  
  TeamColor   team;
  float       testPos[3];
  int	      curMaxPlayers;
  
  float	      safeSWRadius;
  float	      safeDistance;

};
