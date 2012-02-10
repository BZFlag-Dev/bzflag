// playHistoryTracker.cpp : Defines the entry point for the DLL application.
//


#include "bzfsAPI.h"
#include <map>

// event handler callback

class PlayHistoryTracker : public bz_Plugin
{
public:
	virtual const char* Name (){return "Play History Tracker";}
	virtual void Init ( const char* /* config */)
	{
		Register(bz_ePlayerDieEvent);
		Register(bz_ePlayerPartEvent);
		Register(bz_ePlayerSpawnEvent);
		Register(bz_ePlayerJoinEvent);
	}

  virtual void Event ( bz_EventData *eventData );
protected:

  typedef struct
  {
    int playerID;
    std::string callsign;
    double	startTime;
    double	lastUpdateTime;
    int		spreeTotal;
  } trPlayerHistoryRecord;

  std::map<int, trPlayerHistoryRecord > playerList;
};

BZ_PLUGIN(PlayHistoryTracker)


// ----------------- SpreeTracker-----------------

/*typedef struct
{
  int playerID;
  std::string	callsign;
  double	startTime;
  double	lastUpdateTime;
  int		spreeTotal;
} trPlayerHistoryRecord;

std::map<int, trPlayerHistoryRecord > playerList; */

void PlayHistoryTracker::Event( bz_EventData *eventData )
{
  switch (eventData->eventType)
  {
  default:
    // really WTF!!!!
    break;

  case bz_ePlayerDieEvent:
    {
      bz_PlayerDieEventData_V1	*deathRecord = ( bz_PlayerDieEventData_V1*)eventData;

      std::string killerCallSign = "UNKNOWN";

      bz_BasePlayerRecord	*killerData;

      killerData = bz_getPlayerByIndex(deathRecord->killerID);

      if (killerData)
	killerCallSign = killerData->callsign.c_str();

      std::string soundToPlay;

      // clear out the dude who got shot, since he won't be having any SPREEs
      if (playerList.find(deathRecord->playerID) != playerList.end())
      {
	trPlayerHistoryRecord	&record = playerList.find(deathRecord->playerID)->second;
	std::string message;
	if ( record.spreeTotal >= 5 && record.spreeTotal < 10 )
	  message = record.callsign + std::string("'s rampage was stopped by ") + killerCallSign;
	if ( record.spreeTotal >= 10 && record.spreeTotal < 20 )
	  message = record.callsign + std::string("'s killing spree was halted by ") + killerCallSign;
	if ( record.spreeTotal >= 20 )
	  message = std::string("The unstoppable reign of ") + record.callsign + std::string(" was ended by ") + killerCallSign;

	if (message.size())
	{
	  bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, message.c_str());
	  soundToPlay = "spree4";
	}

	record.spreeTotal = 0;
	record.startTime = deathRecord->eventTime;
	record.lastUpdateTime = deathRecord->eventTime;

      }

      // chock up another win for our killer
      // if they weren't the same as the killer ( suicide ).
      if ( (deathRecord->playerID != deathRecord->killerID) && playerList.find(deathRecord->killerID) != playerList.end())
      {
	trPlayerHistoryRecord	&record = playerList.find(deathRecord->killerID)->second;
	record.spreeTotal++;
	record.lastUpdateTime = deathRecord->eventTime;

	std::string message;

	if ( record.spreeTotal == 5 )
	{
	  message = record.callsign + std::string(" is on a Rampage!");
	  if (!soundToPlay.size())
	    soundToPlay = "spree1";
	}
	if ( record.spreeTotal == 10 )
	{
	  message = record.callsign + std::string(" is on a Killing Spree!");
	  if (!soundToPlay.size())
	    soundToPlay = "spree2";
	}
	if ( record.spreeTotal == 20 )
	{
	  message = record.callsign + std::string(" is Unstoppable!!");
	  if (!soundToPlay.size())
	    soundToPlay = "spree3";
	}
	if ( record.spreeTotal > 20 && record.spreeTotal%5 == 0 )
	{
	  message = record.callsign + std::string(" continues to rage on");
	  if (!soundToPlay.size())
	    soundToPlay = "spree4";
	}

	if (message.size())
	  bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, message.c_str());

      }

      bz_freePlayerRecord(killerData);

      //if (soundToPlay.size())
      //	bz_sendPlayCustomLocalSound(BZ_ALLUSERS,soundToPlay.c_str());

    }
    break;

  case  bz_ePlayerSpawnEvent:
    // really WTF!!!!
    break;

  case  bz_ePlayerJoinEvent:
    {
      trPlayerHistoryRecord playerRecord;

      playerRecord.playerID = (( bz_PlayerJoinPartEventData_V1*)eventData)->playerID;
      playerRecord.callsign = (( bz_PlayerJoinPartEventData_V1*)eventData)->record->callsign.c_str();
      playerRecord.spreeTotal = 0;
      playerRecord.lastUpdateTime = (( bz_PlayerJoinPartEventData_V1*)eventData)->eventTime;
      playerRecord.startTime = playerRecord.lastUpdateTime;

      playerList[(( bz_PlayerJoinPartEventData_V1*)eventData)->playerID] = playerRecord;
    }
    break;

  case  bz_ePlayerPartEvent:
    {
      std::map<int, trPlayerHistoryRecord >::iterator	itr = playerList.find( (( bz_PlayerJoinPartEventData_V1*)eventData)->playerID );
      if (itr != playerList.end())
	playerList.erase(itr);
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

