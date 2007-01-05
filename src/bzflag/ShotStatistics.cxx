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

// Interface header
#include "ShotStatistics.h"
#include "TimeKeeper.h"

ShotStatistics::ShotStatistics() :
      normalFired(0), normalHit(0),
      guidedMissileFired(0), guidedMissileHit(0),
      laserFired(0), laserHit(0),
      superBulletFired(0), superBulletHit(0),
      shockWaveFired(0), shockWaveHit(0),
      thiefFired(0), thiefHit(0)
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

void ShotStatistics::recordFire( FlagType* flag, const float *pVec, const float *shotVec )
{
  if (flag == Flags::GuidedMissile)
    guidedMissileFired++;
  else if (flag == Flags::Laser)
    laserFired++;
  else if (flag == Flags::SuperBullet)
    superBulletFired++;
  else if (flag == Flags::ShockWave)
    shockWaveFired++;
  else if (flag == Flags::Thief)
    thiefFired++;
  else
    normalFired++;

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
  if (flag == Flags::GuidedMissile)
    guidedMissileHit++;
  else if (flag == Flags::Laser)
    laserHit++;
  else if (flag == Flags::SuperBullet)
    superBulletHit++;
  else if (flag == Flags::ShockWave)
    shockWaveHit++;
  else if (flag == Flags::Thief)
    thiefHit++;
  else
    normalHit++;
}

typedef std::pair<FlagType*, float> FlagStat;

FlagType* ShotStatistics::getFavoriteFlag() const
{
  /* return the flag the player fired most */
  std::vector<FlagStat> flags;
  FlagStat greatest = std::make_pair(Flags::Null, 0.0f);

  // no entry for none/null - looking for favorite *flag*
  flags.push_back(std::make_pair(Flags::GuidedMissile, (float)guidedMissileFired));
  flags.push_back(std::make_pair(Flags::Laser, (float)laserFired));
  flags.push_back(std::make_pair(Flags::SuperBullet, (float)superBulletFired));
  flags.push_back(std::make_pair(Flags::ShockWave, (float)shockWaveFired));
  flags.push_back(std::make_pair(Flags::Thief, (float)thiefFired));

  // we don't deal with the case where there are two "equally favorite"
  // flags; doesn't really matter
  for (int i = 0; i < (int)flags.size(); i++) {
    if (flags[i].second > greatest.second)
      greatest = flags[i];
  }

  return greatest.first;
}

FlagType* ShotStatistics::getBestFlag() const
{
  /* return the flag with the best hits/fired ratio */
  std::vector<FlagStat> flags;
  FlagStat greatest = std::make_pair(Flags::Null, 0.0f);

  // normal shots have the opportunity to be best
  flags.push_back(std::make_pair(Flags::Null,
    ((float)normalHit / normalFired)));
  flags.push_back(std::make_pair(Flags::GuidedMissile,
    ((float)guidedMissileHit / guidedMissileFired)));
  flags.push_back(std::make_pair(Flags::Laser,
    ((float)laserHit / laserFired)));
  flags.push_back(std::make_pair(Flags::SuperBullet,
    ((float)superBulletHit / superBulletFired)));
  flags.push_back(std::make_pair(Flags::ShockWave,
    ((float)shockWaveHit / shockWaveFired)));
  flags.push_back(std::make_pair(Flags::Thief,
    ((float)thiefHit / thiefFired)));

  // we don't deal with the case where there are two "equally best"
  // flags; doesn't really matter
  for (int i = 0; i < (int)flags.size(); i++) {
    if (flags[i].second > greatest.second)
      greatest = flags[i];
  }

  return greatest.first;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
