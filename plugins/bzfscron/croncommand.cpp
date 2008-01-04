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

#include "croncommand.h"

#include "bzfsAPI.h"

#include "bzfscron.h"
#include "CronJob.h"

CronCommand cronCommand;

bool CronCommand::handle(int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList *params) {
  if (!bz_hasPerm(playerID, "BZFSCRON")) {
    bz_sendTextMessage(BZ_SERVER, playerID, "bzfscron: you do not have permission to run the /cron command.");
    return true;
  }
  if (!params || (params->size() == 0) || !(*params)[0].c_str()) {
    bz_sendTextMessage(BZ_SERVER, playerID, "usage: /cron [list|reload]");
  }
  else if (strcasecmp((*params)[0].c_str(), "reload") == 0) {
    if (cron.reload())
      bz_sendTextMessage(BZ_SERVER, playerID, "bzfscron: reload succeeded.");
    else
      bz_sendTextMessage(BZ_SERVER, playerID, "bzfscron: reload failed.");
  }
  else if (strcasecmp((*params)[0].c_str(), "list") == 0) {
    cron.list(playerID);
  }
  return true;
}

int registerCronCommand(void) {
  bz_registerCustomSlashCommand("cron", &cronCommand);
  return 0;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
