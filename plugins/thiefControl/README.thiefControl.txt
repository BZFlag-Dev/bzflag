BZFlag Server Plugin: thiefControl
================================================================================

The thiefControl plugin prevents players from stealing flags from their
teammates.  It ignore rogues, since they don't have teammates.  On CTF, it
allows stealing the team flags for flag passing.  When the plugin prevents a
flag from being stolen, it also sends the would be thief a message and takes
away their flag.


Loading the plugin
--------------------------------------------------------------------------------

This plugin takes no optional arguments, so load it with:

  -loadplugin thiefControl
