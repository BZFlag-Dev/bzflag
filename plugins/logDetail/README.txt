========================================================================
    DYNAMIC LINK LIBRARY : logDetail Project Overview
========================================================================

This is the logDetail plugin. It displays information about server events 
and chat to the standard output server log.

The log detail plugin displays:
 * unprocessed user commands (slash commands before they are processed - valid 
 	or not)
 * player reports
 * chat messages (broadcasts, team messages, admin channel messages, and 
 	private messages)
 * server messages
 * players joining (ip, auth status, team, etc)
 * player authentication changes (via /password or /identify)
 * players leaving

The plugin currently takes no parameters.

Sample output:

PLAYER-JOIN 10:SomePlayer #0 OBSERVER IP:127.0.0.1
PLAYER-JOIN 15:SomeOtherPlayer #1 RED IP:127.0.0.1
MSG-BROADCAST 15:SomeOtherPlayer hi
MSG-DIRECT 15:SomeOtherPlayer 10:SomePlayer what's up
MSG-TEAM 15:SomeOtherPlayer RED hi team!
MSG-ADMIN 15:SomeOtherPlayer hello admins!
MSG-TEAM 10:SomePlayer OBSERVER hi teammates
MSG-DIRECT 10:SomePlayer 15:SomeOtherPlayer what's up?
MSG-COMMAND 10:SomePlayer password oopsIforgot
MSG-ADMIN 6:SERVER SomePlayer has tried to become administrator with bad password
MSG-COMMAND 10:SomePlayer password secret
PLAYER-AUTH 10:SomePlayer IP:127.0.0.1 ADMIN OPERATOR
MSG-COMMAND 10:SomePlayer say this is a server message
MSG-BROADCAST 6:SERVER this is a server message (SomePlayer)
MSG-COMMAND 10:SomePlayer lagstats
MSG-REPORT 10:SomePlayer nothing to report
MSG-COMMAND 15:SomeOtherPlayer quit
PLAYER-PART 15:SomeOtherPlayer #1
MSG-COMMAND 10:SomePlayer shutdownserver
