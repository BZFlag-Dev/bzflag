/* bzflag
 * Copyright (c) 1993-2013 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// Interface header
#include "ShotStatistics.h"
#include "TimeKeeper.h"

ShotStatistics::ShotStatistics() :
      totalFired(0), totalHit(0)
{
	lastShotTimeDelta = 0;
	lastShotTime = 0;
	lastShotDeviation = 0;
}

ShotStatistics::~ShotStatistics()
{
}

int ShotStatistics::getTotalPerc() const
{
  if (getTotalFired() == 0)
    return 100;
  return (int)(100 * ((float)getTotalHit() / (float)getTotalFired()));
}

void ShotStatistics::recordFire(FlagType* flag, const float *pVec, const float *shotVec )
{
  fired[flag]++;
  totalFired++;

  double currentTime = TimeKeeper::getCurrent().getSeconds();
  if (lastShotTime > 0)
  {
	lastShotTimeDelta = currentTime-lastShotTime;
  }
  lastShotTime = currentTime;

  float playerNorm[3];
  float shotNorm[3];
  float playerMag,shotMag;

  playerMag = sqrt((pVec[0]*pVec[0])+(pVec[1]*pVec[1])+pVec[2]*pVec[2]);
  shotMag = sqrt((shotVec[0]*shotVec[0])+(shotVec[1]*shotVec[1])+shotVec[2]*shotVec[2]);

  playerNorm[0] = pVec[0]/playerMag; playerNorm[1] = pVec[1]/playerMag; playerNorm[2] = pVec[2]/playerMag;
  shotNorm[0] = shotVec[0]/shotMag; shotNorm[1] = shotVec[1]/shotMag; shotNorm[2] = shotVec[2]/shotMag;

  float dot = (shotNorm[0] * playerNorm[0]) + (shotNorm[1] * playerNorm[1]) + shotNorm[2] * playerNorm[2];

  double cos = acos(dot);
  double radToDeg = 180.0/3.1415;

  lastShotDeviation = (float)(cos*radToDeg);
}

void ShotStatistics::recordHit(FlagType* flag)
{
  hit[flag]++;
  totalHit++;
}

typedef std::pair<FlagType*, float> FlagStat;

FlagType* ShotStatistics::getFavoriteFlag() const
{
  /* return the flag the player fired most */
  FlagType* greatest = Flags::Null;

  // we don't deal with the case where there are two "equally favorite"
  // flags; doesn't really matter
  for (auto it = fired.begin(); it != fired.end(); it++) {
	// ignore none/null - looking for favorite *flags*
	if (it->second > getFired(greatest) && it->first != Flags::Null) {
	  greatest = it->first;
	}
  }

  return greatest;
}

FlagType* ShotStatistics::getBestFlag() const
{
  /* return the flag with the best hits/fired ratio */
  FlagType* greatest = Flags::Null;
  float greatestRatio = getHit(greatest)/float(getFired(greatest));

  // we don't deal with the case where there are two "equally best"
  // flags; doesn't really matter
  for (auto it = fired.begin(); it != fired.end(); it++) {
	float ratio = getHit(it->first)/float(it->second);

	// normal shots have the opportunity to be best
	if (ratio > greatestRatio) {
	  greatest = it->first;
	  greatestRatio = getHit(greatest)/float(getFired(greatest));
	}
  }

  return greatest;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
