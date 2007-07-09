#include "RCMessageFactory.h"

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
    ADD_LOOKUP(GetTeams);
    ADD_LOOKUP(GetBases);
    ADD_LOOKUP(GetObstacles);
    ADD_LOOKUP(GetFlags);
    ADD_LOOKUP(GetShots);
    ADD_LOOKUP(GetMyTanks);
    ADD_LOOKUP(GetOtherTanks);
    ADD_LOOKUP(GetConstants);
}
#undef ADD_LOOKUP

#define ADD_LOOKUP(COMMAND) RCREPLY.Register<COMMAND ## Reply>( #COMMAND );
template<>
void RCMessageFactory<RCReply>::initialize()
{
    RCREPLY.Register<IdentifyBackend>("IdentifyBackend");
    ADD_LOOKUP(CommandDone);
    ADD_LOOKUP(GunHeat);
    ADD_LOOKUP(DistanceRemaining);
    ADD_LOOKUP(TurnRemaining);
    ADD_LOOKUP(TickDuration);
    ADD_LOOKUP(TickRemaining);
    ADD_LOOKUP(BattleFieldSize);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

