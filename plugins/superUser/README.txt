========================================================================
    DYNAMIC LINK LIBRARY : superUser Project Overview
========================================================================

This is the superUser plugin.

The superUser plugin lets a server owner grant permissions to globally 
authenticated users based on BZID so they don't have to set up specific 
admin groups in the global space.

This plugin takes a single command line argument that specifies the
filename for its configuration file.

An example configuration file is provided in 'plugins.cfg'

Plugin loading format:

  -loadplugin superUser,/path/to/plugins.cfg
