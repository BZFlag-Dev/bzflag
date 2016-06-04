BZFlag Server Plugin: fastmap
================================================================================

The world cache can be sent using either the game protocol or via HTTP.  The
game protocol method is much slower, especially when there is more latency to
the game server.  However, serving a cache file via HTTP normally required
manually generating and copying the file, and was prone to the file becoming
outdated.  The fastmap plugin uses the HTTP server built into bzfs to
automatically serve an up-to-date cache file without any complicated setup.


Loading the plugin
--------------------------------------------------------------------------------

This plugin takes no optional arguments, so load it with:

  -loadplugin fastmap
