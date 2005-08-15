// rogueGenocide.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <string>
#include <map>
#include "global.h"

BZ_GET_PLUGIN_VERSION

// event handler callback
class RogueDeathHandler : public bz_EventHandler
{
public:
        RogueDeathHandler();
        virtual ~RogueDeathHandler();

        virtual void process ( bz_EventData *eventData );

        virtual bool autoDelete ( void ) { return false;} // this will be used for more then one event
protected:

        typedef struct
        {
                int playerID;
                bz_eTeamType team;
                std::string callsign;
                double startTime;
                double lastUpdateTime;

        }trRogueDeathHandler;

        std::map<int, trRogueDeathHandler > rogueList;
};

RogueDeathHandler	deathHandler;

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
	bz_debugMessage(4,"rogueGenocide plugin loaded");

	bz_registerEvent(bz_ePlayerDieEvent,&deathHandler);
	bz_registerEvent(bz_ePlayerPartEvent,&deathHandler);
	bz_registerEvent(bz_ePlayerJoinEvent,&deathHandler);


	return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
	bz_removeEvent(bz_ePlayerDieEvent,&deathHandler);
	bz_removeEvent(bz_ePlayerPartEvent,&deathHandler);
	bz_removeEvent(bz_ePlayerJoinEvent,&deathHandler);

	bz_debugMessage(4,"rogueGenocide plugin unloaded");
	return 0;
}


RogueDeathHandler::RogueDeathHandler()
{
}

RogueDeathHandler::~RogueDeathHandler()
{
}

void RogueDeathHandler::process ( bz_EventData *eventData )
{

			
	switch (eventData->eventType) 
	{
		default:
			// no clue
			break;
			
		// wait for a tank death and start checking for genocide and rogues
		case bz_ePlayerDieEvent: 
		{
			bz_PlayerDieEventData	*dieData = (bz_PlayerDieEventData*)eventData;
			//if its not a genocide kill, dont care
			if (dieData->flagKilledWith != "G" )
				break;
			// if the tank killed was not a rogue, let the server/client do the normal killing
			if (dieData->team != eRogueTeam )
				break;
			// if the tank killed was a rogue, and the killer was a rogue, kill the lousy tk'er
			if (dieData->killerTeam == eRogueTeam ) 
			{
				bz_killPlayer ( dieData->killerID, 0, dieData->killerID, "G" );
				bz_sendTextMessage(BZ_SERVER,dieData->killerID,"You should be more careful with Genocide");
			// if the tank killed was a rogue, and the killer wasnt, kill all rogues.
			// note the possible issue if the rogue tank being killed has not spawned
			// and if all else fails, just kill all rogues.....nothing wrong with that
			} else {
				for ( std::map<int, trRogueDeathHandler >::iterator itr = rogueList.begin(); itr != rogueList.end(); itr++ )
				{ 
					bz_killPlayer( itr->first, 0, dieData->killerID, "G" );
					// bz_sendTextMessage(BZ_SERVER,itr->first,"You were a victim of Rogue Genocide");
				}
			}
		}
		break;
		
		// remove from the rogueLIst if the rogue tank leaves
		case  bz_ePlayerPartEvent:
		{
			std::map<int, trRogueDeathHandler >::iterator itr = rogueList.find( (( bz_PlayerJoinPartEventData*)eventData)->playerID );
			if (itr != rogueList.end())
				rogueList.erase(itr);
        }                
		break;
		
    	// add to the list of rogue tanks playing				
		case bz_ePlayerJoinEvent: 
		{
			 trRogueDeathHandler   rogueRecord;

			rogueRecord.playerID = (( bz_PlayerJoinPartEventData*)eventData)->playerID;
			rogueRecord.team = (( bz_PlayerJoinPartEventData*)eventData)->team;
			rogueRecord.callsign = (( bz_PlayerJoinPartEventData*)eventData)->callsign.c_str();
			rogueRecord.lastUpdateTime = (( bz_PlayerJoinPartEventData*)eventData)->time;
			rogueRecord.startTime = rogueRecord.lastUpdateTime;

			if (rogueRecord.team == eRogueTeam )
			{
				rogueList[(( bz_PlayerJoinPartEventData*)eventData)->playerID] = rogueRecord;
			}
		}
		break;
		
	}
}
