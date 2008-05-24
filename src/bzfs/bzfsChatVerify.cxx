/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// bzflag global header
#include "bzfsChatVerify.h"

bool checkChatSpam(char* message, GameKeeper::Player* playerData, int t)
{
  PlayerInfo &player = playerData->player;
  const std::string &oldMsg = player.getLastMsg();
  float dt = (float)(TimeKeeper::getCurrent() - player.getLastMsgTime());

  // don't consider whitespace
  std::string newMsg = TextUtils::no_whitespace(message);

  // if it's first message, or enough time since last message - can't
  // be spam yet
  if (oldMsg.length() > 0 && dt < clOptions->msgTimer) {
    // might be spam, start doing comparisons
    // does it match the last message? (disregarding whitespace and case)
    if ((TextUtils::compare_nocase(newMsg, oldMsg) == 0) && (newMsg.length() > 0)) {
      if (newMsg[0] != '/') {
	player.incSpamWarns();
	sendMessage(ServerPlayer, t, "***Server Warning: Please do not spam.");

	// has this player already had his share of warnings?
	if (player.getSpamWarns() > clOptions->spamWarnMax
	    || clOptions->spamWarnMax == 0) {
	  sendMessage(ServerPlayer, t, "You were kicked because of spamming.");
	  logDebugMessage(2,"Kicking player %s [%d] for spamming too much: "
			  "2 messages sent within %fs after %d warnings\n",
			  player.getCallSign(), t, dt, player.getSpamWarns());
	  removePlayer(t, "spam", true);
	  return true;
	}
      }
    }
  }

  // record this message for next time
  player.setLastMsg(newMsg);
  return false;
}


/** check the message being sent for invalid characters.  long
 *  unreadable messages are indicative of denial-of-service and crash
 *  attempts.  remove the player immediately, only modified clients
 *  should be sending such garbage messages.
 *
 *  that said, more than one badChar should be allowed since there
 *  might be two "magic byte" characters being used for backwards
 *  compatibility client-side message processing (as was used for the
 *  /me command at one point).
 */
bool checkChatGarbage(char* message, GameKeeper::Player* playerData, int t)
{
  PlayerInfo &player = playerData->player;
  static const int tooLong = MaxPacketLen / 2;
  const int totalChars = strlen(message);

  /* if the message is very long and looks like junk, give the user
   * the boot.
   */
  if (totalChars > tooLong) {
    int badChars = 0;
    int i;

    /* tally up the junk */
    for (i=0; i < totalChars; i++) {
      if (!TextUtils::isPrintable(message[i])) {
	badChars++;
      }
    }

    /* even once is once too many since they may be attempting to
     * cause a crash, but allow a few anyways.
     */
    if (badChars > 5) {
      sendMessage(ServerPlayer, t, "You were kicked because of a garbage message.");
      logDebugMessage(2,"Kicking player %s [%d] for sending a garbage message: %d of %d non-printable chars\n",
		      player.getCallSign(), t, badChars, totalChars);
      removePlayer(t, "garbage");

      // they're only happy when it rains
      return true;
    }
  }

  // the world is not enough
  return false;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
