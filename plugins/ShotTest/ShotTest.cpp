// SAMPLE_PLUGIN.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"

BZ_GET_PLUGIN_VERSION

class ShotHandler : public bz_EventHandler
{
public:

  double startTime;
  double increment;

  ShotHandler()
  {
    startTime = -1;
    increment = 5.0;
  }

  void fireDamnitFire ( void )
  {
    if (!bz_anyPlayers())
      return;

    float origin[3] = {0,0,2};
    bz_fireWorldWep(NULL,origin);

    bz_sendTextMessage(BZ_SERVER,BZ_ALLUSERS,"Firing Sample Shot");
  }

  virtual void process ( bz_EventData *eventData )
  {
    double now = bz_getCurrentTime();
    if ( now - startTime > increment )
    {
      fireDamnitFire();
      startTime = now;
    }
  }
};

ShotHandler sh;

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
  bz_debugMessage(4,"ShotTest plug-in loaded");

  bz_setMaxWaitTime((float)sh.increment*0.1f,"ShotText");
  bz_registerEvent(bz_eTickEvent,&sh);
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_debugMessage(4,"ShotTest plug-in unloaded");

  bz_removeEvent(bz_eTickEvent,&sh);
  bz_clearMaxWaitTime("ShotText");
  return 0;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
