/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __SHOT_H__
#define __SHOT_H__

#include "common.h"

/* system interface headers */
#include <ostream>

/* local interface headers */
#include "RemotePlayer.h"
#include "RCMessage.h"
#include "BZAdvancedRobot.h"


class Shot {
public:
  Shot();
  Shot(uint64_t _id);
  Shot(PlayerId _plr, uint16_t _sid);

  PlayerId getPlayerId(void) const;
  uint16_t getShotId(void) const;

  uint64_t getId(void) const;
  void setId(uint64_t id);

  messageParseStatus parse(char **arguments, int count);

  virtual void getPosition(double &x, double &y, double &z, double dt = 0) const; //Return the shots position in dt seconds
  virtual void getVelocity(double &x, double &y, double &z) const;

protected:
  uint64_t id;
};

class FrontendShot : public Shot
{
  public:
    FrontendShot();
    FrontendShot(uint64_t _id) : Shot(_id), robot(NULL) {}
    FrontendShot(const Shot &s) : Shot(s), robot(NULL) {}

    void setRobot(const BZAdvancedRobot *_robot);

    void getPosition(double &x, double &y, double &z, double dt = 0) const;
    void getVelocity(double &x, double &y, double &z) const;

    mutable double x, y, z;
    mutable double vx, vy, vz;

  protected:
    const BZAdvancedRobot *robot;
};

std::ostream& operator<<(std::ostream& os, const Shot& shot);

#else

class Shot;
class FrontendShot;
class BackendShot;

#endif /* __SHOT_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
