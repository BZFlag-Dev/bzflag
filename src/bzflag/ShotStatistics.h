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

#ifndef	__SHOTSTATISTICS_H__
#define	__SHOTSTATISTICS_H__

#include "common.h"

#include "Flag.h"

/** ShotStatistics stores and calculates a set of statistics on player's shots 
 *  and accuracy
 */
class ShotStatistics {
public:

  ShotStatistics::ShotStatistics() : 
      normalFired(0), normalHit(0),
      guidedMissileFired(0), guidedMissileHit(0),
      laserFired(0), laserHit(0),
      superBulletFired(0), superBulletHit(0),
      shockWaveFired(0), shockWaveHit(0),
      thiefFired(0), thiefHit(0)
      {};
  ShotStatistics::~ShotStatistics() {};

  // raw stats returns
  int         getNormalFired() const;
  int         getNormalHit() const;
  int         getGMFired() const;
  int         getGMHit() const;
  int         getLFired() const;
  int         getLHit() const;
  int         getSBFired() const;
  int         getSBHit() const;
  int         getSWFired() const;
  int         getSWHit() const;
  int         getTHFired() const;
  int         getTHHit() const;
  
  // stats processing
  int         getTotalFired() const;
  int         getTotalHit() const;
  int         getTotalPerc() const;

  // tally functions
  void	      recordFire(FlagType* flag);
  void	      recordHit(FlagType* flag);

private:
  int	      normalFired;
  int	      normalHit;
  int	      guidedMissileFired;
  int	      guidedMissileHit;
  int	      laserFired;
  int	      laserHit;
  int	      superBulletFired;
  int	      superBulletHit;
  int	      shockWaveFired;
  int	      shockWaveHit;
  int	      thiefFired;
  int	      thiefHit;
};


inline int ShotStatistics::getNormalFired() const {
  return normalFired;
}

inline int ShotStatistics::getNormalHit() const {
  return normalHit;
}

inline int ShotStatistics::getGMFired() const {
  return guidedMissileFired;
}

inline int ShotStatistics::getGMHit() const {
  return guidedMissileHit;
}

inline int ShotStatistics::getLFired() const {
  return laserFired;
}

inline int ShotStatistics::getLHit() const {
  return laserHit;
}

inline int ShotStatistics::getSBFired() const {
  return superBulletFired;
}

inline int ShotStatistics::getSBHit() const {
  return superBulletHit;
}

inline int ShotStatistics::getSWFired() const {
  return shockWaveFired;
}

inline int ShotStatistics::getSWHit() const {
  return shockWaveHit;
}

inline int ShotStatistics::getTHFired() const {
  return thiefFired;
}

inline int ShotStatistics::getTHHit() const {
  return thiefHit;
}

inline int ShotStatistics::getTotalFired() const {
  return normalFired + guidedMissileFired + laserFired + superBulletFired +
         shockWaveFired + thiefFired;
}

inline int ShotStatistics::getTotalHit() const {
  return normalHit + guidedMissileHit + laserHit + superBulletHit +
         shockWaveHit + thiefHit;
}

inline int ShotStatistics::getTotalPerc() const {
  if (getTotalFired() == 0)
    return 100;
  return (int)(100 * ((float)getTotalHit() / (float)getTotalFired()));
}

inline void ShotStatistics::recordFire(FlagType* flag) {
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
}

inline void ShotStatistics::recordHit(FlagType* flag) {
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
#endif // __SHOTSTATISTICS_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
