BZFlag plug-in: fairCTF 
Original Author: L4m3r
=======================

~INTRODUCTION~
During a CTF game, don't you hate it when someone keeps capturing the flag after teams have become very uneven? No more.

fairCTF is a plugin that continually checks team sizes to ensure that they are fair. If the plugin decides that the teams aren't even enough, CTF is disabled and players are notified; team flags will be dropped instantly when picked up. Once the teams even up again, CTF is re-enabled and the game can continue.


~PARAMETERS~
What the plugin defines as fair can be specified by three parameters passed to the plugin, in the following format in the conf file:

	-loadplugin "/pathtoplugin/fairCTF.so,X:Y:Z"

where X, Y, and Z are the following:

X: The maximum difference in team size, by ratio. The ratio is determined by dividing the arithmetic difference by the size of the smallest team. Thus, if it were 10 vs. 8, the plugin would calculate the difference ratio at 2 / 8 = 0.25. Specify this as a decimal number. The default is 0.25. 

Y: The maximum size at which a difference of one player is not fair. This setting may seem a little weird, but I figured it was a good idea because a straight ratio can be a bit overzealous when there are only a few players. Again, this goes by the smaller team. If you specified 2 for this parameter, the plugin would consider 2 vs 3 unfair, but 3 vs. 4 would be ok. The default is 2. Note that this parameter takes precedence when the teams are off by one player- the ratio will not even be tested.

Z: The absolute maximum arithmetic difference in players. Fairly straightforward. The default is 3.

If any of these parameters are omitted, the defaults will be used. 


~COMMANDS~
Users with adequate perms can override the plugin by manually enabling or disabling CTF. This is done with the /ctf command. It will accept three parameters:
/ctf on: Override the plugin and enable CTF until another user changes the setting. This basically disables the plugin.
/ctf off: Disables CTF until the setting is changed.
/ctf auto: Returns control of the game to the plugin; ends user override. The plugin will resume normal operation after this command is issued.

Add the perm "FAIRCTF" to any groups who you want to be able to use these commands. 

These commands will publically announce changes (and who made them) only when they change the state of CTF. For example, if CTF is enabled, /ctf on will not create an announcement, but /ctf off will. The admin channel will always be notified of any change, however.


~OTHER THINGS YOU SHOULD KNOW~
-Rogues and observers are ignored when the plugin is calculating the evenness of the teams.

-This plugin is not really designed or intended for three or four team CTF, but it *will* work. Comparisons are made between the largest and the smallest team. I recommend setting the second parameter (Y) to 1 if you are using this plugin with more than two teams. I strongly advise against using this with four team CTF on a smaller server, because the plugin will most likely disable CTF most of the time.

-There is a five second delay for the plugin to detect a change in the state of fairness in the game. This is a workarond for the spastic nature of the plugin, the cause of which is not fully understood.

-This plugin will reset team scores when only one team is present, to prevent those weird 47 to -6 scores. This is intended for servers that have bots (or junkies). 

-The server will silently allow CTF when only one team is present. This allows the one team to take its flag back to their base.


~ENJOY~
Special thanks to the Planet MoFo team for helping me develop this plugin.