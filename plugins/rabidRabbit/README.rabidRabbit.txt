BZFlag Server Plugin: rabidRabbit
================================================================================

Original author: LouMan

This plugin will allow a rabbit player to enter a map defined volume (rabid
rabbit 'zone') and kill all hunters at once.  The plugin will accommodate as
many rabidRabbit zones as are defined in the map file (minimum of 2) and will
cycle through each rabidRabbit zones sequentially so that the rabbit cannot
repeatedly use the same zone to kill the hunters.  Map designers are highly
advised to clearly indicate rabidRabbit zones in the map (texturing, world
weapon option below, etc.), so that they are easily identified by all players.

If a rabbit enters the incorrect rabidRabbit zone, a message will be sent to
him/her that it isn't the correct rabidRabbit zone and to try the next zone.
If the 'zonekillhunter' option is defined in the map file (see below), a hunter
that enters the rabidRabbit zone will be killed.  If the  'zonekillhunter'
option is not defined, there will be no effect on a hunter entering the
rabidRabbit zone.

The zone may be a rectangular box or a cylinder.  There is an option to fire a
zone defined world weapon for indication to players which zone is active.  There
is also an option to cycle to the next rabid rabbit zone upon the rabbit's
death.


Loading the plugin
--------------------------------------------------------------------------------

This plugin takes no optional arguments, so load it with:

  -loadplugin rabidRabbit


Configuration
--------------------------------------------------------------------------------

This plugin exposes the following BZDB variable:

_rrCycleOnDeath
  This boolean option (true or false) if set to true will cause the plugin to
  cycle to the next rabidRabbit zone upon the rabbit's death.


Map Objects
--------------------------------------------------------------------------------

The plugin adds a 'rabidRabbitzone' object type.  You will specify this when
creating a custom zone.  Both zone shapes will use a 'pos' (position), various
attributes for controlling the world weapon and messages.  The rectangular type
will use a 'size' and 'rot' (rotation) attribute, and the circular type will use
a 'radius' and 'height' attribute.

Here are the common attributes and how to use them:

Define the weapon that will be triggered from this wwzone:
  rrzoneww <flagType> <lifetime> <X> <Y> <Z> <tilt> <direction> <shotID> <DT> <repeat>

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
  <repeat>
    The amount of time between the world weapon shots.  Minimum is 0.1 seconds.

This optional attribute sends a custom message to all players a rabbit enters
the active rabidRabbit zone and kills all the hunters:
  servermessage "your message here"

When a hunter enters a rabidRabbit zone, they will be killed.  This optional
attribute sends a custom message to a hunter they enter this zone:
  zonekillhunter "your message here"



# This defines the two zones that the rabbit can enter to kill all hunters.  If
# the rabbit successfully enters the active zone, the hunters will be killed and
# the active zone will cycle to the next zone.

# This will define a zone at -50 -50 0 with a size of 10 x 10 x 1.  A shockwave
# will fire every 0.5 seconds at -50 -50 1000 when active.
rabidRabbitzone
  pos -50 -50 0
  size 10 10 1
  rrzoneww SW 1 -50 -50 1000 90 0 0 0 .5
  servermessage "You were killed by a rabid rabbit!"
  zonekillhunter "This area for rabbits only!"
end

# This will define a zone at 50 50 0 with a size of 10 x 10 x 1.  A shockwave
# will fire every 0.5 seconds at 50 50 1000 when active.
rabidRabbitzone
  pos 50 50 0
  size 10 10 1
  rrzoneww SW 1 50 50 1000 90 0 0 0 .5
  servermessage "You were killed by a rabid rabbit!"
  zonekillhunter "This area for rabbits only!"
end
