// SAMPLE_PLUGIN.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"

class SAMPLE_PLUGIN : public bz_Plugin
{
  virtual const char* Name (){return "SAMPLE PLUGIN";}
  virtual void Init ( const char* config);

  virtual void Event ( bz_EventData * /* eventData */ ){return;}
};

BZ_PLUGIN(SAMPLE_PLUGIN)

void SAMPLE_PLUGIN::Init ( const char* /*commandLine*/ )
{
  bz_debugMessage(4,"SAMPLE_PLUGIN plugin loaded");

  // init events here with Register();
}
// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

