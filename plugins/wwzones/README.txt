========================================================================
    DYNAMIC LINK LIBRARY : wwzones Project Overview
========================================================================

wwzones plugin

Author: LouMan

Credit to JeffM2501 for base zone and custom map object code (from
flagStay plugin), which I have modified to fit this application.

Description: 

The plugin will fire a map defined world weapon whenever a player is
in a map defined volume.  It could be used for landmines, booby traps,
clearing campers in difficult locations, general amusement, etc.
Multiple zones with different weapons, etc. may be used in same map
file.

-------------------------------------------------------------------------------------

The parameters of the wwzone map definition are follows:


bbox <Xmin> <Xmax> <Ymin> <Ymax> <Zmin> <Zmax>
or
cylinder <X> <Y> <Zmin> <Zmax> <radius>

These define the volume on the map that will trigger a world weapon
whenever players enter it.


zoneweapon <flagType> <lifetime> <X> <Y> <Z> <tilt> <direction> <shotID> <DT>

This defines the world weapon.
<flagType> is a valid type of flag to use for the world weapon (e.g. SW, GM, L, etc.)
<lifetime> defines the life of the weapon's shots.
<X> <Y> <Z> defines the location of the world weapon.
<tilt> defined the tilt of the world weapon.
<direction> defines the direction of the world weapon.
<shotID> ...not sure what this does ;) but it is an input to the API
         world weapon function.
<DT> defines the delay time of the world weapon.


playermessage "your message here"

If used (optional), sends custom message to player when he/she
triggers world weapon.


servermessage "your message here"

If used (optional), sends custom message to all players when a world
weapon is triggered.


infomessage

If used (optional), this will issue standard message to all players
stating which type of world weapon was triggered by whom.  This
message cannot be customized.


repeat <seconds>

If used (optional), will repeat weapon fire while player is in defined
volume.  <seconds> defines the repeat time of the weapon (minimum 0.1
seconds).  If <seconds> not specified, default is 0.5 seconds.  If
repeat is not included in definition, weapon will only fire once when
player enters zone.


timedelay <seconds>

If used (optional), will delay initial fire of world weapon by
specified number of seconds.

-------------------------------------------------------------------------------------

Examples of map (.bzw) entries:


wwzone
  bbox -80 -30 -80 -30 0 2
  zoneweapon SW 1 0 0 1 0 0 0 0
  playermessage "You triggered Shockwave!"
  repeat .25
  timedelay 2
end 

This will fire a shockwave every 0.25 seconds (approximately) after a
player is in the volume defined by the bbox for more than 2 seconds.
In this case, a square volume 50 x 50 x 1 centered at coordinates -50
50 1.  The shockwaves will occur at coordinates 0 0 1 and will send
message "You triggered Shockwave!" to player that triggered it, when
initially fired.


wwzone
  bbox 30 80 30 80 0 2
  zoneweapon L 1 5 5 1 45 25 0 0
  servermessage "Laserz rule!"
  repeat
end

This will fire a laser, every 0.5 seconds (approximately) while a
player is in the volume defined by the bbox.  In this case, a square
volume 50 x 50 x 1 centered at coordinates 50 50 1.  The laser will
occur at coordinates 5 5 1 and be tilted at 45 degrees with a heading
of 25 degrees.  When initially fired, there is a message sent to all
players "Laserz rule!"


wwzone
  cylinder 50 -50 0 6 20
  zoneweapon GM 1 0 0 1 0 0 0 0
  infomessage
  timedelay 1
end

This will fire a GM one time after a player has been in the volume
defined by the cylinder for more than 1 second.  In this case, a
cylindrical volume with a radius of 20, centered at coordinates 50 -50
3.  The GM will shoot from coordinates 0 0 1 with 0 tilt and a heading
of 0 degrees.  When initially fired, there is a message sent to all
players "GM triggered by <callsign>."

-------------------------------------------------------------------------------------

Please report any bugs or issues with this plugin to LouMan, and good luck.
