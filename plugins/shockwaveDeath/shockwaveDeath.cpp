// shockwaveDeath.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

// event handler callback
class SWDeathHandler : public bz_Plugin
{
public:
	virtual const char* Name (){return "Shockwave Death";}
	virtual void Init ( const char* config);
  virtual void	Event ( bz_EventData *eventData );
};

BZ_PLUGIN(SWDeathHandler)

void SWDeathHandler::Init( const char* commandLine )
{
  bz_debugMessage(4,"shockwaveDeath plugin loaded");

  Register(bz_ePlayerDieEvent);

  std::string param = commandLine;

  if (param == "usevictim")
	  bz_debugMessage(0,"shockwaveDeath plugin no longer takes any parameters");
}

void SWDeathHandler::Event ( bz_EventData *eventData )
{
  if (eventData->eventType != bz_ePlayerDieEvent)
    return;

  bz_PlayerDieEventData_V1 *dieData = (bz_PlayerDieEventData_V1*)eventData;

  int playerToUse = BZ_SERVER;

  float reloadTime = (float)bz_getBZDBDouble("_reloadTime");

  if (bz_BZDBItemExists("_swDeathReloadFactor") && bz_getBZDBDouble("_swDeathReloadFactor") > 0)
    reloadTime *= (float)bz_getBZDBDouble("_swDeathReloadFactor");

  bz_fireWorldWep("SW",reloadTime,playerToUse,dieData->state.pos,0,0,0,0.0f);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

