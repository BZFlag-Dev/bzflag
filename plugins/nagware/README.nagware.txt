BZFlag Server Plugin: nagware
================================================================================

Original author: menotume

The nagware plugin is designed to encourage players to register their callsign
at forums.bzflag.org.  It can send text messages to unregistered players at
defined intervals, as well as automatically kick unregistered players.


Loading the plugin
--------------------------------------------------------------------------------

The plugin requires that the path to the nagware configuration file is provided:

  -loadplugin nagware,/path/to/your/nagware.cfg


Configuration
--------------------------------------------------------------------------------

# Sample configuration file for the nagware plugin.

# permission tag to use to grant "/nag" command permissions
# defaults to "NAG" if not specified.
permname = say

# Apply logic to observers ?  (yes/no)
# default is no
kickobs = yes

# minplayers: There must be this many players before anybody is
#    kicked, but the warnings will still happen.
# default is 1
minplayers  = 2

# Include observers in minplayer count ?  (yes/no)
# default is yes
countobs = yes

# messagesuffix: Message to be added to the end of all other defined messages.
messagesuffix = \nRegister at https://forums.bzflag.org/ and use your login in the bzflag connect screen.

# Message: Messages to be sent to players at specified times.
#   Format is:  TIME,[REPEAT] MESSAGE
#       *) Time and repeat are in minutes, whole numbers only (0 is allowed)
#       *) If repeat is specified, the message will be repeated every [REPEAT] minutes until the next
#          message is applied (if any), or the player is kicked.
message = 1 This server requires global registration.
message = 2 Dieser Server erfordert globale Registrierung.\nCe serveur exige l'enregistrement global\nEste camarero requiere matricula global
message = 3,1 You will be kicked from this server shortly, please register and come back!
message = 6,2 You are now marked to be kicked randomly.

# Same format as above, except 'repeat' is not used.
# If not defined, no players will be kicked.
kickmessage = 7 You have been kicked because you are not registered.


Server Commands
--------------------------------------------------------------------------------

The following commands are available to privileged players.

Enable sending messages and kicking players.  Enabled by default.
  /nag on

Disable sending messages and kicking players.  Note that it automatically
disables these functions during a match.
  /nag off

Display the current nagware configuration.
  /nag config

Reload the configuration file.
  /nag reload

Show all unverified players and how long they have been connected
  /nag list
