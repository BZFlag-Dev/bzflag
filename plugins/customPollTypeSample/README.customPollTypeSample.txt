BZFlag Server Plugin: customPollTypeSample
================================================================================

This sample plugin showing how to make use of the bz_CustomPollTypeHandler to
introduce custom poll options.

This plugin creates a very simple '/poll mute <callsign>' option in order to mute
a player.


Loading the plugin
--------------------------------------------------------------------------------

This plugin takes no optional arguments, so load it with:

  -loadplugin customPollTypeSample


bz_CustomPollTypeHandler Functions
--------------------------------------------------------------------------------

There are two virtual functions that will need to be implemented by your plugin
to support custom poll options.

  - bool PollOpen (bz_BasePlayerRecord *player, const char* action, const char* parameters)

    This function is called before every custom poll begins. When this function
    returns true, the server will proceed and begin the poll. If the plugin
    returns false, then the poll will not begin. This function is intended for
    allowing plug-in developers to check conditions or permissions before the
    poll is begun. Any messages or errors that occur should be sent to the
    player from this function, otherwise it will appear as if the poll failed
    to start.

    * player - the player record of the player who would like to initiate a poll
    * action - the action is that being polled, e.g. 'mute'
    * value  - the value or target of the action, e.g. 'talkative_player'

  - void PollClose (const char* action, const char* parameters, bool success)

    This function is called after every custom poll has ended. This is where
    the business logic of a successful poll should be defined.

    * action  - the action is that being polled, e.g. 'mute'
    * value   - the value or target of the action, e.g. 'talkative_player'
    * success - whether or not the poll succeeded
