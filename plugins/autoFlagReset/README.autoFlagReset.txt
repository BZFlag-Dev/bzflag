BZFlag Server Plugin: autoFlagReset (Automatic Flag Reset)
================================================================================

Original author: L4m3r

Automatic Flag Reset will automatically reset unused superflags at a fixed
interval. This can be useful on maps that tend to get all their superflags
taken from certain areas and left to accumulate in others.


Loading the plugin
--------------------------------------------------------------------------------

By default, the reset frequency is every 15 minutes. If that is acceptable, you
can load the plugin without arguments.

  -loadplugin autoFlagReset

If you wish to use a different reset frequency, you can specify that in minutes.

  -loadplugin autoFlagReset,30

Optionally, the plugin can reset flags incrementally, spreading the resets
over the time interval. This setting is recommended for servers with a lot of
flags, because a large flag reset can hang clients. Enable this option by
adding "i" to the end of the time parameter:

  -loadplugin autoFlagReset,30i
