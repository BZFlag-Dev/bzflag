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

// interface header
#include "PlayerState.h"

// system headers
#include <math.h>

// common implementation headers
#include "BZDBCache.h"
#include "NetMessage.h"
#include "Pack.h"
#include "Protocol.h"


// the full scale of a int16_t  (less 1.0 for safety)
const float smallScale     = 32766.0f;

// 2 cm resolution  (range: +/- 655.32 meters)
const float smallMaxDist   = 0.02f * smallScale;

// 1 cm/sec resolution  (range: +/- 327.66 meters/sec)
const float smallMaxVel    = 0.01f * smallScale;

// 0.001 radians/sec resolution  (range: +/- 32.766 rads/sec)
const float smallMaxAngVel = 0.001f * smallScale;


PlayerState::PlayerState()
: order(0)
, status(DeadStatus)
, pos(0.0f, 0.0f, 0.0f)
, velocity(0.0f, 0.0f, 0.0f)
, azimuth(0.0f)
, angVel(0.0f)
, phydrv(-1)
, apparentVelocity(0.0f, 0.0f, 0.0f)
, lastUpdateTime(BzTime::getSunGenesisTime())
, userSpeed(0.0f)
, userAngVel(0.0f)
, jumpJetsScale(0.0f)
, sounds(NoSounds)
{
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


void* PlayerState::pack(void* buf, uint16_t& code, bool increment)
{
  if (increment) {
    order++;
  }

  buf = nboPackInt32(buf, int32_t(order));
  buf = nboPackInt16(buf, int16_t(status));

  static BZDB_bool noSmallPackets(BZDBNAMES.NOSMALLPACKETS);
  if (noSmallPackets                     ||
      (fabsf(pos.x) >= smallMaxDist)     ||
      (fabsf(pos.y) >= smallMaxDist)     ||
      (fabsf(pos.z) >= smallMaxDist)     ||
      (fabsf(velocity.x) >= smallMaxVel) ||
      (fabsf(velocity.y) >= smallMaxVel) ||
      (fabsf(velocity.z) >= smallMaxVel) ||
      (fabsf(angVel) >= smallMaxAngVel)) {

    code = MsgPlayerUpdate;

    buf = nboPackFVec3(buf, pos);
    buf = nboPackFVec3(buf, velocity);
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
    } else if (angle < -M_PI) {
      angle += (float)(M_PI * 2.0);
    }
    aziShort    = (int16_t) ((angle * smallScale) / M_PI);
    angVelShort = (int16_t) ((angVel * smallScale) / smallMaxAngVel);

    buf = nboPackInt16(buf, posShort[0]);
    buf = nboPackInt16(buf, posShort[1]);
    buf = nboPackInt16(buf, posShort[2]);
    buf = nboPackInt16(buf, velShort[0]);
    buf = nboPackInt16(buf, velShort[1]);
    buf = nboPackInt16(buf, velShort[2]);
    buf = nboPackInt16(buf, aziShort);
    buf = nboPackInt16(buf, angVelShort);
  }

  if ((status & JumpJets) != 0) {
    float tmp = clampedValue(jumpJetsScale, 1.0f);
    buf = nboPackInt16(buf, (int16_t) (tmp * smallScale));
  }

  if ((status & OnDriver) != 0) {
    buf = nboPackInt32(buf, phydrv);
  }

  if ((status & UserInputs) != 0) {
    float tmp;
    // pack userSpeed
    tmp = clampedValue(userSpeed, smallMaxVel);
    int16_t speed = (int16_t) ((tmp * smallScale) / smallMaxVel);
    buf = nboPackInt16(buf, speed);
    // pack userAngVel
    tmp = clampedValue(userAngVel, smallMaxAngVel);
    int16_t angvel = (int16_t) ((tmp * smallScale) / smallMaxAngVel);
    buf = nboPackInt16(buf, angvel);
  }

  if ((status & PlaySound) != 0) {
    buf = nboPackUInt8(buf, sounds);
  }

  return buf;
}


