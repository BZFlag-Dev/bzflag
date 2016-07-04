BZFlag Server Plugin: RogueGenocide
================================================================================

Normally, the genocide flag will not affect rogue players, since they are not a
true team.  This plugin changes that behavior so that rogue players are
affected.  This would also allow a rouge player to shoot another rogue with the
genocide flag and wipe out all rogues (including themself).  A rogue shooting
themself does not trigger the genocide effect unless the 'suicide' option is
passed when loading the plugin.


Loading the plugin
--------------------------------------------------------------------------------

To load the plugin with default settings, use:
  -loadplugin RogueGenocide

To allow a rogue to shoot themself to trigger a genocide, use:
  -loadplugin RogueGenocide,suicide
