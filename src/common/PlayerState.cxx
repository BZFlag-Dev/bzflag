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

#include "common.h"
#include "math.h"
#include "PlayerState.h"
#include "Pack.h"
#include "Protocol.h"
#include "StateDatabase.h"

PlayerState::PlayerState()
: order(0), status(DeadStatus), azimuth(0.0f), angVel(0.0f)
{
  pos[0] = pos[1] = pos[2] = 0.0f;
  velocity[0] = velocity[0] = velocity[2] = 0.0f;
}

void*	PlayerState::pack(void* buf, uint16_t& code)
{
  order++;
  buf = nboPackInt(buf, int32_t(order));
  buf = nboPackShort(buf, int16_t(status));

  if (BZDB.eval(StateDatabase::BZDB_NOSMALLPACKETS) > 0.0f) {

    code = MsgPlayerUpdate;
    
    buf = nboPackVector(buf, pos);
    buf = nboPackVector(buf, velocity);
    buf = nboPackFloat(buf, azimuth);
    buf = nboPackFloat(buf, angVel);
  }
  else {

    code = MsgPlayerUpdateSmall;
    
    float worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
    float maxVel = BZDB.eval(StateDatabase::BZDB_MAXVEL);
    float maxAngVel = BZDB.eval(StateDatabase::BZDB_MAXANGVEL);
    float tmpf;
    int16_t posShort[3], velShort[3], aziShort, angVelShort;
    
    for (int i=0; i<3; i++) {
      if (pos[i] >= worldSize) 
        posShort[i] = 32767;
      else if (pos[i] <= -worldSize) 
        posShort[i] = -32767;
      else {
        posShort[i] = (int16_t) ((pos[i] * 32767.0f) / worldSize);
      }
      if (velocity[i] >= maxVel) 
        velShort[i] = 32767;
      else if (velocity[i] <= -maxVel) 
        velShort[i] = -32767;
      else {
        velShort[i] = (int16_t) ((velocity[i] * 32767.0f) / maxVel);
      }
    }
    
    tmpf = fmodf (azimuth + M_PI, 2.0f * M_PI) - M_PI; // between -M_PI and M_PI
    aziShort = (int16_t) ((tmpf * 32767.0f) / M_PI);
    angVelShort = (int16_t) ((angVel * 32767.0f) / maxAngVel);

    buf = nboPackShort(buf, posShort[0]);
    buf = nboPackShort(buf, posShort[1]);
    buf = nboPackShort(buf, posShort[2]);
    buf = nboPackShort(buf, velShort[0]);
    buf = nboPackShort(buf, velShort[1]);
    buf = nboPackShort(buf, velShort[2]);
    buf = nboPackShort(buf, aziShort);
    buf = nboPackShort(buf, angVelShort);
  }
  return buf;
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
    float worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
    float maxVel = BZDB.eval(StateDatabase::BZDB_MAXVEL);
    float maxAngVel = BZDB.eval(StateDatabase::BZDB_MAXANGVEL);
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
      pos[i] = ((float)posShort[i] * worldSize) / 32767.0f;
      velocity[i] = ((float)velShort[i] * maxVel) / 32767.0f;
    }
    azimuth = ((float)aziShort * M_PI) / 32767.0f;
    angVel = ((float)angVelShort * maxAngVel) / 32767.0f;
  }
  return buf;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
