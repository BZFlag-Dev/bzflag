BZFlag Server Plugin: changeTeam
================================================================================

This is a quick and dirty plugin to test dynamic team changing in BZFlag 2.5.
It's mostly useful until an in-game interface is designed for changing the
team.


Loading the plugin
--------------------------------------------------------------------------------

This plugin takes no optional arguments, so load it with:
  -loadplugin changeTeam


Server Commands
--------------------------------------------------------------------------------

Describe the server commands, if any, that this plugin exposes.

/team {observer|rogue|red|green|blue|purple|hunter|rabbit}
  Changes (or queues a change) to the player's team

