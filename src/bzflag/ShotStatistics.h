/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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

  ShotStatistics();
  ~ShotStatistics();

  // raw stats returns
  int	 getNormalFired() const;
  int	 getNormalHit() const;
  int	 getGMFired() const;
  int	 getGMHit() const;
  int	 getLFired() const;
  int	 getLHit() const;
  int	 getSBFired() const;
  int	 getSBHit() const;
  int	 getSWFired() const;
  int	 getSWHit() const;
  int	 getTHFired() const;
  int	 getTHHit() const;

  // stats processing
  int	 getTotalFired() const;
  int	 getTotalHit() const;
  int	 getTotalPerc() const;
  FlagType*   getFavoriteFlag() const;
  FlagType*   getBestFlag() const;

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

#endif // __SHOTSTATISTICS_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
