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

#include <string>
#include <vector>

#include "CronJob.h"
#include "bzfsAPI.h"

// -- Event handler class
class CronPlayer : public bz_ServerSidePlayerHandler {
public:
  CronPlayer(); // c'tor
  void added(int player);
  void playerRejected(bz_eRejectCodes code, const char *reason);
  void sendCommand(std::string message); // expose inherited protected member sendChatMessage
};

class CronManager : public bz_EventHandler {
public:
  CronManager();
  ~CronManager();

  void process(bz_EventData *eventData);
  bool autoDelete(void) {return false;};

  bool connect();
  void sendMessage(std::string message);

  void addJob(CronJob &job) {jobs.push_back(job);};
  void clearJobs() {jobs.clear();};

  void setCrontab(std::string _crontab) {this->crontab = _crontab;};
  bool reload();
  void list(int playerID) const;

private:
  std::vector<CronJob> jobs;
  double lastTick;
  int lastMinute;
  std::string crontab;
  CronPlayer* player;
};

extern CronManager cron;
extern int debugLevel;
