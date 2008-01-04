/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "RCMessageFactory.h"

#include "RCRequests.h"
#include "RCReplies.h"
#include "RCEvents.h"

#define ADD_LOOKUP(COMMAND) RCREQUEST.Register<COMMAND ## Req>( #COMMAND );
template<>
void RCMessageFactory<RCRequest>::initialize()
{
    RCREQUEST.Register<IdentifyFrontend>("IdentifyFrontend");
    ADD_LOOKUP(Execute);
    ADD_LOOKUP(SetSpeed);
    ADD_LOOKUP(SetTurnRate);
    ADD_LOOKUP(SetAhead);
    ADD_LOOKUP(SetTurnLeft);
    ADD_LOOKUP(SetFire);
    ADD_LOOKUP(GetGunHeat);
    ADD_LOOKUP(GetDistanceRemaining);
    ADD_LOOKUP(GetTurnRemaining);
    ADD_LOOKUP(GetTickDuration);
    ADD_LOOKUP(SetTickDuration);
    ADD_LOOKUP(GetTickRemaining);
    ADD_LOOKUP(GetBattleFieldSize);
    ADD_LOOKUP(GetPlayers);
    ADD_LOOKUP(GetTeams);
    ADD_LOOKUP(GetBases);
    ADD_LOOKUP(GetObstacles);
    ADD_LOOKUP(GetFlags);
    ADD_LOOKUP(GetShots);
    ADD_LOOKUP(GetMyTanks);
    ADD_LOOKUP(GetOtherTanks);
    ADD_LOOKUP(GetConstants);
    ADD_LOOKUP(GetX);
    ADD_LOOKUP(GetY);
    ADD_LOOKUP(GetZ);
    ADD_LOOKUP(SetResume);
    ADD_LOOKUP(GetWidth);
    ADD_LOOKUP(GetHeight);
    ADD_LOOKUP(GetLength);
    ADD_LOOKUP(GetHeading);
}
#undef ADD_LOOKUP

#define ADD_LOOKUP(COMMAND) RCREPLY.Register<COMMAND ## Reply>( #COMMAND );
template<>
void RCMessageFactory<RCReply>::initialize()
{
    RCREPLY.Register<IdentifyBackend>("IdentifyBackend");
    ADD_LOOKUP(Event);
    ADD_LOOKUP(CommandDone);
    ADD_LOOKUP(GunHeat);
    ADD_LOOKUP(DistanceRemaining);
    ADD_LOOKUP(TurnRemaining);
    ADD_LOOKUP(TickDuration);
    ADD_LOOKUP(TickRemaining);
    ADD_LOOKUP(BattleFieldSize);
    ADD_LOOKUP(X);
    ADD_LOOKUP(Y);
    ADD_LOOKUP(Z);
    ADD_LOOKUP(Width);
    ADD_LOOKUP(Height);
    ADD_LOOKUP(Length);
    ADD_LOOKUP(Heading);
    ADD_LOOKUP(PlayersBegin);
    ADD_LOOKUP(Players);
}
#undef ADD_LOOKUP

#define ADD_LOOKUP(COMMAND) RCEVENT.Register<COMMAND ## Event>( #COMMAND );
template<>
void RCMessageFactory<RCEvent>::initialize()
{
  ADD_LOOKUP(HitWall);
}
#undef ADD_LOOKUP

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
