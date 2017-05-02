BZFlag Server Plugin: TimeLimit
================================================================================

This plugin allows players to adjust the time limit on a server, which is mostly
useful for match servers.  Before starting a match with the /countdown command,
you can set the match duration with the /timelimit command.  It's not possible
to set the match duration when a countdown is in progress or when a match is
already started.

The timelimit will reset itself to the -time value at the end of a match or when
there are no players on the server.  It is also required that bzfs is started
with the -timemanual option.


Loading the plugin
--------------------------------------------------------------------------------

To load the plugin with default settings (which allows all times), use:
  -loadplugin TimeLimit

The time limits can be configured two ways.  You can either specify a comma
separated list of at most 20 times, or specify a single range of times.

To load the plugin with a list of times, use the format:
  -loadplugin TimeLimit,<time1>,<time2>,...,<timeX>
For example:
  -loadplugin TimeLimit,15,20,30

To load the plugin with a range of times, use the format:
  -loadplugin TimeLimit,<low>-<high>
For example:
  -loadplugin TimeLimit,15-30


Server Commands
--------------------------------------------------------------------------------

The plugin adds a /timelimit command, and requires the TIMELIMIT permission.

/timelimit
  Displays the usage message

/timelimit <minutes>
  Sets the timelimit to <minutes> minutes
Example:
  /timelimit 15

/timelimit show
  Shows the current set timelimit

/timelimit reset
  Resets the timelimit back to default (-time setting)
