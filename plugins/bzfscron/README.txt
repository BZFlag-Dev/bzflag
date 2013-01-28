========================================================================
    DYNAMIC LINK LIBRARY : bzfscron Project Overview
========================================================================

This is the bzfscron plugin.

It allows you to run commands on certain intervals.  It takes a single
argument on the commandline, a file to parse.


FILE FORMAT:

If you are familiar with crontab on unix-like systems it uses an identical
file format, like:

Min	Hour	Day	Month	Weekday	Command

Allowed values are:

field         allowed values
-----         --------------
minute        0-59
hour          0-23
day of month  1-31
month         1-12 (names are NOT allowed)
day of week   0-7 (0 or 7 is Sun, NO names)

A * means every valid value for that field.


EXAMPLE CRONTABS:

So for instance, to run "/flag reset unused" at quarter-after, 
every hour you could do

15	*	*	*	*	/flag reset unused

Or to restart your server every day at 2 am you could do

0	2	*	*	*	/shutdownserver

Multiple values (lists) can be specified, so if you wanted to restart your
server only on Mondays, Wednesdays, and Fridays, you could do

0	2	*	*	1,3,5	/shutdownserver

Likewise ranges can be used... to welcome your players every 15 minutes
in different languages depending on where they were likely from, you 
could do something like:

0,15,30,45	0-6	*	*	*	/say "Heissen Sie Willkommen zu meinem Diener!"
0,15,30,45	7-23	*	*	*	/say "Welcome to my server!"

This can also be used in concert with other plugins, for example if you
were running RPG's RaceTo7 plugin you could restart the match every 15 minutes
with a five-minute wait period like this:

0,20,40	*	*	*	*	*	/match end
0,20,40	*	*	*	*	*	/say "Next match starts in five minutes!"
1,21,41	*	*	*	*	*	/say "Next match starts in four minutes!"
2,22,42	*	*	*	*	*	/say "Next match starts in three minutes!"
3,23,43	*	*	*	*	*	/say "Next match starts in two minutes!"
4,24,44	*	*	*	*	*	/say "Next match starts in one minute!"
5,25,45	*	*	*	*	*	/match start


SIMULTANEOUS EVENTS:

Events which should occur "simultaneously" according to the crontab are
executed in the order they're written, so to restart the match immediately 
every 20 minutes you could do:

0,20,40	*	*	*	*	*	/match end
0,20,40	*	*	*	*	*	/match start

which is *different* from

0,20,40	*	*	*	*	*	/match start
0,20,40	*	*	*	*	*	/match end

which would give you a zero-time match!


MULTIPLE RESTRICTIONS:

Since there are two fields to restrict a command's execution, the behavior when
both are restricted is undefined by standard.  bzfscron chooses to execute it only
when BOTH fields are matched (e.g. if you say run on fridays only, and run on the 
13th only, it will only run on friday the 13th).  This is contrary to the way that
most modern system crons deal with this situation.  You have been warned.


FORMAT EXTENSIONS (Advanced):

The bzfscron plugin also supports "step values" like several modern crons, so

*/10	*	*	*	*	*	/flag reset unused

can be used to reset the unused flags every 10 minutes.

The bzfscron plugin also supports range/list unions, so

0-10,30-40	*	*	*	*	*	/say "I'm being annoying!"

will work just fine.

Note that those two extensions CAN be combined in the same field, so

0-10,30-59/5	*	*	*	*	*	/say "Hey, this works!"

Will run every five minutes EXCEPT for minutes FIFTEEN, TWENTY, and TWENTY-FIVE.

However, the step value is applied to the entire field, including singly 
specified numbers.  Thus

1,6-10/2	*	*	*	*	*	/say "Hey there"

will run on the SIXTH, EIGHTH and TENTH minutes, but NOT on the FIRST minute.
Contrarily,

2,6-10/2	*	*	*	*	*	/say "Hey there"

will run on the SECOND, SIXTH, EIGTH and TENTH minutes, since 2 hits the step value.
Zero always matches the step value, no matter what the step value is.
