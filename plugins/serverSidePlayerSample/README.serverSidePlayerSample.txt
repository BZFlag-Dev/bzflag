BZFlag Server Plugin: serverSidePlayerSample
================================================================================

This plugin joins a single server-side player as observer.  It listens for
events such as spawning and shots being fired and sends a message for each of
these events.  It also echos back private messages that are sent directly to it.

So, it's not a plugin you'd want to run on your server as-is.  It's merely an
example of what the server-side player (SSP) API is currently capable of.
Eventually, we may have the ability for SSP's to play the game, which would
allow plugins to define their AI.


Loading the plugin
--------------------------------------------------------------------------------

This plugin takes no optional arguments, so load it with:
  -loadplugin serverSidePlayerSample
