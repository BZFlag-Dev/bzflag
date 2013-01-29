========================================================================
    DYNAMIC LINK LIBRARY : chathistory Project Overview
========================================================================

The chathistory plugin logs all the chat text for all players for 
administrators to review.

When chathistory is loaded it will begin logging every line for every 
player that is in the game. It will log these lines in internal memory
for as long as a player is on the server, or until a preset line limit
is reached. The default line limit, which is 1000 lines, can be changed
by passing in a new limit when the plug-in is loaded as a parameter.

The plug-in installs two custom slash commands for administrators to use
to review the stored chat logs.

/last <NUMBER OF LINES> <CALLSIGN>
is used to list the last <NUMBER OF LINES> of chat text for the
specified callsign.

/flushchat manually flushes all stored chat. 
