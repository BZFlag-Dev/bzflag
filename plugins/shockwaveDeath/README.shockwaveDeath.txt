BZFlag Server Plugin: shockwaveDeath
================================================================================

The shockwaveDeath plugin causes shockwaves to appear where a tank blows up.
So don't get too close when shooting another tank or you're get caught in the
explosion!


Loading the plugin
--------------------------------------------------------------------------------

This plugin takes no optional arguments, so load it with:

  -loadplugin shockwaveDeath


Configuration
--------------------------------------------------------------------------------

The plugin has a single BZDB variable that can be set to affect the lifetime of
the shockwaves it creates.  This can be used to make the shockwave last a
shorter or longer amount of time.  For instance, setting it to 0.5 would make it
last half as long as a normal shockwave:

  -set _swDeathReloadFactor 0.5
