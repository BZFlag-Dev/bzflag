TimeLimit plugin version 1.0.4  (December 26, 2006)
---------------------------------------------------

Author: Steven Mertens <steven.mertens@catay.be>

The TimeLimit plugin makes it possible to change live the
match duration on a match server.


Plugin command Line:
====================
  -loadplugin PLUGINNAME[,<time1>,<time2>] | ,starttime-endtime

  The first example allows every match duration limit.

  Example:   -loadplugin TimeLimit

  The second example allows only those match durations
  passed as arguments. A maximum of 20 different durations can
  be specified.

  Example :   -loadplugin TimeLimit,5,15,30,60

  The third example allows a specific range of match durations.

  Example :   -loadplugin TimeLimit,5-15


  When the wrong arguments get passed through, the plugin will
  fallback to the first example.

In-game commands:
=================

The timelimit in game command requires the TIMELIMIT permission.
It is also required the bzfs server is started with -timemanual .

  /timelimit          : displays the usage message
  /timelimit <minutes>: sets the timelimit
  /timelimit show     : shows the current set timelimit
  /timelimit reset    : resets the timelimit back to default (-time setting)

  Example : /timelimit 15

Matches:
========

 Before starting a match with the /countdown command you can set the
 match duration with the /timelimit command.
 It's not possible to set the match duration when a countdown is in
 progress or when a match is already started.
 The timelimit will reset itself to the -time value at the end of a
 match or when there are no players on the server.

Changelog:
==========

 * TimeLimit 1.0.4 (26 December 2006)

   - Added a makefile so the plugin doesn't relay anymore on
     bzflag plugin build system.

 * TimeLimit 1.0.3 (26 July 2006)

   - Applied uso his changes (timelimit reset at end of game)
   - Add feature that allows a range of match durations

 * TimeLimit 1.0.2 (08 June 2006)

   - Add feature that only allows certain match durations

 * TimeLimit 1.0.1 (18 May 2006)

   - bugfix : don't allow negative match durations

 * TimeLimit 1.0.0 (17 May 2006)

   - Initial release
