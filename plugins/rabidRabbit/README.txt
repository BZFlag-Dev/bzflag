========================================================================
    DYNAMIC LINK LIBRARY : rabidRabbit Project Overview
========================================================================

rabidRabbit plugin

Author: LouMan

Description:

The rabidRabbit plugin will allow a rabbit player to enter a map defined volume (rabid rabbit 'zone') and kill all hunters at once.  The plugin will accomodate as many rabidRabbit zones as are defined in the map file (minimum of 2) and will cycle through each rabidRabbit zones sequentially so that the rabbit cannot repeatedly use the same zone to kill the hunters.  Map designers are highly advised to clearly indicate rabidRabbit zones in the map (texturing, world weapon option below, etc.), so that they are easily identified by all players.

If a rabbit enters the incorrect rabidRabbit zone, a message will be sent to him/her that it isn't the correct rabidRabbit zone and to try the next zone.  If the 'zonekillhunter' option is defined in the map file (see below), a hunter that enters the rabidRabbit zone will be killed.  If the  'zonekillhunter' option is not defined, there will be no effect on a hunter entering the rabidRabbit zone.

There is an option to fire a zone defined world weapon for indication to players which zone is active.
There is also an option to cycle to the next rabid rabbit zone upon the rabbit's death.

-------------------------------------------------------------------------------------

The parameters of the rabidRabbitzone map definition are follows:


bbox <Xmin> <Xmax> <Ymin> <Ymax> <Zmin> <Zmax>
or
cylinder <X> <Y> <Zmin> <Zmax> <radius>

Required (at least 2 volumes).  These define a volume on the map that the rabbit can enter to trigger death of all hunters.


rrzoneww <flagType> <lifetime> <X> <Y> <Z> <tilt> <direction> <shotID> <DT> <repeat>

Optional. This defines the world weapon.
<flagType> is a valid type of flag to use for the world weapon (e.g. SW, GM, L, etc.)
<lifetime> defines the life of the weapon's shots.
<X> <Y> <Z> defines the location of the world weapon.
<tilt> defined the tilt of the world weapon.
<direction> defines the direction of the world weapon.
<shotID> ...not sure what this does ;) but it is an input to the API world weapon function.
<DT> defines the delay time of the world weapon.
<repeat> defines the amount of time between the world weapon shots.  Minimum = 0.1 seconds.


servermessage "your message here"

Optional.  If used, sends custom message to all players when a rabbit kills all hunters by entering rabidRabbit zone.


zonekillhunter "your message here"

Optional.  If used, a hunter that enters the rabidRabbit zone will be killed with a custom message.


There is also a separate map parameter available related to rabid rabbit:

rrsoundoff

Optional.  This will disable sounds associated with the rabid rabbit plugin.


rrcycleondie

Optional.  This will cause the plugin to cycle to the next rabid rabbit zone upon the rabbit's death.


-------------------------------------------------------------------------------------

Examples of map (.bzw) entries:

rabidRabbitzone
  bbox -80 -30 -80 -30 0 2
  rrzoneww SW 1 -50 -50 1000 90 0 0 0 .5
  servermessage "You were killed by a rabid rabbit!"
  zonekillhunter "This area for rabbits only!"
end
rabidRabbitzone
  bbox 30 80 30 80 0 2
  rrzoneww SW 1 50 50 25 1000 0 0 0 .5
  servermessage "You were killed by a rabid rabbit!"
  zonekillhunter "This area for rabbits only!"
end

rrsoundoff
end

rrcycleondie
end


This defines the 2 zones that the rabbit can enter to kill all hunters.  The plugin will use the first volume definition (bbox -80 -30 -80 -30 0 2) as the zone for the rabbit to enter to kill hunters.  If the rabbit successfully enters this zone and kill the hunters, the plugin will use the next defined volume (bbox 30 80 30 80 0 2) as the zone for the rabbit to enter to kill hunters.  And so on - the plugin will cycle through each zone sequentially.  If a rabbit successfully enters the correct zone, all hunters will be killed with a message stating, "You were killed by a rabid rabbit!"  If a hunter enters a rabidRabbit zone, he/she will be killed with message "This area for rabbits only!"  A laser world weapon will repeatedly fire
every 0.5 seconds (at the coordinates and with the tilt/direction indicated) when each zone is active for rabidRabbit.  Rabid rabbit sounds will be disabled due to "rrsoundoff" map entry, and the plugin will cycle to the next rabid rabbit zone upon the rabbit's death.

-------------------------------------------------------------------------------------

Please report any bugs or issues with this plugin to LouMan, and good luck.
