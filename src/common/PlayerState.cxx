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

// interface header
#include "PlayerState.h"

// system headers
#include <math.h>

// local implementation headers
#include "common.h"
#include "Pack.h"
#include "Protocol.h"
#include "StateDatabase.h"

// the full scale of a int16_t  (less 1.0 for safety)
const float smallScale     = 32766.0f;

// 2 cm resolution  (range: +/- 655.32 meters)
const float smallMaxDist   = 0.02f * smallScale;

// 1 cm/sec resolution  (range: +/- 327.66 meters/sec)
const float smallMaxVel    = 0.01f * smallScale;

// 0.001 radians/sec resolution  (range: +/- 32.766 rads/sec)
const float smallMaxAngVel = 0.001f * smallScale;


PlayerState::PlayerState()
  : order(0), status(DeadStatus), azimuth(0.0f), angVel(0.0f)
{
  pos[0] = pos[1] = pos[2] = 0.0f;
  velocity[0] = velocity[0] = velocity[2] = 0.0f;
  phydrv = -1;
  userSpeed = 0.0f;
  userAngVel = 0.0f;
  jumpJetsScale = 0.0f;
  sounds = NoSounds;

  apparentVelocity[0] = apparentVelocity[1] = apparentVelocity[2] = 0.0f;
  lastUpdateTime = -1;

  return;
}


static float clampedValue(float input, float max)
{
  if (input > max) {
    return max;
  } else if (input < -max) {
    return -max;
  } else {
    return input;
  }
}

void*	PlayerState::pack(void* buf, uint16_t& code, bool increment)
{
	if (increment)
		order++;

  buf = nboPackInt(buf, int32_t(order));
  buf = nboPackShort(buf, int16_t(status));

  if ((BZDB.eval(StateDatabase::BZDB_NOSMALLPACKETS) > 0.0f) ||
      (fabsf (pos[0]) >= smallMaxDist)      ||
      (fabsf (pos[1]) >= smallMaxDist)      ||
      (fabsf (pos[2]) >= smallMaxDist)      ||
      (fabsf (velocity[0]) >= smallMaxVel)  ||
      (fabsf (velocity[1]) >= smallMaxVel)  ||
      (fabsf (velocity[2]) >= smallMaxVel)  ||
      (fabsf (angVel) >= smallMaxAngVel)) {

    code = MsgPlayerUpdate;

    buf = nboPackVector(buf, pos);
    buf = nboPackVector(buf, velocity);
    buf = nboPackFloat(buf, azimuth);
    buf = nboPackFloat(buf, angVel);
  }
  else {

    code = MsgPlayerUpdateSmall;

    int16_t posShort[3], velShort[3], aziShort, angVelShort;

    for (int i=0; i<3; i++) {
      posShort[i] = (int16_t) ((pos[i] * smallScale) / smallMaxDist);
      velShort[i] = (int16_t) ((velocity[i] * smallScale) / smallMaxVel);
    }

    // put the angle between -M_PI and +M_PI
    float angle = fmodf (azimuth, (float)M_PI * 2.0f);
    if (angle > M_PI) {
      angle -= (float)(M_PI * 2.0);
    }
    else if (angle < -M_PI) {
      angle += (float)(M_PI * 2.0);
    }
    aziShort = (int16_t) ((angle * smallScale) / M_PI);
    angVelShort = (int16_t) ((angVel * smallScale) / smallMaxAngVel);

    buf = nboPackShort(buf, posShort[0]);
    buf = nboPackShort(buf, posShort[1]);
    buf = nboPackShort(buf, posShort[2]);
    buf = nboPackShort(buf, velShort[0]);
    buf = nboPackShort(buf, velShort[1]);
    buf = nboPackShort(buf, velShort[2]);
    buf = nboPackShort(buf, aziShort);
    buf = nboPackShort(buf, angVelShort);
  }

  if ((status & JumpJets) != 0) {
    float tmp = clampedValue(jumpJetsScale, 1.0f);
    buf = nboPackShort(buf, (int16_t) (tmp * smallScale));
  }

  if ((status & OnDriver) != 0) {
    buf = nboPackInt(buf, phydrv);
  }

  if ((status & UserInputs) != 0) {
    float tmp;
    // pack userSpeed
    tmp = clampedValue(userSpeed, smallMaxVel);
    int16_t speed = (int16_t) ((tmp * smallScale) / smallMaxVel);
    buf = nboPackShort(buf, speed);
    // pack userAngVel
    tmp = clampedValue(userAngVel, smallMaxAngVel);
    int16_t angvel = (int16_t) ((tmp * smallScale) / smallMaxAngVel);
    buf = nboPackShort(buf, angvel);
  }

  if ((status & PlaySound) != 0) {
    buf = nboPackUByte(buf, sounds);
  }

  return buf;
}



