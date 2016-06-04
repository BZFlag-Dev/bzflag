BZFlag Server Plugin: CustomZoneSample
================================================================================

This is a sample plugin that shows how to use the bz_CustomZoneObject class.
This plugin creates a "msgzone" map object.  Various attributes are used to
define the position, size, and shape (rectalgular or circular) of the zone.  A
flag type and a message are also defined.  When a player enters this zone while
holding the specified flag type, they will recieve the message defined in the
object, and the server takes their flag away.


Loading the plugin
--------------------------------------------------------------------------------

This plugin takes no optional arguments, so load it with:

  -loadplugin CustomZoneSample


Map Objects
--------------------------------------------------------------------------------

The plugin adds a 'msgzone' object type.  You will specify this when creating a
custom zone.  Both zone shapes will use a 'pos' (position), 'message', and
'flag' attribute.  The rectangular type will use a 'size' and 'rot' (rotation)
attribute, and the circular type will use a 'radius' and 'height' attribute.

Here's an example of a rectangular custom zone:

msgzone
  pos 40 0 0
  size 10 15 5
  rot 45
  message "This is a no laser zone!"
  flag L
end


Here's an example of a circular custom zone:

msgzone
  pos 0 40 0
  radius 10
  height 5
  message "Sorry, we don't allow missiles here"
  flag GM
end
