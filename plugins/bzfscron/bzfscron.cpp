/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// bzfscron plugin - cron-style timed command execution for bzfs
// see ReadMe.txt for usage notes.

#include "bzfscron.h"

#include <string>
#include <vector>
#include <fstream>
#include <math.h>

#include "croncommand.h"
#include "CronJob.h"
#include "bzfsAPI.h"
#include "TextUtils.h"

#define BCVERSION "bzfscron 1.0.0"

int debugLevel;
CronManager cron;

// -- Library entry points for bzfs

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load(const char* commandLine)
{
  // should have a filename on the command line.  try to open it.
  if (!commandLine) {
    bz_debugMessage(1, "bzfscron: no crontab specified");
    return 1;
  }
  cron.setCrontab(commandLine);
  if (!cron.reload())
    return 2;

  // we have a granularity of 1 minute but we want to run things ASAP in that minute
  // since we rely on "real time" and not "relative time". n+<=5 sec should not hurt.
  bz_setMaxWaitTime(5.0f);

  // register to receive ticks
  bz_registerEvent(bz_eTickEvent, &cron);

  // register /cron
  registerCronCommand();

  bz_debugMessage(4, BCVERSION ": plugin loaded");

  if (!cron.connect())
    bz_debugMessage(1, BCVERSION ": fake player could not connect!");

  bz_debugMessage(4, BCVERSION ": fake player connected");
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload(void)
{
  bz_removeEvent(bz_eTickEvent, &cron);
  bz_debugMessage(4, BCVERSION ": plugin unloaded");
  return 0;
}


// -- Utility functions

// calculate the day of the week (0 = sunday)
static int dow(int m, int d, int y) {
  // W = (k + floor(2.6m - 0.2) - 2C + Y + floor(Y/4) + floor(C/4)) mod 7

  m -= 2; // march is base month
  if (m < 1) {
    y -= 1;
    m += 12;
  }

  const int c = (int)((y - 50.0f) / 100.0f); // century

  // whew!
  return ((int)(d + floor(2.6f * m - 0.2f) - 2 * c + y + floor(y / 4.0f) + floor(c / 4.0f)) % 7);
}


// -- Event callbacks

CronManager::CronManager() : lastTick(0.0), lastMinute(-1), player(NULL) {
  // do nothing
}

CronManager::~CronManager() {
  if (player) {
    delete player;
    player = NULL;
  }
}

bool CronManager::reload(void) {
  // open the crontab
  std::ifstream input(crontab.c_str(), std::ios::in);
  if (input.peek() == EOF) { 
    bz_debugMessage(1, "bzfscron: crontab nonexistant or invalid");
    return false;
  }

  // clear current jobs
  clearJobs();

  // read in the crontab
  char buffer[1024];
  while (!input.eof() && !input.fail() && input.good()) {
    input.getline(buffer, sizeof(buffer));
    if (buffer[0] != '#') {
      CronJob newcron(buffer);
      addJob(newcron);
    }
  }
  return true;
}

void CronManager::list(int playerID) const {
  for (std::vector<CronJob>::const_iterator itr = jobs.begin(); itr != jobs.end(); ++itr) {
    bz_sendTextMessage(BZ_SERVER, playerID, TextUtils::replace_all(itr->displayJob(), "\t", " ").c_str());
  }
}

void CronManager::process(bz_EventData *eventData) {
  switch (eventData->eventType) {
  
  default: {
    // no, sir, we didn't ask for THIS!!
    bz_debugMessage(1, "bzfscron: received event with unrequested eventType!");
    return;
  }

  case bz_eTickEvent: {
    bz_TickEventData_V1* event = (bz_TickEventData_V1*)eventData;

    // ignore ticks that are less than 5 seconds apart
    if (lastTick + 4.95f > event->eventTime)
      return;
    lastTick = event->eventTime;
    bz_debugMessage(4, "bzfscron: tick!");

    // ensure that the minute has changed
    bz_localTime t;
    bz_getLocaltime(&t);
    if (t.minute == lastMinute)
      return;
    lastMinute = t.minute;
    bz_debugMessage(4, "bzfscron: minute change");

    // make sure we have a valid player
    if (!player || player->playerID < 0) return;

    // iterate through all the jobs.  if they match the current minute, run them.
    std::vector<CronJob>::iterator itr;
    for (itr = jobs.begin(); itr != jobs.end(); ++itr)
      if (itr->matches(t.minute, t.hour, t.day, t.month, dow(t.month, t.day, t.year))) {
	bz_debugMessage(4, TextUtils::format("bzfscron: job matched at %d-%d-%d %d:%d - \"%s\"", t.year, t.month, t.day, t.hour, t.minute, itr->getCommand().c_str()).c_str());
	player->sendCommand(itr->getCommand());
      }

    break;
  }
  }
}

bool CronManager::connect()
{
  // Create fake player
  player = new CronPlayer();
  return (bz_addServerSidePlayer(player) >= 0);
}

CronPlayer::CronPlayer()
{
  playerID = -1;
}

void CronPlayer::added(int player)
{
  if (player == playerID) { // oh look, it's ME!
    // set my information
    setPlayerData("bzfscron", "" /*email*/, "" /*token*/, BCVERSION, eObservers);

    // I think I'll make myself admin, so I can run all sorts of fun commands
    if (!bz_setPlayerOperator(playerID))
      bz_debugMessage(1, "bzfscron: unable to make myself an administrator");

    // But nobody needs to know I'm admin, 'cause I can't do anything unless crontab told me to
    bz_grantPerm(playerID, bz_perm_hideAdmin);
  }
}

void CronPlayer::playerRejected(bz_eRejectCodes code, const char *reason)
{
  std::string temp = TextUtils::format("Player rejected (reason: %s)", reason);
  bz_debugMessage(1, temp.c_str());
}

void CronPlayer::sendCommand(std::string message)
{
  sendChatMessage(message.c_str());
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

