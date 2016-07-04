BZFlag Server Plugin: HoldTheFlag
================================================================================

The HoldTheFlag plugin is a score-tracker for the Hold The Flag (HTF) game mode.
In HTF, all players are on the same team, and their goal is to take their own
team flag to the opposing team base on the other side of the map.  When a player
captures their own flag, they earn a point.  Killing "teammates" to prevent them
from reaching the other base first is encouraged.

The HoldTheFlag plugin understands the normal (/countdown) timed match command.
Scores will automatically be displayed whenever a player captures a flag, and a
winner will be declared at the end of the match. An '*' after a player's score
indicates the player who made the last capture. This is used to break a tie.


Loading the plugin
--------------------------------------------------------------------------------

By default, the green team is used.  If that is acceptable, you can just load
the plugin without additional arguments

  -loadplugin HoldTheFlag

If you would like to use a different team, you can specify any of red, purple,
or blue (case doesn't matter):

  -loadplugin HoldTheFlag,team=Purple


Server Commands
--------------------------------------------------------------------------------

The HoldTheFlag plugin defines a single 'htf' command that also has several
subcommands.

To display the current HTF scores:
  /htf

If you have the HTFONOFF permission, to enable HTF mode (which is enabled by
default):
  /htf on

If you have the HTFONOFF permission, to disable HTF mode:
  /htf off

To reset the HTF scores (requires the COUNTDOWN permission):
  /htf reset
