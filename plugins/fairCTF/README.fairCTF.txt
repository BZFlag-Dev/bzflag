BZFlag Server Plugin: fairCTF
================================================================================

The fairCTF plugin regulates CTF games for team evenness. If the team sizes
become unbalanced, CTF is disabled and players are notified.  Team flags then
cannot be picked up, and previously-held team flags may be dropped.  Once the
teams even up again, CTF is re-enabled and the game can continue.

This plugin uses a state-based approach that was designed for 2 team CTF
games.  While it will still work with more teams, chances are that CTF play
will be disabled most of the time.  Fairness is determined by the sizes of the
largest and smallest teams.  For 3 or 4 team games, a more dynamic calculation
of fairness would probably work better.  Pacman87's "fair4CTF" is more suited to
this approach.


Loading the plugin
--------------------------------------------------------------------------------

To load the plugin with default settings, use:
  -loadplugin fairCTF

To load the plugin with specific settings, use the format:
  -loadplugin fairCTF,<maxRatioDiff>:<smallTeamLimit>:<maxDiff>:<delay>

<maxRationDiff>
  The maximum difference in team size, by ratio. The ratio is determined by
  dividing the arithmetic difference by the size of the smallest team. Thus, if
  it were 10 vs. 8, the plugin would calculate the difference ratio at
    2 / 8 = 0.25.
  Specify this as a decimal number. The default is 0.25.

<smallTeamLimit>
  The maximum size at which a difference of one player is not fair. This
  compensates for the fact that a straight ratio can be a bit overzealous when
  there are only a few players. As before, this goes by the smaller team. If "2"
  was specified for this parameter, the plugin would consider 2 vs 3 unfair, but
  3 vs. 4 would be acceptable. The default is 2. Note that this parameter takes
  precedence when the teams are off by one player- the ratio-based measurement
  will not even be tested.

<maxDiff>
  The absolute maximum arithmetic difference in players. Fairly
  straightforward. The default is 3.

<delay>
  The delay, in seconds, between the time that teams become uneven and the
  time that all team flags get dropped. Players cannot grab team flags at all
  once the teams become uneven, but players who were already holding team flags
  may be given some time to reach their base before the flags are frozen. This
  helps mitigate abuse issues with players leaving the game to disrupt an
  impending capture. Set this to 0 to drop flags instantly. If -1 is given,
  flags will not force-drop at all; any team flags will be stuck wherever the
  players drop them (by capturing, dying, or regular dropping). Only integers
  are valid; The default is 5 seconds.

If any of these parameters are omitted, the defaults will be used.


Server Commands
--------------------------------------------------------------------------------

This plugin adds a single '/ctf' command that requires the "FAIRCTF" permission.
It allows manually overriding the plugin and returning it to automatic
operation.  These commands will publicly announce changes (and who made them)
only when they change the state of CTF.  For example, if CTF is enabled, '/ctf
on' will not create an announcement, but '/ctf off' will. The admin channel will
always be notified of any change, however.

/ctf on
  Override the plugin and enable CTF until another user changes the setting.
  This basically turns off the plug-in.

/ctf off
  Disables CTF until the setting is changed.

/ctf auto
  Returns control of the game to the plugin; ends user override. The plugin will
  resume normal operation after this command is issued.
