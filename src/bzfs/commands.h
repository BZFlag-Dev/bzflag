/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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

// implementation-specific bzflag headers
#include "GameKeeper.h"

/// /serverquery command
void handleServerQueryCmd(GameKeeper::Player *playerData, const char *);

/// /uptime command prints current server running time.
void handleUptimeCmd(GameKeeper::Player *playerData, const char *);

/// /part <message> leaves game with a goodbye message
void handlePartCmd(GameKeeper::Player *playerData, const char *message);

/// /quit <message> quits game with a goodbye message
void handleQuitCmd(GameKeeper::Player *playerData, const char *message);

/// /me command allows player to express an action
void handleMeCmd(GameKeeper::Player *playerData, const char *message);

/// /msg command allows player to directly communicate to other players
void handleMsgCmd(GameKeeper::Player *playerData, const char *message);

/// /password command allows player to become operator
void handlePasswordCmd(GameKeeper::Player *playerData, const char *message);

/// /set command sets a world configuration variable that gets sent to all clients
void handleSetCmd(GameKeeper::Player *playerData, const char *message);

/// /reset command
void handleResetCmd(GameKeeper::Player *playerData, const char *message);

/// /shutdownserver command terminates the server
void handleShutdownserverCmd(GameKeeper::Player *playerData, const char *);

/// /superkill command closes all player connections
void handleSuperkillCmd(GameKeeper::Player *playerData, const char *);

/// /gameover command allows operator to end the game
void handleGameoverCmd(GameKeeper::Player *playerData, const char *message);

/// /countdown command allows operator to end the game
void handleCountdownCmd(GameKeeper::Player *playerData, const char *message);

/// /flag command allows operator to control flags
void handleFlagCmd(GameKeeper::Player *playerData, const char *message);

/// /kick command allows operator to remove players
void handleKickCmd(GameKeeper::Player *playerData, const char *message);

/// /banlist command shows ips that are banned
void handleBanlistCmd(GameKeeper::Player *playerData, const char *);

/// /hostbanlist command shows ips that are banned
void handleHostBanlistCmd(GameKeeper::Player *playerData, const char *);


/** /ban command allows operator to ban players based on ip
 * /ban <ip> [duration] ...
 * any text after duration is considered as the reason for banning.
 */
void handleBanCmd(GameKeeper::Player *playerData, const char *message);


/** /hostban command allows operator to ban players based on hostname
 * /hostban <hostpat> [duration] ...
 * any text after duration is considered as the reason for banning.
 * <hostpat> may contain *'s as wildcards
 */
void handleHostBanCmd(GameKeeper::Player *playerData, const char *message);


/// /unban command allows operator to remove ips from the banlist
void handleUnbanCmd(GameKeeper::Player *playerData, const char *message);

/// /hostunban command allows operator to remove host patterns from the banlist
void handleHostUnbanCmd(GameKeeper::Player *playerData, const char *message);

/// /lagwarn - set maximum allowed lag
void handleLagwarnCmd(GameKeeper::Player *playerData, const char *message);

/// /lagstats gives simple statistics about players' lags
void handleLagstatsCmd(GameKeeper::Player *playerData, const char * /*message*/);

/// /date && /time responds with the current server time, in form of DAY MON DATE TIME YEAR
void handleDateCmd(GameKeeper::Player *playerData, const char *message);

/// /idlestats gives a list of players' idle times
void handleIdlestatsCmd(GameKeeper::Player *playerData, const char *message);

/// /flaghistory gives history of what flags player has carried
void handleFlaghistoryCmd(GameKeeper::Player *playerData, const char *message);

/// /playerlist dumps a list of players with IPs etc.
void handlePlayerlistCmd(GameKeeper::Player *playerData, const char *message);

/// /report sends a message to the admin and/or stores it in a file
void handleReportCmd(GameKeeper::Player *playerData, const char *message);

/// /help command
void handleHelpCmd(GameKeeper::Player *playerData, const char *message);

/// /identify command
void handleIdentifyCmd(GameKeeper::Player *playerData, const char *message);

/// /register command
void handleRegisterCmd(GameKeeper::Player *playerData, const char *message);

/// /ghost command
void handleGhostCmd(GameKeeper::Player *playerData, const char *message);

/// /deregister command
void handleDeregisterCmd(GameKeeper::Player *playerData, const char *message);

/// /setpass command
void handleSetpassCmd(GameKeeper::Player *playerData, const char *message);

/// /grouplist command
void handleGrouplistCmd(GameKeeper::Player *playerData, const char *message);

/// /showgroup command
void handleShowgroupCmd(GameKeeper::Player *playerData, const char *message);

/// /groupperms command
void handleGrouppermsCmd(GameKeeper::Player *playerData, const char *message);

/// /setgroup command
void handleSetgroupCmd(GameKeeper::Player *playerData, const char *message);

/// /removegroup command hook funciton
void handleRemovegroupCmd(GameKeeper::Player *playerData, const char *message);

/// init groups from file or compiled defaults
void initGroups();

/// reset command
void handleReloadCmd(GameKeeper::Player *playerData, const char *message);

/// /poll command
void handlePollCmd(GameKeeper::Player *playerData, const char *message);

/// /vote command
void handleVoteCmd(GameKeeper::Player *playerData, const char *message);

/// /veto command
void handleVetoCmd(GameKeeper::Player *playerData, const char *message);

/// /viewreports - view report file
void handleViewReportsCmd(GameKeeper::Player *playerData, const char *message);

/// /clientquery returns all attached clients' version strings
void handleClientqueryCmd(GameKeeper::Player *playerData, const char *);


/** /record command
 *
 *  /record start	       # start buffering
 *  /record stop		# stop buffering (or saving to file)
 *  /record size <Mbytes>       # set the buffer size, and truncate
 *  /record rate <secs>	 # set the state capture rate
 *  /record stats	       # display buffer time and memory information
 *  /record file [filename]     # begin capturing straight to file, flush buffer
 *  /record save [filename]     # save buffer to file (or default filename)
 */
void handleRecordCmd(GameKeeper::Player *playerData, const char *);


/** /replay command
 *
 *  /replay list		# list available replay files
 *  /replay load [filename]     # set the replay file (or load the default)
 *  /replay play		# began playing
 *  /replay skip <secs>	 # fast foward or rewind in time
 */
void handleReplayCmd(GameKeeper::Player *playerData, const char *);


/** /masterban command
 *
 * /masterban flush	     # remove all master ban entries from this server
 * /masterban reload	    # reread and reload all master ban entries
 * /masterban list	      # output a list of who is banned
 */
void handleMasterBanCmd(GameKeeper::Player *playerData, const char *message);

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
