BZFlag Server Plugin: keepaway
================================================================================

Original author: LouMan

The plugin will create a Keep Away game mode in which a player/team must hold a
"Keep Away" flag (defined in map file) for a defined amount of time; if held
long enough all other teams (or players) are killed.

There are 2 modes of play available:
1. Teamplay: if a player from a team holds the Keep Away flag long enough, all
  other teams (except his/hers) are killed.  Score credits are given to the
  team, but not given to player that kept the flag.  If a rogue manages to hold
  the Keep Away flag long enough in this mode, all players (except him/her) are
  killed and he/she gets individual score credit for all of the kills.
2. No teamplay: for this mode it is recommended to set up a map with only rogue
  players allowed.  If a player holds the Keep Away flag long enough, all other
  players are killed and score credit is given to the player who held the Keep
  Away flag for all of the kills.  Obviously, if teams are allowed in this mode,
  it would result in repeated team kills - no fun.  Basically this is a
  free-for-all mode for Keep Away.

Plugin features/notes:
* Server messaging for Keep Away flag is free (which one to hold), Keep Away
  flag held (by whom and how long they have to hold it), regular countdown
  warnings while someone is holding the Keep Away Flag (every minute and 30, 20
  & 10 seconds until flag is held long enough), and successful Keep Away
  announcements.  Long callsigns are truncated in server messages.
* Players cannot pause while holding Keep Away flag - if they try the flag will
  be taken away with a warning message not to do so.
* Keep Away will be disabled until there are 2 or more teams, 2 or more rogue
  players, or at least 1 rogue player and 1 or more team player(s).
* It is highly recommended to add exactly 1 flag to the map file for each type
  of flag that might be considered the Keep Away flag.  Duplicate Keep Away
  flags have not been tested.
* There is an autotime function (optional) that will reduce the time to hold the
  Keep Away flag, based on the number of players in the game.  With this option
  enabled, the time is reduced a certain percentage (default is 3%) with every
  additional player (after 2) to a certain minimum percentage (default is 50%)
  of the original specified time to hold.  This means that (with default
  settings) the time to hold would not be affected for 2 players, but would be
  reduced by 3% for 3 players, 6% for 4 players and so on.  The minimum 50% of
  the original time to hold would be reached at about 19 players and would
  remain at 50%, no matter how many additional players were to join after that.
* There is a "/kas <message>" command available to all players to send a message
  to person holding the Keep Away flag.
* There is a "/kaf" command available to all players that will return the
  current Keep Away flag to find.


Loading the plugin
--------------------------------------------------------------------------------

This plugin takes no optional arguments, so load it with:

  -loadplugin keepaway


Server Commands
--------------------------------------------------------------------------------

There are two commands available to all players with the plugin:

/kas <message>
  Send message to the player holding the Keep Away flag.
/kaf
  Return the current Keep Away flag to be found & who holds it.

There are several administrator commands available with the plugin:

/kaon
  Turn Keep Away mode on, if turned off previously.
/kaoff
  Turn Keep Away mode off for normal gameplay.
/katimemult
  Set auto time multiplier (per new player).
/katimemultmin
  Set minimum auto time multiplier.
/kaautotimeon
  Enable the autotime function.
/kaautotimeoff
  Disable the autotime function.
/katime <seconds>
  Change Keep Away hold time 1 -> 7200 seconds.
/kastatus
  Return the status of the plugin modes, times, etc.
/kaffon
  Turn on forced flags.
/kaffoff
  Turn off forced flags.
/kaf+
  Advance to next Keep Away flag.
/kasoundon
  Enables Keep Away sounds.
/kasoundoff
  Disables Keep Away sounds.
/kaflagreseton
  Enables Keep Away sounds.
/kaflagresetoff
  Disables Keep Away sounds.


Map Objects
--------------------------------------------------------------------------------

The plugin adds a 'keepaway' that has a single require attribute and can also
contain several optional attributes:

This required attribute will define which flags the plugin will cycle through as
Keep Away flags.  At least one is required.  If no flags are defined, the plugin
will be effectively disabled.  Remember, the plugin has not been tested for
duplicate flags on the map, so it is highly recommended to explicitly define
only 1 flag per Keep Away flag type to be present on map.
  keepawayflags <flag type> <flag type> <flag type> <flag type> <flag type>...

This optional attribute will enable the teamplay mode of Keep Away (see above).
If not included, teamplay is disabled and it becomes as basic free for all Keep
Away mode.
  teamplay

This optional attribute defines the time (in seconds) required to hold the Keep
Away flag before a player is declared winner and other teams/players are zapped.
If not included, the default setting is 120 seconds (2 minutes).  The limits of
this setting are controlled to a minimum of 1 second and a maximum of 7200
seconds.
  holdtime <seconds>

This optional attribute will enable the automatic time reduction feature of the
plugin that will reduce the time to hold the Keep Away flag, based on the number
of players in the game (see description above).  The multiplier field will set
the hold time reduction percentage per new player.  The minimum multiplier field
will set the minimum hold time reduction percentage.  If these fields are left
blank, the default multiplier = 3% and the default minimum multiplier = 50%.
The multiplier fields should be between 1 and 99 (percent).
  autotime <multiplier> <minimum multiplier>

This optional attribute will force sequential Keep Away flag selection based on
flags defined in flag parameter (below).  If a player is holding the next flag
to be used as Keep Away flag, it is taken away from him/her with a message
apologizing for the take ("Sorry, server needs your flag for Keep Away :/").
If this parameter is not included, the plugin will take away the winner's flag
and search for the next available Keep Away flag that is not held.  If it cannot
find one, it will repeat the Keep Away flag that was just used.
  forcedflags

This optional attribute will disable automatic flag reset function of the Keep
Away plugin (enabled as default).
  noflagreset



# This would set up a Keep Away game using Super Bullet, Genocide, Thief,
# Machine Gun, Guided Missile and Laser flags for Keep Away flags.  Teamplay
# would be enabled and the initial required time to hold the Keep Away flag
# would be 90 seconds (1.5 minutes).  Time to hold would automatically adjust
# downward with new players; 2% per new player after 2, to a minimum of 40% of
# the original time to hold (36 seconds).  The automatic flag reset function of
# the plugin would be disabled.
keepaway
  holdtime 90
  autotime 2 40
  keepawayflags SB G TH MG GM L
  forcedflags
  teamplay
  noflagreset
end


# This would set up a Keep Away game using Invisible Bullet, Phantom Zone,
# Useless and Reverse Only flags for Keep Away flags.  Teamplay would be enabled
# and the initial required time to hold the Keep Away flag would be 60 seconds
# (1 minute).  Time to hold would not automatically adjust downward with new
# players - it would remain at 60 seconds.
keepaway
  holdtime 60
  keepawayflags IB PZ US RO
  teamplay
end
