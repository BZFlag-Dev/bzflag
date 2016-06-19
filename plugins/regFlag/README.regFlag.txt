BZFlag Server Plugin: regFlag
================================================================================

When this plugin is loaded, players that are not registered and identified will
not be able to grab flags (good or bad).  They will be allowed to grab team
flags so that CTF is still possible, however.

The player will be notified that they need to be registered to grab flags.  The
message will appear at most once every 5 minutes.


Loading the plugin
--------------------------------------------------------------------------------

This plugin takes no optional arguments, so load it with:
  -loadplugin regFlag
