========================================================================
    DYNAMIC LINK LIBRARY : rabidRabbit Plugin
========================================================================

This is the rabidRabbit plugin.

Description:

The rabidrabbit plugin will allow a rabbit player to enter a map
defined volume (rabid rabbit 'zone') and kill all hunters at once.
The plugin will accomodate as many rabidrabbit zones as are defined in
the map file (minimum of 2) and will cycle through each rabidrabbit
zones sequentially so that the rabbit cannot repeatedly use the same
zone to kill the hunters.  Map designers are highly advised to clearly
indicate rabidrabbit zones in the map (texturing, world weapon option
below, etc.), so that they are easily identified by all players.

If a rabbit enters the incorrect rabidrabbit zone, a message will be
sent to him/her that it isn't the correct rabidrabbit zone and to try
the next zone.  If the 'zonekillhunter' option is defined in the map
file (see below), a hunter that enters the rabidrabbit zone will be
killed.  If the 'zonekillhunter' option is not defined, there will be
no effect on a hunter entering the rabidrabbit zone.

There is also an option to fire a zone defined world weapon for
indication to players which zone is active.

-------------------------------------------------------------------------------------

The parameters of the rabidrabbitzone map definition are follows:

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

Optional.  If used, sends custom message to all players when a rabbit kills all hunters by entering rabidrabbit zone.

zonekillhunter "your message here"

Optional.  If used, a hunter that enters the rabidrabbit zone will be killed with a custom message.

There is also a separate map parameter available related to rabid rabbit:

rrsoundoff

Optional.  This will disable sounds associated with the rabid rabbit plugin.

-------------------------------------------------------------------------------------

Examples of map (.bzw) entries:

rabidrabbitzone
  bbox -80 -30 -80 -30 0 2
  rrzoneww SW 1 -50 -50 1000 90 0 0 0 .5
  servermessage "You were killed by a rabid rabbit!"
  zonekillhunter "This area for rabbits only!"
end
rabidrabbitzone
  bbox 30 80 30 80 0 2
  rrzoneww SW 1 50 50 25 1000 0 0 0 .5
  servermessage "You were killed by a rabid rabbit!"
  zonekillhunter "This area for rabbits only!"
end

rrsoundoff
end

This defines the 2 zones that the rabbit can enter to kill all
hunters.  The plugin will use the first volume definition (bbox -80
-30 -80 -30 0 2) as the zone for the rabbit to enter to kill hunters.
If the rabbit successfully enters this zone and kill the hunters, the
plugin will use the next defined volume (bbox 30 80 30 80 0 2) as the
zone for the rabbit to enter to kill hunters.  And so on - the plugin
will cycle through each zone sequentially.  If a rabbit successfully
enters the correct zone, all hunters will be killed with a message
stating, "You were killed by a rabid rabbit!"  If a hunter enters a
rabidrabbit zone, he/she will be killed with message "This area for
rabbits only!"  A laser world weapon will repeatedly fire every 0.5
seconds (at the coordinates and with the tilt/direction indicated)
when each zone is active for rabidrabbit.  Rabid rabbit sounds will be
disabled due to "rrsoundoff" map entry.
