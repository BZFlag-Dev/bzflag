// mapchange.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include <math.h>

class Game
{
public:
	bool hasBeenPlayed;

	std::string mapFile;
	std::string publicText;
	Game()
	{
		hasBeenPlayed = false;
	}
};

typedef enum
{
	eTimedGame,
	eMaxKillScore,
	eMaxCapScore,
	eNoPlayers
}EndCond;
EndCond				endCond;
double				timeLimit;
int					scoreCapLimit;
double				startTime;
int					currentIndex;

std::vector<Game>	gameList;
bool				loop;

class MapChangeEventHandler : public bz_EventHandler
{
public:
	virtual void process ( bz_EventData *eventData );
};

MapChangeEventHandler handler;

EndCond condfromString ( std::string &str )
{
	if ( str == "timed")
		return eTimedGame;
	else if ( str == "maxkill")
		return eMaxKillScore;
	else if ( str == "maxcap")
		return eMaxCapScore;
	else if ( str == "empty")
		return eNoPlayers;

	return eTimedGame;
}

bool loadGamesFromFile ( const char* config )
{
	FILE *fp = fopen(config,"rb");
	if(!fp)
		return false;
	fseek(fp,SEEK_END,0);

	std::string text;
	unsigned int size = ftell(fp);
	fseek(fp,SEEK_SET,0);

	char *temp = (char*)malloc(size+1);
	fread(temp,size,1,fp);
	fclose(fp);
	temp[size] = 0;
	text = temp;
	free(temp);

	endCond = eTimedGame;
	timeLimit = 30.0*60.0;
	scoreCapLimit = 10;

	startTime = -1;
	currentIndex = -1;
	loop = true;

	std::vector<std::string> lines = tokenize(text,std::string("\r\n"),0,false);

	for ( unsigned int i = 0; i < (unsigned int)lines.size(); i++ )
	{
		std::string line = lines[i];

		std::vector<std::string> params = tokenize(line,std::string(","),0,true);

		if (params.size())
		{
			if (tolower(params[0]) == "mode")
			{
				if ( params.size() > 1 )
					endCond = condfromString(tolower(params[1]));
				if ( params.size() > 2 )
				{
					if (endCond == eTimedGame )
						timeLimit = fabs(atof(params[2].c_str()));
					else
						scoreCapLimit = atoi(params[2].c_str());

					if (timeLimit == 0.0)
						timeLimit = 30;

					if ( scoreCapLimit <= 0 )
						scoreCapLimit = 10;

				}

				if ( params.size() > 3)
					loop = tolower(params[3]) == "loop";
			}
			else
			{
				Game	game;
				game.mapFile = params[0];
				if ( params.size() > 1 )
					game.publicText = params[1];

				gameList.push_back(game);
			}
		}
	}
	return gameList.size();
}

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
	if(!commandLine || !strlen(commandLine))
	{
		bz_debugMessage(0,"mapchange plugin requires a config file as a param and will not load");
		return -1;
	}

	if(!loadGamesFromFile(commandLine))
	{
		bz_debugMessage(0,"mapchange plugin config file failure, aborting load");
		return -1;
	}

	bz_debugMessage(4,"mapchange plugin loaded");

	bz_APIIntList *players = bz_getPlayerIndexList();
	if ( players->size() )
		startTime = bz_getCurrentTime();

	bz_registerEvent ( bz_ePlayerJoinEvent, &handler );
	bz_registerEvent ( bz_ePlayerDieEvent, &handler );
	bz_registerEvent ( bz_ePlayerPartEvent, &handler );
	bz_registerEvent ( bz_eCaptureEvent, &handler );
	bz_registerEvent ( bz_eGetWorldEvent, &handler );

  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_removeEvent ( bz_ePlayerJoinEvent, &handler );
  bz_removeEvent ( bz_ePlayerDieEvent, &handler );
  bz_removeEvent ( bz_ePlayerPartEvent, &handler );
  bz_removeEvent ( bz_eCaptureEvent, &handler );
  bz_removeEvent ( bz_eGetWorldEvent, &handler );

  bz_debugMessage(4,"mapchange plugin unloaded");
  return 0;
}

void MapChangeEventHandler::process ( bz_EventData *eventData )
{
	if (!eventData)
		return;

}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
