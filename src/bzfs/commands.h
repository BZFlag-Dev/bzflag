/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __COMMANDS_H__
#define __COMMANDS_H__


/** password command hook function
 *
 * /password command allows player to become operator
 */
void handlePasswordCmd(int t, const char *message);


/** set command hook function
 *
 * /set sets a world configuration variable that gets sent to all clients
 */
void handleSetCmd(int t, const char *message);


/** reset command hook function
 */
void handleResetCmd(int t, const char *message);


/** shutdownserver command hook function
 *
 * /shutdownserver terminates the server
 */
void handleShutdownserverCmd(int t, const char *);


/** superkill command hook function
 *
 * /superkill closes all player connections
 */
void handleSuperkillCmd(int t, const char *);


/** gameover command hook function
 *
 * /gameover command allows operator to end the game
 */
void handleGameoverCmd(int t, const char *message);


/** countdown command hook function
 *
 * /countdown command allows operator to end the game
 */
void handleCountdownCmd(int t, const char *message);


/** flag command hook function
 *
 * /flag command allows operator to control flags
 */
void handleFlagCmd(int t, const char *message);


/** kick command hook function
 *
 * /kick command allows operator to remove players
 */
void handleKickCmd(int t, const char *message);


/** unban command hook function
 *
 * /banlist command shows ips that are banned
 */
void handleBanlistCmd(int t, const char *);


/** unban command hook function
 *
 * /ban command allows operator to ban players based on ip
 * /ban <ip> [duration] ...
 * any text after duration is considered as the reason for banning.
 */
void handleBanCmd(int t, const char *message);


/** unban command hook function
 *
 * /unban command allows operator to remove ips from the banlist
 */
void handleUnbanCmd(int t, const char *message);


/** lagwarn command hook function
 *
 * /lagwarn - set maximum allowed lag
 */
void handleLagwarnCmd(int t, const char *message);


/** lagstats command hook function
 *
 * /lagstats gives simple statistics about players' lags
 */
void handleLagstatsCmd(int t, const char *message);


/** idlestats command hook function
 *
 * /idlestats gives a list of players' idle times
 */
void handleIdlestatsCmd(int t, const char *message);


/** flaghistory command hook function
 *
 * /flaghistory gives history of what flags player has carried
 */
void handleFlaghistoryCmd(int t, const char *message);


/** playerlist command hook function
 *
 * /playerlist dumps a list of players with IPs etc.
 */
void handlePlayerlistCmd(int t, const char *message);


/** report command hook function
 *
 * /report sends a message to the admin and/or stores it in a file
 */
void handleReportCmd(int t, const char *message);


/** help command hook function
 */
void handleHelpCmd(int t, const char *message);


/** identify command hook function
 */
void handleIdentifyCmd(int t, const char *message);


/** register command hook function
 */
void handleRegisterCmd(int t, const char *message);


/** ghost command hook function
 */
void handleGhostCmd(int t, const char *message);


/** deregister command hook function
 */
void handleDeregisterCmd(int t, const char *message);


/** setpass command hook function
 */
void handleSetpassCmd(int t, const char *message);


/** grouplist command hook function
 */
void handleGrouplistCmd(int t, const char *message);


/** showgroup command hook function
 */
void handleShowgroupCmd(int t, const char *message);


/** groupperms command hook function
 */
void handleGrouppermsCmd(int t, const char *message);


/** setgroup command hook function
 */
void handleSetgroupCmd(int i, const char *message);


/** removegroup command hook funciton
 */
void handleRemovegroupCmd(int i, const char *message);


/** reset command hook function
 */
void handleReloadCmd(int i, const char *message);


/** /poll command hook function
 */
void handlePollCmd(int t, const char *message);


/** /vote command hook function
 */
void handleVoteCmd(int t, const char *message);


/** /veto command hook function
 */
void handleVetoCmd(int t, const char *message);


#endif

/* ex: shiftwidth=2 tabstop=8
 * Local Variables: ***
 * mode:C++ ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 */

