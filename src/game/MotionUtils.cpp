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

/* interface header */

#include "MotionUtils.h"
#include "game/Flag.h"
#include "common/StateDatabase.h"
#include "game/BZDBCache.h"


float computeAngleVelocity(float old, float desired, float dt) {
  float newAngVel;
  float frames = 1.0f;

  // keyboard users
  if ((old * desired < 0.0f) || // reversed direction
      (NEAR_ZERO(desired, ZERO_TOLERANCE)) || // stopped
      (NEAR_ZERO(old - desired, ZERO_TOLERANCE))) {
    // close enough
    newAngVel = desired;
  }
  else  {
    /* dampened turning for aim control */

    /* mildly fudgey factor controls turn rate dampening.  should
     * converge roughly within converge and converge*converge
     * seconds (assuming converge is < 1).
     *
     * it would converge within .5 but we didn't accelerate
     * non-linearly by combining additional previous velocity, so
     * it's generally around sqrt(converge) * converge seconds
     * (i.e., around .35 sec for converge of .5)
     */
    static const float converge = 0.5f;

    /* spread out dampening over this many frames */
    frames = converge / dt;
    if (frames < 1.0f) {
      frames = 1.0f;  // framerate too low
    }

    /* accelerate towards desired */
    newAngVel = (old + (old / frames)) + (desired / frames);

    // if reached desired, clamp it
    if (desired > 0) {
      if (newAngVel > desired) {
        newAngVel = desired;
      }
    }
    else {
      if (newAngVel < desired) {
        newAngVel = desired;
      }
    }
  }
  return newAngVel;
}

float computeMaxAngleVelocity(FlagType* flag, float z) {
  float angvel = BZDB.eval(BZDBNAMES.TANKANGVEL);

  if (flag) {
    if (flag == Flags::QuickTurn) {
      return angvel * BZDB.eval(BZDBNAMES.ANGULARAD);
    }
    else if (flag == Flags::QuickTurn && z < 0.0f) {
      return angvel * BZDB.eval(BZDBNAMES.BURROWANGULARAD);
    }
  }

  return angvel;
}


float computeMaxLinVelocity(FlagType* flag, float z) {
  float speed = BZDB.eval(BZDBNAMES.TANKSPEED);

  if (flag) {
    if (flag == Flags::Velocity) {
      return speed * BZDB.eval(BZDBNAMES.VELOCITYAD);
    }
    else if (flag == Flags::Thief) {
      return speed * BZDB.eval(BZDBNAMES.THIEFVELAD);
    }
    else if (flag == Flags::QuickTurn && z < 0.0f) {
      return speed * BZDB.eval(BZDBNAMES.BURROWSPEEDAD);
    }
  }

  return speed;
}


void computeMomentum(float dt, FlagType* flag,
                     float& speed, float& angVel,
                     const float lastSpeed, const float lastAngVel) {
  static BZDB_float bzdbLinearMomentum(BZDBNAMES.MOMENTUMLINACC);
  static BZDB_float bzdbLinearInertia(BZDBNAMES.INERTIALINEAR);
  static BZDB_float bzdbAngularMomentum(BZDBNAMES.MOMENTUMANGACC);
  static BZDB_float bzdbAngularInertia(BZDBNAMES.INERTIAANGULAR);

  // get maximum linear and angular accelerations
  const float linearAcc = (flag == Flags::Momentum) ? bzdbLinearMomentum
                          : bzdbLinearInertia;
  const float angularAcc = (flag == Flags::Momentum) ? bzdbAngularMomentum
                           : bzdbAngularInertia;

  // limit linear acceleration
  if (linearAcc > 0.0f) {
    const float acc = (speed - lastSpeed) / dt;

    if (acc > 20.0f * linearAcc) {
      speed = lastSpeed + dt * 20.0f * linearAcc;
    }
    else if (acc < -20.0f * linearAcc) {
      speed = lastSpeed - dt * 20.0f * linearAcc;
    }
  }

  // limit angular acceleration
  if (angularAcc > 0.0f) {
    const float angAcc = (angVel - lastAngVel) / dt;
    if (angAcc > angularAcc) {
      angVel = lastAngVel + dt * angularAcc;
    }
    else if (angAcc < -angularAcc) {
      angVel = lastAngVel - dt * angularAcc;
    }
  }
}


void computeFriction(float dt, FlagType* flag,
                     const fvec3& oldVelocity, fvec3& newVelocity) {
  static BZDB_float bzdbFriction(BZDBNAMES.FRICTION);
  static BZDB_float bzdbMomentumFriction(BZDBNAMES.MOMENTUMFRICTION);

  const float friction = (flag == Flags::Momentum) ? bzdbFriction
                         : bzdbMomentumFriction;

  if (friction > 0.0f) {
    // limit vector acceleration


    float delta[2] = {newVelocity[0] - oldVelocity[0], newVelocity[1] - oldVelocity[1]};
    float acc2 = (delta[0] * delta[0] + delta[1] * delta[1]) / (dt * dt);
    float accLimit = 20.0f * friction;

    if (acc2 > accLimit * accLimit) {
      float ratio = accLimit / sqrtf(acc2);
      newVelocity[0] = oldVelocity[0] + delta[0] * ratio;
      newVelocity[1] = oldVelocity[1] + delta[1] * ratio;
    }
  }
}


float computeJumpVelocity(FlagType* flag) {
  if (flag == Flags::Wings) {
    return BZDB.eval(BZDBNAMES.WINGSJUMPVELOCITY);
  }

  return BZDB.eval(BZDBNAMES.JUMPVELOCITY);
}


float computeGroundLimit(FlagType* flag) {
  float groundLimit = 0.0f;
  if (flag == Flags::Burrow) {
    groundLimit = BZDB.eval(BZDBNAMES.BURROWDEPTH);
  }

  return groundLimit;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
