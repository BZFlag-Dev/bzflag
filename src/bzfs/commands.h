/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
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


/* uptime command hook function
 *
 * /uptime prints current server running time.
 */

void handleUptimeCmd(GameKeeper::Player *playerData, const char *);


/** part command hook function
 *
 * /part <message> quits game with a goodbye message
 */
void handlePartCmd(GameKeeper::Player *playerData, const char *message);


/** me command hook function
 *
 * /me command allows player to express an action
 */
void handleMeCmd(GameKeeper::Player *playerData, const char *message);


/** msg command hook function
 *
 * /msg command allows player to directly communicate to other players
 */
void handleMsgCmd(GameKeeper::Player *playerData, const char *message);


/** password command hook function
 *
 * /password command allows player to become operator
 */
void handlePasswordCmd(GameKeeper::Player *playerData, const char *message);


/** set command hook function
 *
 * /set sets a world configuration variable that gets sent to all clients
 */
void handleSetCmd(GameKeeper::Player *playerData, const char *message);


/** reset command hook function
 */
void handleResetCmd(GameKeeper::Player *playerData, const char *message);


/** shutdownserver command hook function
 *
 * /shutdownserver terminates the server
 */
void handleShutdownserverCmd(GameKeeper::Player *playerData, const char *);


/** superkill command hook function
 *
 * /superkill closes all player connections
 */
void handleSuperkillCmd(GameKeeper::Player *playerData, const char *);


/** gameover command hook function
 *
 * /gameover command allows operator to end the game
 */
void handleGameoverCmd(GameKeeper::Player *playerData, const char *message);


/** countdown command hook function
 *
 * /countdown command allows operator to end the game
 */
void handleCountdownCmd(GameKeeper::Player *playerData, const char *message);


/** flag command hook function
 *
 * /flag command allows operator to control flags
 */
void handleFlagCmd(GameKeeper::Player *playerData, const char *message);


/** kick command hook function
 *
 * /kick command allows operator to remove players
 */
void handleKickCmd(GameKeeper::Player *playerData, const char *message);


/** banlist command hook function
 *
 * /banlist command shows ips that are banned
 */
void handleBanlistCmd(GameKeeper::Player *playerData, const char *);


/** hostbanlist command hook function
 *
 * /hostbanlist command shows ips that are banned
 */
void handleHostBanlistCmd(GameKeeper::Player *playerData, const char *);


/** ban command hook function
 *
 * /ban command allows operator to ban players based on ip
 * /ban <ip> [duration] ...
 * any text after duration is considered as the reason for banning.
 */
void handleBanCmd(GameKeeper::Player *playerData, const char *message);


/** hostban command hook function
 *
 * /hostban command allows operator to ban players based on hostname
 * /hostban <hostpat> [duration] ...
 * any text after duration is considered as the reason for banning.
 * <hostpat> may contain *'s as wildcards
 */
void handleHostBanCmd(GameKeeper::Player *playerData, const char *message);


/** unban command hook function
 *
 * /unban command allows operator to remove ips from the banlist
 */
void handleUnbanCmd(GameKeeper::Player *playerData, const char *message);

/** hostunban command hook function
 *
 * /hostunban command allows operator to remove host patterns from the banlist
 */
void handleHostUnbanCmd(GameKeeper::Player *playerData, const char *message);


/** lagwarn command hook function
 *
 * /lagwarn - set maximum allowed lag
 */
void handleLagwarnCmd(GameKeeper::Player *playerData, const char *message);


/** lagstats command hook function
 *
 * /lagstats gives simple statistics about players' lags
 */
void handleLagstatsCmd(GameKeeper::Player *playerData, const char * /*message*/);


/** date/time command hook function
 *
 * /date && /time responds with the current server time, in form of DAY MON DATE TIME YEAR
 */
void handleDateCmd(GameKeeper::Player *playerData, const char *message);

/** idlestats command hook function
 *
 * /idlestats gives a list of players' idle times
 */
void handleIdlestatsCmd(GameKeeper::Player *playerData, const char *message);


/** flaghistory command hook function
 *
 * /flaghistory gives history of what flags player has carried
 */
void handleFlaghistoryCmd(GameKeeper::Player *playerData, const char *message);


/** playerlist command hook function
 *
 * /playerlist dumps a list of players with IPs etc.
 */
void handlePlayerlistCmd(GameKeeper::Player *playerData, const char *message);


/** report command hook function
 *
 * /report sends a message to the admin and/or stores it in a file
 */
void handleReportCmd(GameKeeper::Player *playerData, const char *message);


/** help command hook function
 */
void handleHelpCmd(GameKeeper::Player *playerData, const char *message);


/** identify command hook function
 */
void handleIdentifyCmd(GameKeeper::Player *playerData, const char *message);


/** register command hook function
 */
void handleRegisterCmd(GameKeeper::Player *playerData, const char *message);


/** ghost command hook function
 */
void handleGhostCmd(GameKeeper::Player *playerData, const char *message);


/** deregister command hook function
 */
void handleDeregisterCmd(GameKeeper::Player *playerData, const char *message);


/** setpass command hook function
 */
void handleSetpassCmd(GameKeeper::Player *playerData, const char *message);


/** grouplist command hook function
 */
void handleGrouplistCmd(GameKeeper::Player *playerData, const char *message);


/** showgroup command hook function
 */
void handleShowgroupCmd(GameKeeper::Player *playerData, const char *message);


/** groupperms command hook function
 */
void handleGrouppermsCmd(GameKeeper::Player *playerData, const char *message);


/** setgroup command hook function
 */
void handleSetgroupCmd(GameKeeper::Player *playerData, const char *message);


/** removegroup command hook funciton
 */
void handleRemovegroupCmd(GameKeeper::Player *playerData, const char *message);


/** reset command hook function
 */
void handleReloadCmd(GameKeeper::Player *playerData, const char *message);


/** /poll command hook function
 */
void handlePollCmd(GameKeeper::Player *playerData, const char *message);


/** /vote command hook function
 */
void handleVoteCmd(GameKeeper::Player *playerData, const char *message);


/** /veto command hook function
 */
void handleVetoCmd(GameKeeper::Player *playerData, const char *message);

/** viewreports command hook function
 *
 *  /viewreports - view report file
 */
void handleViewReportsCmd(GameKeeper::Player *playerData, const char *message);

/** /clientquery command hook function
 *
 *  /clientquery returns all attached clients' version strings
 */
void handleClientqueryCmd(GameKeeper::Player *playerData, const char *);


/** /record command hook function
 *
 *  /record start               # start buffering
 *  /record stop                # stop buffering (or saving to file)
 *  /record size <Mbytes>       # set the buffer size, and truncate
 *  /record rate <secs>         # set the state capture rate
 *  /record stats               # display buffer time and memory information
 *  /record file [filename]     # begin capturing straight to file, flush buffer
 *  /record save [filename]     # save buffer to file (or default filename)
 */
void handleRecordCmd(GameKeeper::Player *playerData, const char *);


/** /replay command hook function
 *
 *  /replay list                # list available replay files
 *  /replay load [filename]     # set the replay file (or load the default)
 *  /replay play                # began playing
 *  /replay skip <secs>         # fast foward or rewind in time
 */
void handleReplayCmd(GameKeeper::Player *playerData, const char *);


/** /masterban command hook function
 *
 * /masterban flush             # remove all master ban entries from this server
 * /masterban reload            # reread and reload all master ban entries
 * /masterban list              # output a list of who is banned
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
