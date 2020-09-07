BZFlag Server Plugin: genocide
================================================================================

This is a simple sample plugin that implements the legacy "genocide" flag from
previous versions.  Shots from this flag will kill all members of the team it
strikes, except rogue by default. In addition, rogueAsTeam or disableSuicide
exist as options, with rogueAsTeam enabling genocide to affect rogues and
disableSuicide preventing selfkills from triggering genocide effect.


Loading the plugin
--------------------------------------------------------------------------------

By default, simply genocide is enabled.  If that is acceptable, 
you can just load the plugin without additional arguments.

  -loadplugin genocide

To enable rogueAsTeam or disableSuicide, specify them as arguments.

  -loadplugin genocide,rogueAsTeam,disableSuicide
