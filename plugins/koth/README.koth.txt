BZFlag Server Plugin: koth
================================================================================

Original author: LouMan

The plugin will create a King of the Hill game mode in which a team (or player)
can occupy a defined volume ("Hill") for a defined amount of time and kill all
other teams (or players) if occupied long enough.

There are 2 modes of play available:
1. Teamplay: if a player from a team occupies the Hill long enough, all other
  teams (except his/hers) are killed.  Score credits are given to the team, but
  not given to player that was King of the Hill.  If a rogue manages to become
  King of the Hill in this mode, all players (except him/her) are killed and
  he/she gets individual score credit for all of the kills.
2. No teamplay: for this mode it is recommended to set up a map with only rogue
  players allowed.  If a player occupies the Hill long enough, all other players
  are killed and score credit is given to the player who became King of the Hill
  for all of the kills.  Obviously, if teams are allowed in this mode, it would
  result in repeated team kills - no fun.  Basically this is a free-for-all mode
  for King of the Hill.

Plugin features/notes:
* Server messaging for Hill unoccupied (can be taken), Hill occupied (by whom
  and how long they have to hold it), regular countdown warnings while someone
  is holding Hill (every minute and 30, 20 & 10 seconds until Hill is
  "captured"), and successful King of the Hill announcements.  Long callsigns
  are truncated in server messages.
* In teamplay mode, if any team player manages to lay claim to the Hill and kill
  all other team players, his/her entire team must exit the Hill volume and at
  least one would need to re-enter to try and hold the Hill again.  If the
  player that captures the hill is a rogue in teamplay mode, only that player
  will need to exit and re-enter to try and take the Hill again.
* In no teamplay mode, if a player manages to lay claim to the Hill and kill all
  other players, only he/she must exit the Hill volume and re-enter to try and
  hold the Hill again.
* Players cannot pause on the Hill (everywhere else is ok) - if they try they
  will be killed with a warning message not to do so.
* King of the Hill will be disabled until there are 2 or more teams present.
* Explicit spawn zones are recommended in map files to prevent player spawns on
  the Hill.
* There is an autotime function (optional) that will reduce the time to hold the
  hill, based on the number of players in the game.  With this option enabled,
  the time is reduced a certain percentage (default is 3%) with every additional
  player (after 2) to a certain minimum percentage (default is 50%) of the
  original specified time to hold.  This means that (with default settings) the
  time to hold would not be affected for 2 players, but would be reduced by 3%
  for 3 players, 6% for 4 players and so on.  The minimum 50% of the original
  time to hold would be reached at about 19 players and would remain at 50%, no
  matter how many additional players were to join after that.
* There is a "/kingsay <message>" command available to all players to send a
  message to person holding the hill.


Loading the plugin
--------------------------------------------------------------------------------

This plugin takes no optional arguments, so load it with:

  -loadplugin koth


Server Commands
--------------------------------------------------------------------------------

There is one command available to all players with the plugin:

/kingsay <message>
  Send message to the player occupying the hill.

There are several administrator commands available with the plugin:

/kothon
  Turn King of the Hill mode on, if turned off previously.
/kothoff
  Turn King of the Hill mode off for normal gameplay.
/kothtimemult
  Set auto time multiplier (per new player).
/kothtimemultmin
  Set minimum auto time multiplier.
/kothautotimeon
  Enable the autotime function.
/kothautotimeoff
  Disable the autotime function.
/kothtime <seconds>
  Change King of the Hill hold time 1 -> 7200 seconds.
/kothstatus
  Return the status of the plugin modes, times, etc.


Map Objects
--------------------------------------------------------------------------------

The plugin adds a 'koth' object type.  You will specify this when creating a
custom zone.  Both zone shapes will use a 'pos' (position) and various
attributes for controlling the mode.  The rectangular type will use a 'size' and
'rot' (rotation) attribute, and the circular type will use a 'radius' and
'height' attribute.

Here are the common attributes and how to use them:

This optional attribute will enable the teamplay mode of King of the Hill
(see above).  If not included, teamplay is disabled and it becomes as basic free
for all King of the Hill mode.
  teamplay

This optional attribute defines the time (in seconds) required to occupy the
King of the Hill volume before a player is declared King of the Hill and other
teams/players are zapped.  If not included, the default setting is 60 seconds
(1 minute).  The limits of this setting are controlled to a minimum of 1 second
and a maximum of 7200 seconds.
  holdtime <seconds>

This optional attribute will enable the automatic time reduction feature of the
plugin that will reduce the time to hold the hill, based on the number of
players in the game (see description above).  The multiplier field will set the
hold time reduction percentage per new player.  The minimum multiplier field
will set the minimum hold time reduction percentage.  If these fields are left
blank, the default multiplier = 3% and the default minimum multiplier = 50%.
The multiplier fields should be between 1 and 99 (percent).
  autotime <multiplier> <minimum multiplier>


# This would set up a King of the Hill volume 60 x 60 x 30, centered at 0 0 25.
# Teamplay would be enabled and the required time to occupy the "Hill" would be
# 60 seconds (1 minute).  Time to hold would not automatically adjust downward
# with new players.
koth
  pos 0 0 25
  size 60 60 30
  teamplay
  holdtime 60
end


# This would set up a King of the Hill cylindrical volume with a radius of 15
# and a height of 20, centered at 0 0 30.  Teamplay would be enabled and the
# required time to occupy the "Hill" would be 100 seconds.  The time to occupy
# the hill would adjust downward with an increasing number of players (see
# autotime description above).  Each new player after 2 will reduce the hold
# time by 5% to a minimum of 25% of the hold time (25 seconds).
koth
  pos 0 0 30
  radius 15
  height 20
  holdtime 100
  teamplay
  autotime 5 25
end
