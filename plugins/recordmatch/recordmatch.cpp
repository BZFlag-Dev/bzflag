// recordmatch.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <stdio.h>

class GameStartEndHandler : public bz_Plugin
{
public:
  virtual const char* Name () {return "Record Match";}
  virtual void Init ( const char* config );
  virtual void Event ( bz_EventData *eventData );

  bool started;
  std::string filename;
};

BZ_PLUGIN(GameStartEndHandler)

void GameStartEndHandler::Init( const char* /*commandLine*/ )
{
  Register(bz_eGameStartEvent);
  Register(bz_eGameEndEvent);

  started = false;
}

void GameStartEndHandler::Event( bz_EventData *eventData )
{
  switch (eventData->eventType)
  {
    case bz_eGameStartEvent:
    {
      started = bz_startRecBuf();

      bz_Time time;

      bz_getLocaltime(&time);

      char temp[512];
      sprintf(temp,"match-%d%02d%02d-%02d%02d%02d.rec",
	time.year,time.month,time.day,
	time.hour,time.minute,time.second);

      filename = temp;
    }
    break;

    case bz_eGameEndEvent:
    {
      if (!started)
	break;

      bz_saveRecBuf(filename.c_str(),0);
      bz_stopRecBuf();

      started = false;
      bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS,
	"Match saved in file %s", filename.c_str());
    }
    break;

    default:
    {
      // do nothing
    }
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
