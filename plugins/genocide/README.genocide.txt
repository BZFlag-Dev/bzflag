BZFlag Server Plugin: genocide
================================================================================

This is a simple sample plugin that implements the legacy "genocide" flag from
previous versions.  Shots from this flag will kill all members of the team it
strikes, except rogue by default.


Loading the plugin
--------------------------------------------------------------------------------

By default, simply genocide is enabled.  If that is acceptable, 
you can just load the plugin without additional arguments.

  -loadplugin genocide

To enable rogueGenocide, specify 1 or 2 as arguments,
with 2 enabling the "suicide" option of rogue genocide.

  -loadplugin genocide,1
