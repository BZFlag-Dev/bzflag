// recordmatch.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

class GameStartEndHandler : public bz_EventHandler
{
public:
	virtual void process ( bz_EventData *eventData );
};

GameStartEndHandler	gameStartEndHandler;

std::string path;
bool started = false;
std::string filename;

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
	bz_registerEvent(bz_eGameStartEvent,&gameStartEndHandler);
	bz_registerEvent(bz_eGameEndEvent,&gameStartEndHandler);
	bz_debugMessage(4,"recordmatch plugin loaded");

	filename = commandLine;
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
	bz_debugMessage(4,"recordmatch plugin unloaded");
	bz_removeEvent(bz_eGameStartEvent,&gameStartEndHandler);
	bz_removeEvent(bz_eGameEndEvent,&gameStartEndHandler);

  return 0;
}

void GameStartEndHandler::process( bz_EventData *eventData )
{
	switch(eventData->eventType)
	{
	case bz_eGameStartEvent:
		{
			started = bz_startRecBuf();

			bz_localTime time;

			bz_getLocaltime(&time);

			char temp[512];
			sprintf(temp,"%d%d%d-%d%d%d.rec",time.month,time.day,time.year,time.hour,time.minute,time.second);

			filename = temp;
		}
		break;

	case bz_eGameEndEvent:
		{
			if (!started)
				break;
			std::string recFile = path + filename;
			
			bz_saveRecBuf(recFile.c_str(),0);
			bz_stopRecBuf();
		}
		break;
	default:
		{
			// do nothing
		}
	}
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

