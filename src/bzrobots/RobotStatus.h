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

#ifndef __BZROBOTS_ROBOTSTATUS_H__
#define __BZROBOTS_ROBOTSTATUS_H__

namespace BZRobots {

  class RobotStatus {
    public:
      RobotStatus();
      RobotStatus(
        double _botDistanceRemaining,
        double _botEnergy,
        double _botGunHeading,
        double _botGunHeadingRadians,
        double _botGunHeat,
        double _botGunTurnRemaining,
        double _botGunTurnRemainingRadians,
        double _botHeading,
        double _botHeadingRadians,
        int    _botNumRounds,
        int    _botOthers,
        double _botRadarHeading,
        double _botRadarHeadingRadians,
        double _botRadarTurnRemaining,
        double _botRadarTurnRemainingRadians,
        int    _botRoundNum,
        double _botTime,
        double _botTurnRemaining,
        double _botTurnRemainingRadians,
        double _botVelocity,
        double _botX,
        double _botY,
        double _botZ
      );
      ~RobotStatus();

      inline double  getDistanceRemaining() const { return botDistanceRemaining; }
      inline double  getEnergy() const { return botEnergy; }
      inline double  getGunHeading() const { return botGunHeading; }
      inline double  getGunHeadingRadians() const { return botGunHeadingRadians; }
      inline double  getGunHeat() const { return botGunHeat; }
      inline double  getGunTurnRemaining() const { return botGunTurnRemaining; }
      inline double  getGunTurnRemainingRadians() const { return botGunTurnRemainingRadians; }
      inline double  getHeading() const { return botHeading; }
      inline double  getHeadingRadians() const { return botHeadingRadians; }
      inline int     getNumRounds() const { return botNumRounds; }
      inline int     getOthers() const { return botOthers; }
      inline double  getRadarHeading() const { return botRadarHeading; }
      inline double  getRadarHeadingRadians() const { return botRadarHeadingRadians; }
      inline double  getRadarTurnRemaining() const { return botRadarTurnRemaining; }
      inline double  getRadarTurnRemainingRadians() const { return botRadarTurnRemainingRadians; }
      inline int     getRoundNum() const { return botRoundNum; }
      inline double  getTime() const { return botTime; }
      inline double  getTurnRemaining() const { return botTurnRemaining; }
      inline double  getTurnRemainingRadians() const { return botTurnRemainingRadians; }
      inline double  getVelocity() const { return botVelocity; }
      inline double  getX() const { return botX; }
      inline double  getY() const { return botY; }
      inline double  getZ() const { return botZ; }

    protected:
      double botDistanceRemaining;
      double botEnergy;
      double botGunHeading;
      double botGunHeadingRadians;
      double botGunHeat;
      double botGunTurnRemaining;
      double botGunTurnRemainingRadians;
      double botHeading;
      double botHeadingRadians;
      int    botNumRounds;
      int    botOthers;
      double botRadarHeading;
      double botRadarHeadingRadians;
      double botRadarTurnRemaining;
      double botRadarTurnRemainingRadians;
      int    botRoundNum;
      double botTime;
      double botTurnRemaining;
      double botTurnRemainingRadians;
      double botVelocity;
      double botX;
      double botY;
      double botZ;
  };

} // namespace BZRobots

#else
namespace BZRobots {
  class RobotStatus;
}
#endif /* __BZROBOTS_ROBOTSTATUS_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
