BZFlag Server Plugin: teamflagreset
================================================================================

This plugin monitors team flag drops and will automatically reset an idle team
flag after a specified amount of time.  It will also send a message stating that
the flag sat idle for too long.  It will not affect teams without players
(to prevent erronous messages) and will disable itself when there is only one
team with players.


Loading the plugin
--------------------------------------------------------------------------------

To load the plugin with a default timer of 5 minutes, use:
  -loadplugin teamflagreset

To load the plugin with a different timer value (in minutes from 1 to 120), use
the format:
  -loadplugin teamflagreset,<minutes>
For example:
  -loadplugin teamflagreset,1


Server Commands
--------------------------------------------------------------------------------

The plugin exposes a number of commands to control the timed reset.

/tfrtime <minutes>
  Set the team flag idle timer to <minute> minutes (1-120).
Example: /tfrtime 3

/tfroff
  Disable the timer.

/tfron
  Enable the timer.

/tfrstatus
  Display the status of the timer (enabled or disabled and flag idle timer).
