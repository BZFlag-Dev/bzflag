========================================================================
    DYNAMIC LINK LIBRARY : serverControl Project Overview
========================================================================

This is the serverControl plugin. 

This plugin takes a single command line argument that specifies the
filename for the plugin configuration file.

An example configuration file is provided in 'plugins.cfg'

The purpose of this plugin is for ban file synchronization across
multiple BZFlag servers running on the same host and for controlled
server shutdown for queued server restarts.  The plugin will reload
the ban file or master ban file entries when it notices a modification
to the local file.

To use this effectively with the BZFlag masterban file you should copy
the banfile from the official server to a local file and only modify
the local master ban file the plugin looks at if the contents of the
ban file changes.

An example script (for Linux systems) to keep a local copy of the
masterban file is provided in check_masterbans.sh.

This plugin can shut down the BZFlag server on demand or everytime a
game ends.  See the supplied plugin.cfg configuration file for
details.

Plugin loading format:

  -loadplugin /path/to/serverControl,/path/to/plugins.cfg