void PlayerState::pack(NetMessage& netMsg, uint16_t& code, bool increment)
{
  if (increment) {
    order++;
  }

  netMsg.packInt32(int32_t(order));
  netMsg.packInt16(int16_t(status));

  static BZDB_bool noSmallPackets(BZDBNAMES.NOSMALLPACKETS);
  if (noSmallPackets                     ||
      (fabsf(pos.x) >= smallMaxDist)     ||
      (fabsf(pos.y) >= smallMaxDist)     ||
      (fabsf(pos.z) >= smallMaxDist)     ||
      (fabsf(velocity.x) >= smallMaxVel) ||
      (fabsf(velocity.y) >= smallMaxVel) ||
      (fabsf(velocity.z) >= smallMaxVel) ||
      (fabsf(angVel) >= smallMaxAngVel)) {

    code = MsgPlayerUpdate;

    netMsg.packFVec3(pos);
    netMsg.packFVec3(velocity);
    netMsg.packFloat(azimuth);
    netMsg.packFloat(angVel);
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
    } else if (angle < -M_PI) {
      angle += (float)(M_PI * 2.0);
    }
    aziShort = (int16_t) ((angle * smallScale) / M_PI);
    angVelShort = (int16_t) ((angVel * smallScale) / smallMaxAngVel);

    netMsg.packInt16(posShort[0]);
    netMsg.packInt16(posShort[1]);
    netMsg.packInt16(posShort[2]);
    netMsg.packInt16(velShort[0]);
    netMsg.packInt16(velShort[1]);
    netMsg.packInt16(velShort[2]);
    netMsg.packInt16(aziShort);
    netMsg.packInt16(angVelShort);
  }

  if ((status & JumpJets) != 0) {
    float tmp = clampedValue(jumpJetsScale, 1.0f);
    netMsg.packInt16((int16_t) (tmp * smallScale));
  }

  if ((status & OnDriver) != 0) {
    netMsg.packInt32(phydrv);
  }

  if ((status & UserInputs) != 0) {
    float tmp;
    // pack userSpeed
    tmp = clampedValue(userSpeed, smallMaxVel);
    int16_t speed = (int16_t) ((tmp * smallScale) / smallMaxVel);
    netMsg.packInt16(speed);
    // pack userAngVel
    tmp = clampedValue(userAngVel, smallMaxAngVel);
    int16_t angvel = (int16_t) ((tmp * smallScale) / smallMaxAngVel);
    netMsg.packInt16(angvel);
  }

  if ((status & PlaySound) != 0) {
    netMsg.packUInt8(sounds);
  }
}


void* PlayerState::unpack(void* buf, uint16_t code)
{
  int32_t inOrder;
  int16_t inStatus;
  buf = nboUnpackInt32(buf, inOrder);
  buf = nboUnpackInt16(buf, inStatus);
  order = int(inOrder);
  status = short(inStatus);

  if (code == MsgPlayerUpdate) {
    buf = nboUnpackFVec3(buf, pos);
    buf = nboUnpackFVec3(buf, velocity);
    buf = nboUnpackFloat(buf, azimuth);
    buf = nboUnpackFloat(buf, angVel);
  }
  else {
    int16_t posShort[3], velShort[3], aziShort, angVelShort;

    buf = nboUnpackInt16(buf, posShort[0]);
    buf = nboUnpackInt16(buf, posShort[1]);
    buf = nboUnpackInt16(buf, posShort[2]);
    buf = nboUnpackInt16(buf, velShort[0]);
    buf = nboUnpackInt16(buf, velShort[1]);
    buf = nboUnpackInt16(buf, velShort[2]);
    buf = nboUnpackInt16(buf, aziShort);
    buf = nboUnpackInt16(buf, angVelShort);

    for (int i=0; i<3; i++) {
      pos[i] = ((float)posShort[i] * smallMaxDist) / smallScale;
      velocity[i] = ((float)velShort[i] * smallMaxVel) / smallScale;
    }
    azimuth = (float)((aziShort * M_PI) / smallScale);
    angVel = ((float)angVelShort * smallMaxAngVel) / smallScale;
  }

  if ((inStatus & JumpJets) != 0) {
    int16_t jumpJetsShort;
    buf = nboUnpackInt16(buf, jumpJetsShort);
    jumpJetsScale = ((float)jumpJetsShort) / smallScale;
  } else {
    jumpJetsScale = 0.0f;
  }

  if ((inStatus & OnDriver) != 0) {
    int32_t inPhyDrv;
    buf = nboUnpackInt32(buf, inPhyDrv);
    phydrv = int(inPhyDrv);
  } else {
    phydrv = -1;
  }

  if ((inStatus & UserInputs) != 0) {
    int16_t userSpeedShort, userAngVelShort;
    buf = nboUnpackInt16(buf, userSpeedShort);
    buf = nboUnpackInt16(buf, userAngVelShort);
    userSpeed = ((float)userSpeedShort * smallMaxVel) / smallScale;
    userAngVel = ((float)userAngVelShort * smallMaxAngVel) / smallScale;
  } else {
    userSpeed = 0.0f;
    userAngVel = 0.0f;
  }

  if ((inStatus & PlaySound) != 0) {
    buf = nboUnpackUInt8(buf, sounds);
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
