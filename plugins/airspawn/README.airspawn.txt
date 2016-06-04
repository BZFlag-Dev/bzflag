BZFlag Server Plugin: airspawn
================================================================================

The airspawn plugin modifies the spawn location of all tanks, moving them up
into the air a specified number of units.  Do note that if there are objects
above the ground that it will be possible that tanks spawn inside solid objects.


Loading the plugin
--------------------------------------------------------------------------------

The plugin can be loaded without arguments to spawn takes 10 units higher.

  -loadplugin airspawn

You can also pass a positive float value to the plugin to use a different value.

  -loadplugin airspawn,15.5
  -loadplugin airspawn,6
