BZFlag Server Plugin: wwzones
================================================================================

Original author: LouMan

The plugin will fire a map defined world weapon whenever a player is in a map
defined volume (either rectangular or circular).  It could be used for
landmines, booby traps, clearing campers in difficult locations, general
amusement, etc.  Multiple zones with different weapons, etc. may be used in
same map file.


Loading the plugin
--------------------------------------------------------------------------------

This plugin takes no optional arguments, so load it with:

  -loadplugin wwzones


Map Objects
--------------------------------------------------------------------------------

The plugin adds a 'wwzone' object type.  You will specify this when creating a
custom zone.  Both zone shapes will use a 'pos' (position), various attributes
for controlling the world weapon and messages.  The rectangular type will use a
'size' and 'rot' (rotation) attribute, and the circular type will use a 'radius'
and 'height' attribute.

Here are the common attributes and how to use them:

Define the weapon that will be triggered from this wwzone:
  zoneweapon <flagType> <lifetime> <X> <Y> <Z> <tilt> <direction> <shotID> <DT>

  <flagType>
    This is a valid type of flag to use for the world weapon (e.g. SW, GM, etc)
  <lifetime>
    This defines (in seconds) how long the shot should exist.
  <X> <Y> <Z>
    This defines the starting position of the world weapon.
  <tilt>
    This defines the tilt angle of the world weapon (in degrees) with 0 being
    level to the ground.
  <direction>
    This defines the rotation angle (in degrees) with 0 pointing east, and
    moving in a counter-clockwise direction.  So 90 is north, 45 is northeast,
    and so on.
  <shotID>
    This value should be 0 unless you know what you're doing. Using a value of 0
    will let the server generate an appropriate shot ID for you.
  <DT>
    This value is completely ignored, so just pass a 0 to it.

This optional attribute sends custom message to player when he/she triggers the
world weapon.  It is sent as a private message to the player:
  playermessage "your message here"

This optional attribute sends a custom message to all players when the world
weapon is triggered:
  servermessage "your message here"

This optional attribute will issue a plugin defined message to all players
stating which type of world weapon was triggered by whom:
  infomessage

This optional attribute will cause the world weapon to fire releated every
<seconds> seconds while player is in defined volume:
  repeat <seconds>

  <seconds>
    This defines the repeat time of the weapon (minimum 0.1 seconds).  If you
    specify only "repeat" but don't include a value after it, it will repeat
    every 0.5 seconds.  If you don't include repeat in the wwzone definition, it
    will only fire once when player enters zone.

This optional attribute will delay the initial fire of world weapon by <seconds>
seconds:
  timedelay <seconds>


# This will fire a shockwave every 0.25 seconds (approximately) after a player
# is in the volume defined by the bbox for more than 2 seconds.  In this case, a
# square volume 50 x 50 x 1 centered at coordinates -50 50 1.  The shockwaves
# will occur at coordinates 0 0 1 and will send message "You triggered
# Shockwave!" to player that triggered it, when initially fired.

wwzone
  pos -50 50 0
  size 50 50 1
  zoneweapon SW 1 0 0 1 0 0 0 0
  playermessage "You triggered Shockwave!"
  repeat .25
  timedelay 2
end


# This will fire a laser, every 0.5 seconds (approximately) while a player is in
# the volume defined by the bbox.  In this case, a square volume 50 x 50 x 1
# centered at coordinates 50 50 1.  The laser will occur at coordinates 5 5 1
# and be tilted at 45 degrees with a heading of 25 degrees.  When initially
# fired, there is a message sent to all players "Laserz rule!"

wwzone
  pos 50 50 0
  size 50 50 1
  zoneweapon L 1 5 5 1 45 25 0 0
  servermessage "Laserz rule!"
  repeat
end


# This will fire a GM one time after a player has been in the volume defined by
# the cylinder for more than 1 second.  In this case, a cylindrical volume with
# a radius of 20, centered at coordinates 50 -50 3.  The GM will shoot from
# coordinates 0 0 1 with 0 tilt and a heading of 0 degrees.  When initially
# fired, there is a message sent to all players "GM triggered by <callsign>."

wwzone
  pos 50 -50 0
  radius 20
  height 6
  zoneweapon GM 1 0 0 1 0 0 0 0
  infomessage
  timedelay 1
end
