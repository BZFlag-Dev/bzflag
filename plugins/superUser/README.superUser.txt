BZFlag Server Plugin: superUser
================================================================================

The superUser plugin lets a server owner grant permissions to globally
authenticated users based on BZID so they don't have to set up specific
admin groups in the global space.


Loading the plugin
--------------------------------------------------------------------------------

You must specify a configuration file when loading this plugin.  Use the format:
  -loadplugin superUser,<configfilename>
For example:
  -loadplugin superUser,/path/to/my/superUser.cfg


Configuration
--------------------------------------------------------------------------------

The configuration file format is very simple.  Comments start with a # symbol,
and can be used to document your configuration file, such as documenting the
username you are giving rights to.

Before defining user rights, you must first create a line with "[Users]" to
start the section.  After that, you will specify the BZID, an equal sign, and a
comma separated list of permissions you wish to grant to the user.  You must
look up the user's BZID on the BZFlag forums.  Search for them on the member
list, and look at the numeric ID that is shown in the URL.

Here is a basic example configuration file:

# Plugin configuration
[Users]

  # JeffM
  48 = adminMessageReceive
  # Tim Riker
  1037 = setAll,kick,kill,shortban,adminMessageReceive
