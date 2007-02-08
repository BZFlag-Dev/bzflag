========================================================================
    DYNAMIC LINK LIBRARY : teamflagreset Project Overview
========================================================================

This is the teamflagreset plugin. It will look at the team flags and see if they have been held by a player and dropped - if so, it will start timing to a limit.  If the flag is not picked up again within that time limit the team flag will reset with a message saying which team flag sat idle too long.  It will also automatically disable itself for teams with no players and for only 1 team in the match (no opposing teams).


There are 5 administrator commands available with the plugin:

"tfrtime <iii>" this will set timer interval for checking idle flags to <iii> minutes (1-120).

"tfroff" this will disable the timer.

"tfron" this will enable the timer.

"tfrstatus" this will return the status of the timer (enabled or disabled and time interval).

The plugin will also allow the time limit to be passed to it through the -loadplugin command line with bzfs.  The format is as follows:

"-loadplugin <path>teamflagreset,m" where m is the timer value (in minutes) for timed ctf.  For example,

"-loadplugin plugins/teamflagreset,2" would start the plugin with the timer set for 2 minutes.  Again, the minimum is 1 minute and the maximum is 120 minutes.  If no time is specified, the plugin defaults to 5 minutes.



