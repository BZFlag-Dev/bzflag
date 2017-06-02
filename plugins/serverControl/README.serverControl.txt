BZFlag Server Plugin: serverControl
================================================================================

The purpose of this plugin is to synchronize ban file reloads across multiple
servers running on the same host that all use the same ban file, and to queue
server restarts that will trigger when the server empties out.  The plugin will
monitor the ban file and masterban file for changes and automatically reload
the ban lists when it detects a change.

To use this effectively with the BZFlag masterban file you should copy the
banfile from the official server to a local file and only modify the local
master ban file the plugin looks at if the contents of the ban file changes.


Loading the plugin
--------------------------------------------------------------------------------

To load the plugin and specify a configuration file, use the format:
  -loadplugin serverControl,<configfilename>
For example:
  -loadplugin serverControl,/path/to/my/serverControl.cfg


Configuration
--------------------------------------------------------------------------------

The configuration file format resembles that of a Windows INI file.  Here is a
basic sample configuration file.  A more detailed file is provided with the
BZFlag source code.


# Sample configuration file (this is a comment)

[ServerControl]

  #
  # Ban File Section
  # ----------------
  # Ban files can be shared between multiple servers.  Specify the location for
  # the shared banfile. The plugin checks the modification time for the file
  # approximately every 3 seconds and reloads the ban file if it has changed.

  # This is the relative or absolute location of the main ban file.
  BanFile = db/banfile

  # Message to display when bans are reloaded.
  BanReloadMessage = Bans updated

  # This is the relative or absolute location of the local copy of the
  # masterbans file.
  MasterBanFile = db/master-bans.txt

  # Message to display when the master ban list is reloaded.
  MasterBanReloadMessage = Master bans updated

  #
  # Server Restart Section
  # ----------------------
  #
  # If you run your BZFlag server in a server loop from a shell script you can
  # force the server to exit so the server can be reloaded by the controlling
  # shell script.

  # This defines a file to watch that allows shutting down this server instance
  # when the server empties out.  Once this triggers, this file is removed, so
  # it only shuts the server down once.
  ResetServerOnceFile = reset-server-once

  # This works the same as the ResetServerOnceFile, except it doesn't remove the
  # file when it triggers.  So, this would allow you to make the server restart
  # every time it empties out.
  ResetServerAlwaysFile = reset-server-always

  # This option will cause the plugin to ignore observers, allowing the server
  # to shut down when all non-observers leave the server.
  #IgnoreObservers = true
