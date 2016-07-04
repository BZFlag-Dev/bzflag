BZFlag Server Plugin: flagStay
================================================================================

The flagStay plugin adds a custom map object that allows you to prevent
specific flags from leaving a defined area (either rectangular or circular).


Loading the plugin
--------------------------------------------------------------------------------

This plugin takes no optional arguments, so load it with:

  -loadplugin flagStay


Map Objects
--------------------------------------------------------------------------------

The plugin adds a 'flagstayzone' object type.  You will specify this when
creating a custom zone.  Both zone shapes will use a 'pos' (position),
'message', and one or more 'flag' attributes.  The rectangular type will use a
'size' and 'rot' (rotation) attribute, and the circular type will use a 'radius'
and 'height' attribute.  This plugin would almost always be used in combination
with a normal 'zone' object that would spawn flags in a specific area using the
'zoneflag' attribute.

# Here's an example of a rectangular zone that prevents GM and L from leaving a
# tower area.

# Spawn 5 GM, 5 L, and 1 G flags on the tower.
zone
  pos 0 0 40
  size 10 10 1
  zoneflag GM 5
  zoneflag L 5
  zoneflag G 1
end

# Keep GM and L within the tower.
flagstayzone
  pos 0 0 40
  size 10 10 10
  rot 45
  message "You can't leave the tower with that flag!"
  flag GM
  flag L
end


# Here's an example of a circular flagstay zone that keeps a machine gun flag in
# a defined area.

# Spawn an MG flag at 80 0 0 within a 1 by 1 area.
zone
  pos 80 0 0
  size 1 1 1
  zoneflag MG 1
end

# Keep it within a 5 unit radius of the center.
flagstayzone
  pos 80 0 0
  radius 5
  height 10
  message "The MG can't leave the turret area"
  flag MG
end