void	PlayerState::pack(BufferedNetworkMessage *msg, uint16_t& code, bool increment)
{
  if (increment)
    order++;

  msg->packInt(int32_t(order));
  msg->packShort(int16_t(status));

  if ((BZDB.eval(StateDatabase::BZDB_NOSMALLPACKETS) > 0.0f) ||
      (fabsf (pos[0]) >= smallMaxDist)      ||
      (fabsf (pos[1]) >= smallMaxDist)      ||
      (fabsf (pos[2]) >= smallMaxDist)      ||
      (fabsf (velocity[0]) >= smallMaxVel)  ||
      (fabsf (velocity[1]) >= smallMaxVel)  ||
      (fabsf (velocity[2]) >= smallMaxVel)  ||
      (fabsf (angVel) >= smallMaxAngVel)) {

    code = MsgPlayerUpdate;

    msg->packVector(pos);
    msg->packVector(velocity);
    msg->packFloat(azimuth);
    msg->packFloat(angVel);
  }
  else {

    code = MsgPlayerUpdateSmall;

    int16_t posShort[3], velShort[3], aziShort, angVelShort;

    for (int i=0; i<3; i++) {
      posShort[i] = (int16_t) ((pos[i] * smallScale) / smallMaxDist);
      velShort[i] = (int16_t) ((velocity[i] * smallScale) / smallMaxVel);
    }

    // put the angle between -M_PI and +M_PI
    float angle = fmodf (azimuth, (float)M_PI * 2.0f);
    if (angle > M_PI) {
      angle -= (float)(M_PI * 2.0);
    }
    else if (angle < -M_PI) {
      angle += (float)(M_PI * 2.0);
    }
    aziShort = (int16_t) ((angle * smallScale) / M_PI);
    angVelShort = (int16_t) ((angVel * smallScale) / smallMaxAngVel);

    msg->packShort(posShort[0]);
    msg->packShort(posShort[1]);
    msg->packShort(posShort[2]);
    msg->packShort(velShort[0]);
    msg->packShort(velShort[1]);
    msg->packShort(velShort[2]);
    msg->packShort(aziShort);
    msg->packShort(angVelShort);
  }

  if ((status & JumpJets) != 0) {
    float tmp = clampedValue(jumpJetsScale, 1.0f);
    msg->packShort((int16_t) (tmp * smallScale));
  }

  if ((status & OnDriver) != 0) {
    msg->packInt(phydrv);
  }

  if ((status & UserInputs) != 0) {
    float tmp;
    // pack userSpeed
    tmp = clampedValue(userSpeed, smallMaxVel);
    int16_t speed = (int16_t) ((tmp * smallScale) / smallMaxVel);
    msg->packShort(speed);
    // pack userAngVel
    tmp = clampedValue(userAngVel, smallMaxAngVel);
    int16_t angvel = (int16_t) ((tmp * smallScale) / smallMaxAngVel);
    msg->packShort(angvel);
  }

  if ((status & PlaySound) != 0) {
    msg->packUByte(sounds);
  }
}


void*	PlayerState::unpack(void* buf, uint16_t code)
{
  int32_t inOrder;
  int16_t inStatus;
  buf = nboUnpackInt(buf, inOrder);
  buf = nboUnpackShort(buf, inStatus);
  order = int(inOrder);
  status = short(inStatus);

  if (code == MsgPlayerUpdate) {
    buf = nboUnpackVector(buf, pos);
    buf = nboUnpackVector(buf, velocity);
    buf = nboUnpackFloat(buf, azimuth);
    buf = nboUnpackFloat(buf, angVel);
  }
  else {
    int16_t posShort[3], velShort[3], aziShort, angVelShort;

    buf = nboUnpackShort(buf, posShort[0]);
    buf = nboUnpackShort(buf, posShort[1]);
    buf = nboUnpackShort(buf, posShort[2]);
    buf = nboUnpackShort(buf, velShort[0]);
    buf = nboUnpackShort(buf, velShort[1]);
    buf = nboUnpackShort(buf, velShort[2]);
    buf = nboUnpackShort(buf, aziShort);
    buf = nboUnpackShort(buf, angVelShort);

    for (int i=0; i<3; i++) {
      pos[i] = ((float)posShort[i] * smallMaxDist) / smallScale;
      velocity[i] = ((float)velShort[i] * smallMaxVel) / smallScale;
    }
    azimuth = (float)((aziShort * M_PI) / smallScale);
    angVel = ((float)angVelShort * smallMaxAngVel) / smallScale;
  }

  if ((inStatus & JumpJets) != 0) {
    int16_t jumpJetsShort;
    buf = nboUnpackShort(buf, jumpJetsShort);
    jumpJetsScale = ((float)jumpJetsShort) / smallScale;
  } else {
    jumpJetsScale = 0.0f;
  }

  if ((inStatus & OnDriver) != 0) {
    int32_t inPhyDrv;
    buf = nboUnpackInt(buf, inPhyDrv);
    phydrv = int(inPhyDrv);
  } else {
    phydrv = -1;
  }

  if ((inStatus & UserInputs) != 0) {
    int16_t userSpeedShort, userAngVelShort;
    buf = nboUnpackShort(buf, userSpeedShort);
    buf = nboUnpackShort(buf, userAngVelShort);
    userSpeed = ((float)userSpeedShort * smallMaxVel) / smallScale;
    userAngVel = ((float)userAngVelShort * smallMaxAngVel) / smallScale;
  } else {
    userSpeed = 0.0f;
    userAngVel = 0.0f;
  }

  if ((inStatus & PlaySound) != 0) {
    buf = nboUnpackUByte(buf, sounds);
  } else {
    sounds = NoSounds;
  }

  return buf;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
