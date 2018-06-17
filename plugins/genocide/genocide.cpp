// genocide.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

// event handler callback
class genocide : public bz_Plugin
{
public:
  virtual ~genocide() {};
  virtual const char* Name () {return "Genocide";}
  virtual void Init ( const char* c );
  virtual void Cleanup ( void );
  virtual void Event ( bz_EventData *eventData );
};

BZ_PLUGIN(genocide)

void genocide::Init ( const char* /*commandLine*/ )
{
  bz_debugMessage(4, "genocide plugin loaded");

  // register our special custom flag
  bz_RegisterCustomFlag("G", "Genocide", "Killing one tank kills that tank's whole team.", /*eSuperShot*/0, eGoodFlag);

  // register events for pick up, drop, transfer, and fire
  Register(bz_eFlagTransferredEvent);
  Register(bz_eFlagGrabbedEvent);
  Register(bz_eFlagDroppedEvent);
  Register(bz_eShotFiredEvent);
  Register(bz_ePlayerDieEvent);
}

void genocide::Cleanup ( void )
{
  // unregister our events
  Flush();

  bz_debugMessage(4, "genocide plugin unloaded");
}

void genocide::Event(bz_EventData *eventData)
{
  switch (eventData->eventType)
  {
      default:
      {
          // no, sir, we didn't ask for THIS!!
          bz_debugMessage(1, "genocide: received event with unrequested eventType!");
          return;
      }

      case bz_eFlagTransferredEvent:
      {
        /*  bz_FlagTransferredEventData_V1* fte = (bz_FlagTransferredEventData_V1*)eventData;*/
          break;
      }

      case bz_eFlagGrabbedEvent:
      {
       /*   bz_FlagGrabbedEventData_V1* fge = (bz_FlagGrabbedEventData_V1*)eventData;*/
          break;
      }

      case bz_eFlagDroppedEvent:
      {
         /* bz_FlagDroppedEventData_V1* fde = (bz_FlagDroppedEventData_V1*)eventData;*/
          break;
      }

      case bz_eShotFiredEvent:
      {
          bz_ShotFiredEventData_V1* sfed = (bz_ShotFiredEventData_V1*)eventData;

          bz_BasePlayerRecord *playerRecord = bz_getPlayerByIndex(sfed->playerID);
          if (!playerRecord)
              break;

          bz_freePlayerRecord(playerRecord);
          break;
      }

      case bz_ePlayerDieEvent:
      {
          bz_PlayerDieEventData_V1* deed = (bz_PlayerDieEventData_V1*)eventData;
          bz_ApiString& flag = deed->flagKilledWith;

          if (flag == "G")
          {
              // if the tank killed was a rogue, kill all rogues.
              bz_APIIntList *playerList = bz_newIntList();

              bz_getPlayerIndexList(playerList);

              for (unsigned int i = 0; i < playerList->size(); i++)
              {
                  int targetID = (*playerList)[i];
                  bz_BasePlayerRecord *playRec = bz_getPlayerByIndex(targetID);
                  if (!playRec)
                      continue;

                  if (deed->team == playRec->team && deed->team != eRogueTeam && deed->team != eObservers)
                      bz_killPlayer(targetID, false, eGenocideEffect, deed->killerID, "G");

                  bz_freePlayerRecord(playRec);
              }
              bz_deleteIntList(playerList);
          }
          break;
      }
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
