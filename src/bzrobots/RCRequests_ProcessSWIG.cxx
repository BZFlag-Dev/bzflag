#include "RCRequests.h"

#define SWIG_PROCESS(request) bool request::process(RCRobotPlayer *) { return true; }

SWIG_PROCESS(ExecuteReq)
SWIG_PROCESS(SetSpeedReq)
SWIG_PROCESS(SetTurnRateReq)
SWIG_PROCESS(SetAheadReq)
SWIG_PROCESS(SetTurnLeftReq)
SWIG_PROCESS(SetFireReq)
SWIG_PROCESS(SetResumeReq)
SWIG_PROCESS(GetGunHeatReq)
SWIG_PROCESS(GetDistanceRemainingReq)
SWIG_PROCESS(GetTurnRemainingReq)
SWIG_PROCESS(GetTickDurationReq)
SWIG_PROCESS(SetTickDurationReq)
SWIG_PROCESS(GetTickRemainingReq)
SWIG_PROCESS(GetBattleFieldSizeReq)
SWIG_PROCESS(GetTeamsReq)
SWIG_PROCESS(GetBasesReq)
SWIG_PROCESS(GetObstaclesReq)
SWIG_PROCESS(GetFlagsReq)
SWIG_PROCESS(GetShotsReq)
SWIG_PROCESS(GetMyTanksReq)
SWIG_PROCESS(GetOtherTanksReq)
SWIG_PROCESS(GetConstantsReq)
SWIG_PROCESS(GetXReq)
SWIG_PROCESS(GetYReq)
SWIG_PROCESS(GetZReq)
SWIG_PROCESS(GetWidthReq)
SWIG_PROCESS(GetLengthReq)
SWIG_PROCESS(GetHeightReq)
SWIG_PROCESS(GetHeadingReq)
SWIG_PROCESS(SetStopReq)
SWIG_PROCESS(GetPlayersReq)

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
