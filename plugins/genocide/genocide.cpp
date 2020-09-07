// genocide.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

// event handler callback
class genocide : public bz_Plugin
{
public:
    virtual ~genocide() {};
    virtual const char* Name ()
    {
        return "Genocide";
    }
    virtual void Init ( const char* config );
    virtual void Cleanup ( void );
    virtual void Event ( bz_EventData *eventData );

    bool disableSuicide = false;
    bool rogueAsTeam = false;
};

BZ_PLUGIN(genocide)

void genocide::Init ( const char* cmdLine )
{
    bz_debugMessage(4, "genocide plugin loaded");

    if (cmdLine)
    {
        auto cmd = std::string(cmdLine);
        disableSuicide =  (cmd.find("disableSuicide") != cmd.npos);
        rogueAsTeam    =  (cmd.find("rogueAsTeam")    != cmd.npos);
    }
    
    if (rogueAsTeam)
        bz_debugMessage(4, "rogueAsTeam enabled");

    if (disableSuicide)
        bz_debugMessage(4, "disableSuicide enabled");

    // register our special custom flag
    bz_RegisterCustomFlag("G", "Genocide", "Killing one tank kills that tank's whole team.");

    // register our events
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

    case bz_ePlayerDieEvent:
    {
        bz_PlayerDieEventData_V1 *dieData = (bz_PlayerDieEventData_V1*)eventData;
        //if its not a genocide kill, dont care
        if (dieData->flagKilledWith != "G")
            break;
        // if the tank killed is a rogue and we don't have rogue as team enabled
        if (!rogueAsTeam && dieData->team == eRogueTeam)
            break;
        // option to disallow genocide from being trigger by selfkills
        if (disableSuicide && dieData->killerID == dieData->playerID)
            break;

        // if we pass options, proceed to kill tanks of specified team
        bz_APIIntList *playerList = bz_newIntList();

        bz_getPlayerIndexList(playerList);

        if (dieData->team == eRogueTeam)
        {
            for (unsigned int i = 0; i < playerList->size(); i++)
            {
                int targetID = (*playerList)[i];
                bz_BasePlayerRecord *playRec = bz_getPlayerByIndex (targetID);
                if (!playRec)
                    continue;

                // the sucker is a spawned rogue, kill him.  This generates another death event,
                // so if you kill another rogue with geno while you are a rogue you end up dead too.
                // and you get both messages (victim and be careful)
                if (playRec->spawned && playRec->team == eRogueTeam)
                {
                    bz_killPlayer(targetID, false, dieData->killerID, "G");
                    bz_sendTextMessage(BZ_SERVER, targetID, "You were a victim of Rogue Genocide");

                    // oops, I ended up killing myself (directly or indirectly) with Genocide!
                    if (targetID == dieData->killerID)
                        bz_sendTextMessage(BZ_SERVER, targetID, "You should be more careful with Genocide!");
                }

                bz_freePlayerRecord(playRec);
            }
        }
        else
        {
            for (unsigned int i = 0; i < playerList->size(); i++)
            {
                int targetID = (*playerList)[i];
                bz_BasePlayerRecord *playRec = bz_getPlayerByIndex(targetID);
                if (!playRec)
                    continue;

                if (dieData->team == playRec->team && dieData->team != eObservers)
                    bz_killPlayer(targetID, false, eGenocideEffect, dieData->killerID, "G");

                bz_freePlayerRecord(playRec);
            }
        }
        bz_deleteIntList(playerList);
    }
    break;
    }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
