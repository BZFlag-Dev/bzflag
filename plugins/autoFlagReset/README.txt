===============================================================================
Automatic Flag Reset: BZFlag server plug-in
Original author: L4m3r
===============================================================================

Automatic Flag Reset will automatically reset unused superflags at a fixed 
interval. This can be useful on maps that tend to get all their superflags 
taken from certain areas and left to accumulate in others.

Specify the interval in minutes by passing it as a parameter to the plugin:

	-loadplugin /pathtoplugin/autoFlagReset.so,15

The default frequency is once every 15 minutes.

Optionally, the plugin can reset flags incrementally, spreading the resets 
over the time interval. This setting is recommended for servers with a lot of 
flags, because a large flag reset can hang clients. Enable this option by 
adding "i" to the end of the time parameter:

	-loadplugin /pathtoplugin/autoFlagReset.so,15i



