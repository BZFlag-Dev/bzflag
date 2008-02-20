/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "RCMessageFactory.h"

/* local implementation headers */
#include "RCRequests.h"
#include "RCReplies.h"
#include "RCEvents.h"


#define ADD_REQUEST(COMMAND) RCREQUEST.Register<COMMAND ## Req>( #COMMAND );
#define ADD_REPLY(COMMAND) RCREPLY.Register<COMMAND ## Reply>( #COMMAND );
#define ADD_EVENT(COMMAND) RCEVENT.Register<COMMAND ## Event>( #COMMAND );


template<>
void RCMessageFactory<RCRequest>::initialize()
{
  RCREQUEST.Register<IdentifyFrontend>("IdentifyFrontend");
  ADD_REQUEST(Execute);
  ADD_REQUEST(SetSpeed);
  ADD_REQUEST(SetTurnRate);
  ADD_REQUEST(SetAhead);
  ADD_REQUEST(SetTurnLeft);
  ADD_REQUEST(SetFire);
  ADD_REQUEST(GetGunHeat);
  ADD_REQUEST(GetDistanceRemaining);
  ADD_REQUEST(GetTurnRemaining);
  ADD_REQUEST(GetTickDuration);
  ADD_REQUEST(SetTickDuration);
  ADD_REQUEST(GetTickRemaining);
  ADD_REQUEST(GetBattleFieldSize);
  ADD_REQUEST(GetPlayers);
  ADD_REQUEST(GetTeams);
  ADD_REQUEST(GetBases);
  ADD_REQUEST(GetObstacles);
  ADD_REQUEST(GetFlags);
  ADD_REQUEST(GetShots);
  ADD_REQUEST(GetMyTanks);
  ADD_REQUEST(GetOtherTanks);
  ADD_REQUEST(GetConstants);
  ADD_REQUEST(GetX);
  ADD_REQUEST(GetY);
  ADD_REQUEST(GetZ);
  ADD_REQUEST(SetResume);
  ADD_REQUEST(GetWidth);
  ADD_REQUEST(GetHeight);
  ADD_REQUEST(GetLength);
  ADD_REQUEST(GetHeading);
  ADD_REQUEST(GetObstacles);
  ADD_REQUEST(GetShotPosition);
}


template<>
void RCMessageFactory<RCReply>::initialize()
{
  RCREPLY.Register<IdentifyBackend>("IdentifyBackend");
  ADD_REPLY(Event);
  ADD_REPLY(CommandDone);
  ADD_REPLY(GunHeat);
  ADD_REPLY(DistanceRemaining);
  ADD_REPLY(TurnRemaining);
  ADD_REPLY(TickDuration);
  ADD_REPLY(TickRemaining);
  ADD_REPLY(BattleFieldSize);
  ADD_REPLY(X);
  ADD_REPLY(Y);
  ADD_REPLY(Z);
  ADD_REPLY(Width);
  ADD_REPLY(Height);
  ADD_REPLY(Length);
  ADD_REPLY(Heading);
  ADD_REPLY(PlayersBegin);
  ADD_REPLY(Players);
  ADD_REPLY(ObstaclesBegin);
  ADD_REPLY(Obstacle);
  ADD_REPLY(ShotsBegin);
  ADD_REPLY(Shot);
  ADD_REPLY(ShotPosition);
}


template<>
void RCMessageFactory<RCEvent>::initialize()
{
  ADD_EVENT(HitWall);
  ADD_EVENT(Death);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
