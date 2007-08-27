========================================================================
    DYNAMIC LINK LIBRARY : rabbitTimer Project Overview
========================================================================

The rabbitTimer plug-in helps solve that pesky rabbit cowardice problem.
Rabbits don't get any points for hiding; they must kill hunters to 
increase their score. Hiding also slows down the game and frustrates
the hunters, especially when flags like ST or WG are involved.

This plug-in is used to set a maximum time between kills for the rabbit.
In other words, the rabbit must kill a hunter every N seconds, or they 
are destroyed. There is also an optional rollover mode, in which extra 
time leftover from kills are added to the time allowance. Simply pass 
the desired time limit (in seconds) to the plug-in as a parameter, like 
this:

-loadplugin /home/me/bzflag/plugins/rabbitTimer.so,30

To enable rollover mode, add a + to the beginning of the number:

-loadplugin /home/me/bzflag/plugins/rabbitTimer.so,+30

By default, the time limit is 30 seconds, and rollover mode is disabled.