========================================================================
    DYNAMIC LINK LIBRARY : King of the Hill Project Overview
========================================================================

koth plugin version 2.3

Author: LouMan
Description: See below. 
Min Version: Latest CVS 2.0 branch code (to compile), bzfs 2.0.8 
Files: Attached in .zip file: 
Makefile.am 
README.txt 
koth.cpp 
koth.def 
koth.dll (for Windows versions) 
koth.sln 
koth.vcproj 
koth.hlp (for server help files)
kothadmin.hlp (for server help files)

Credit to JeffM2501 for base zone and custom map object code (from flagStay plugin), which I have modified to fit this application.

The plugin will create a King of the Hill game mode in which a team (or player) can occupy a defined volume ("Hill") for a defined amount of time and kill all other teams (or players) if occupied long enough.

There are 2 modes of play available:

1) Teamplay: if a player from a team occupies the Hill long enough, all other teams (except his/hers) are killed.  Score credits are given to the team, but not given to player that was King of the Hill.  If a rogue manages to become King of the Hill in this mode, all players (except him/her) are killed and he/she gets individual score credit for all of the kills.

2) No teamplay: for this mode it is recommended to set up a map with only rogue players allowed.  If a player occupies the Hill long enough, all other players are killed and score credit is given to the player who became King of the Hill for all of the kills.  Obviously, if teams are allowed in this mode, it would result in repeated team kills - no fun.  Basically this is a free-for-all mode for King of the Hill.

Plugin features/notes:

* Server messaging for Hill unoccupied (can be taken), Hill occupied (by whom and how long they have to hold it), regular countdown warnings while someone is holding Hill (every minute and 30, 20 & 10 seconds until Hill is "captured"), and successful King of the Hill announcements.  Long callsigns are truncated in server messages.

* In teamplay mode, if any team player manages to lay claim to the Hill and kill all other team players, his/her entire team must exit the Hill volume and at least one would need to re-enter to try and hold the Hill again.  If the player that captures the hill is a rogue in teamplay mode, only that player will need to exit and re-enter to try and take the Hill again.

* In no teamplay mode, if a player manages to lay claim to the Hill and kill all other players, only he/she must exit the Hill volume and re-enter to try and hold the Hill again.

* Players cannot pause on the Hill (everywhere else is ok) - if they try they will be killed with a warning message not to do so.

* King of the Hill will be disabled until there are 2 or more teams present.

* Explicit spawn zones are recommended in map files to prevent player spawns on the Hill.

* There is an autotime function (optional) that will reduce the time to hold the hill, based on the number of players in the game.  With this option enabled, the time is reduced a certain percentage (default is 3%) with every additional player (after 2) to a certain minimum percentage (defaut is 50%) of the original specified time to hold.  This means that (with default settings) the time to hold would not be affected for 2 players, but would be reduced by 3% for 3 players, 6% for 4 players and so on.  The minimum 50% of the original time to hold would be reached at about 19 players and would remain at 50%, no matter how many additional players were to join after that.

* There is a "/kingsay <message>" command available to all players to send a message to person holding the hill.

* When Hill is initially occupied and when hill is taken, standard team audio alerts take place.

-------------------------------------------------------------------------------------

The parameters of the King of the Hill (koth) map definition are follows:


bbox <Xmin> <Xmax> <Ymin> <Ymax> <Zmin> <Zmax>
or
cylinder <X> <Y> <Zmin> <Zmax> <radius>

Required.  These define the volume on the map to be occupied for the King of the Hill.

teamplay

Optional.  This will enable the teamplay mode of King of the Hill (see above).  If not included, teamplay is disabled and it becomes as basic free for all King of the Hill mode.


holdtime <seconds>

Optional.  This defines the time (in seconds) required to occupy the King of the Hill volume before a player is declared King of the Hill and other teams/players are zapped.  If not included, the default setting is 60 seconds (1 minute).  The limits of this setting are controlled to a minimum of 1 second and a maximum of 7200 seconds.

autotime <multiplier> <minimum multiplier>

Optional.  This will enable the automatic time reduction feature of the plugin that will reduce the time to hold the hill, based on the number of players in the game (see description above).  The multiplier field will set the hold time reduction percentage per new player.  The minimum multiplier field will set the minimum hold time reduction percentage.  If these fields are left blank, the default multiplier = 3% and the default minimum multiplier = 50%.  The multiplier fields should be between 1 and 99 (percent).


nosound

Optional.  This will disable sounds associated with Keep Away plugin (some clients seem to have issues with this feature).

-------------------------------------------------------------------------------------

Examples of map (.bzw) entries:


koth
  bbox -30 30 -30 30 10 40
  teamplay
  holdtime 60
end

This would set up a King of the Hill volume 60 x 60 x 30, centered at 0 0 25.  Teamplay would be enabled and the required time to occupy the "Hill" would be 60 seconds (1 minute).  Time to hold would not automatically adjust downward with new players.


koth
  cylinder 0 0 20 40 15
  holdtime 100
  teamplay
  autotime 5 25
  nosound
end

This would set up a King of the Hill cylindrical volume 30 x 30 x 20, centered at 0 0 30.  Teamplay would be enabled and the required time to occupy the "Hill" would be 100 seconds.  The time to occupy the hill would adjust downward with an increasing number of players (see autotime description above).  Each new player after 2 will reduce the hold time by 5% to a minimum of 25% of the hold time (25 seconds).  The sounds associated with the plugin would be disabled.
  
Obviously, the plugin must be loaded to the server at startup, or it will not recoginize these map parameters.

-------------------------------------------------------------------------------------

There is one command available to all players with the plugin:

"kingsay <message>" - this will send message to the player occupying the hill.

There are several administrator commands available with the plugin:

"kothon" - this will turn King of the Hill mode on, if turned off previously.
"kothoff" - this will turn King of the Hill mode off for normal gameplay.
"kothtimemult" - this will set auto time multiplier (per new player).
"kothtimemultmin" - this will set minimum auto time multiplier.
"kothautotimeon" - this will enable the autotime function.
"kothsoundon" - this will enable the plugin's sounds.
"kothsoundoff" - this will disable the plugin's sounds.
"kothautotimeoff" - this will disable the autotime funtion.
"kothtime <seconds>" - this will change King of the Hill hold time 1 -> 7200 seconds.
"kothstatus" - this will return the status of the plugin modes, times, etc.

-------------------------------------------------------------------------------------

Please report any bugs or issues with this plugin to LouMan, and good luck.