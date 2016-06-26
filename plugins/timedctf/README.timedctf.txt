BZFlag Server Plugin: timedctf
================================================================================

Original author: Louman

It will start a timed capture the flag game in which a team must capture another
team's flag in a configurable amount of time. If the time expires before the
team captures a flag, all members of the team will be destroyed and the timer
will restart for that team. If the team captures another team's flag before
their time expires, their timer is reset and starts counting down again. All 4
teams have individual timers.

There are warning messages sent to individual teams as their time starts to
expire:
* A warning every minute (with time remaining) until the last minute.
* A warning at 30 seconds before time expires.
* A warning at 20 seconds before time expires.
* A warning at 10 seconds before time expires.
* (boom)

The plugin also monitors the balance of teams and will disable the timer if
teams are uneven. To determine if teams are balanced or not, at least a 75%
match is used. This means that even teams would be considered:
* 1 vs 1 (100%)
* 2 vs 2 (100%)
* 3 vs 3 (100%)
* 3 vs 4 (75%)
* 4 vs 4 (100%)
* 4 vs 5 (80%)
* 5 vs 5 (100%)
* 5 vs 6 (83%)
* and so on.

If the teams are uneven, capture the flag is disabled; if a player tries to pick
up a team flag, it will almost immediately be dropped.

The plugin is set up for all 4 teams (red, green, blue and purple) and will
monitor/time all 4 teams. However, it is best suited to 2 team capture the flag
type maps in that it only looks for the 75% match between any 2 team sizes. This
means that it is possible (on a 4 team ctf map) to have teams considered to be
even if:
* 1 vs 1 vs 3 vs 1
* 2 vs 2 vs 8 vs 1
* and so on.

There are also informative messages when the status of the game changes, for
example:

* If a new player joins/parts that causes team imbalance, a message is sent
  stating that teams are uneven and timed ctf is disabled.
* If a new player joins/parts that balances teams out, a message is sent that
  timed ctf is enabled and how much time there is to capture a flag.
* And so on...


Loading the plugin
--------------------------------------------------------------------------------

To load the plugin with default setting of 5 minutes, use:
  -loadplugin timedctf

To load the plugin with a different timer value, use the format, where <minutes>
is a value from 1 to 120:
  -loadplugin timedctf,<minutes>
For example:
  -loadplugin timedctf,10


Server Commands
--------------------------------------------------------------------------------

The plugin has the following commands available to administrators:

/tctftime <iii>
  Change required capture time to <iii> (1-120 minutes).

/tctfoff
  Disable timed CTF (but see the note below).

/tctfon
  Enable timed CTF.

/tctfstatus
  Return the current status of functions and the current time limit.

/fairctfoff
  Disable the fair ctf function.

/fairctfon
  Enable the fair ctf function.

Note that the balance of teams will still be monitored when the ctf timer is
off, and team flags will be dropped if teams are uneven (as described above).
This allows this plugin to be utilized for fair, normal ctf play as well, if the
ctf timer is turned off.  If both functions (tctf and fairctf) are turned off,
the plugin is effectively disabled.  If the fair CTF function is disabled and
the timed CTF function is enabled, the plugin will check for at least 2 teams
present before allowing timed CTF to take place.
