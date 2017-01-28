// customPollTypeSample.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"

class customPollTypeSample : public bz_Plugin, public bz_CustomPollTypeHandler
{
public:
  virtual const char* Name () {return "Custom Poll Type";}
  virtual void Init ( const char* config );
  virtual void Cleanup ( void );
  virtual void Event ( bz_EventData * /* eventData */ ) {return;}

  virtual bool PollOpen  (bz_BasePlayerRecord *player, const char* action, const char* parameters);
  virtual void PollClose (const char* action, const char* parameters, bool success);
};

BZ_PLUGIN(customPollTypeSample)

void customPollTypeSample::Init ( const char* /*commandLine*/ )
{
  // This will introduce a '/poll mute callsign' option
  bz_registerCustomPollType("mute", "callsign", this);
}

void customPollTypeSample::Cleanup ()
{
  Flush();

  // Remove the poll option when this plugin is loaded or else what other plugin would handle it?
  bz_removeCustomPollType("mute");
}

// This function is called before a `/poll mute <callsign>` poll is started. If this function returns false, then the poll will not
// start. This is useful for checking permissions or other conditions.
bool customPollTypeSample::PollOpen(bz_BasePlayerRecord *player, const char* action, const char* /*parameters*/)
{
  int playerID = player->playerID;
  std::string _action = action;

  // If a player doesn't have the 'poll' permission, they will not be able to start a poll. Be sure to send the playerID a message
  // or else it'll appear as if the /poll command did not work.
  if (!bz_hasPerm(playerID, "pollMute")) {
    bz_sendTextMessage(BZ_SERVER, playerID, "You can't start a poll!");
    return false;
  }

  // The 'action' variable will be set whichever poll option is being called
  if (_action == "mute") {

    // Return true in order to let BZFS start the poll
    return true;
  }

  // This should never be reached but it'll take care of compiler warnings
  return false;
}

void customPollTypeSample::PollClose(const char* action, const char* parameters, bool success)
{
  std::string _action = action;
  std::string _parameters = parameters;

  if (_action == "mute" && success) {
    bz_BasePlayerRecord *pr = bz_getPlayerBySlotOrCallsign(_parameters.c_str());

    if (!pr) {
      bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "player %s not found", _parameters.c_str());
      return;
    }

    // Poll succeeded, so mute the player
    bz_revokePerm(pr->playerID, "talk");

    bz_freePlayerRecord(pr);
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
