BZFlag Server Plugin: chathistory
================================================================================

The chathistory plugin logs all the chat text in memory (including team and
private chat).  It then allows an administrator to view the last several
messages that a player sent. It does not include who the message was set to,
however.


Loading the plugin
--------------------------------------------------------------------------------

By default, the plugin stores the last 50 lines of text for *each* player. If
this is enough, you can load the plugin without additional arguments.

  -loadplugin chathistory

If you wish to store more or less than 50 lines, you can provide that as an
argument when loading the plugin.  Note that the game server may kick you if you
request too much information, so storing more than 50 might be useful.

  -loadplugin chathistory,20

Do note that this plugin doesn't automatically remove chat histories from
players that leave the server, so a long running server can begin to use more
and more memory.  Running the /flushchat command mentioned below should free up
the memory that was used.


Server Commands
--------------------------------------------------------------------------------

To run either of the two commands, you must be an administrator on the server.

To show the last 8 lines of chat for a player, you can the 'last' command. If
the player's name has spaces, you must enclose their name in double quotes.
  /last 8 "some callsign here"

To clear the chat history for all players:
  /flushchat
