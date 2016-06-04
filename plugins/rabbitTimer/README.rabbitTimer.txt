BZFlag Server Plugin: rabbitTimer
================================================================================

Original author: L4m3r

The rabbitTimer plug-in helps solve that pesky rabbit cowardice problem.
Rabbits don't get any points for hiding; they must kill hunters to increase
their score.  Hiding also slows down the game and frustrates the hunters,
especially when flags like ST or WG are involved.

This plugin is used to set a maximum time between kills for the rabbit.  In
other words, the rabbit must kill a hunter every N seconds, or they are
destroyed.  There is also an optional rollover mode, in which extra time
left over from kills are added to the time allowance.

Loading the plugin
--------------------------------------------------------------------------------

To load the plugin with default settings (30 seconds without rollover), use:
  -loadplugin rabbitTimer

To load the plugin with a different timer, use the format:
  -loadplugin SAMPLE_PLUGIN,<seconds>
For example:
  -loadplugin SAMPLE_PLUGIN,60

To load the plugin with a different timer with rollover, use the format:
  -loadplugin SAMPLE_PLUGIN,+<seconds>
For example:
  -loadplugin SAMPLE_PLUGIN,+60
