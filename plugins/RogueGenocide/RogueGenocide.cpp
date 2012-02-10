// rogueGenocide.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <map>


// event handler callback
class RogueGenoHandler : public bz_Plugin
{
public:
	virtual const char* Name (){return "Rogue Genocide";}
	virtual void Init ( const char* config);

  virtual void Event ( bz_EventData *eventData );

  bool noSuicide;
};

BZ_PLUGIN(RogueGenoHandler)

void RogueGenoHandler::Init( const char* commandLine )
{
  Register(bz_ePlayerDieEvent);

  std::string param = commandLine;

  noSuicide = (param == "nosuicide");
}

void RogueGenoHandler::Event ( bz_EventData *eventData )
{
  switch (eventData->eventType) {
  default:
    // no clue
    break;

  // wait for a tank death and start checking for genocide and rogues
  case bz_ePlayerDieEvent:
    {
      bz_PlayerDieEventData_V1	*dieData = (bz_PlayerDieEventData_V1*)eventData;
      //if its not a genocide kill, dont care
      if (dieData->flagKilledWith != "G" )
	break;
      // if the tank killed was not a rogue, let the server/client do the normal killing
      if (dieData->team != eRogueTeam )
	break;
      // option to disallow rogues getting points for shooting themselves
      if ( noSuicide && dieData->killerID == dieData->playerID )
	break;

      // if the tank killed was a rogue, kill all rogues.
      bz_APIIntList	*playerList = bz_newIntList();

      bz_getPlayerIndexList(playerList);

      for ( unsigned int i = 0; i < playerList->size(); i++)
      {
	int targetID = (*playerList)[i];
	bz_BasePlayerRecord *playRec = bz_getPlayerByIndex ( targetID );
	if (!playRec) continue;

	// the sucker is a spawned rogue, kill him.  This generates another death event,
	// so if you kill another rogue with geno while you are a rogue you end up dead too.
	// and you get both messages (victim and be careful)
	if ( playRec->spawned && playRec->team == eRogueTeam )
	{
	  bz_killPlayer( targetID, false, dieData->killerID, "G" );
	  bz_sendTextMessage(BZ_SERVER, targetID, "You were a victim of Rogue Genocide");

	  // oops, I ended up killing myself (directly or indirectly) with Genocide!
	  if ( targetID == dieData->killerID )
	    bz_sendTextMessage(BZ_SERVER, targetID, "You should be more careful with Genocide!");
	}

	bz_freePlayerRecord(playRec);
      }
      bz_deleteIntList(playerList);
    }
    break;
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

