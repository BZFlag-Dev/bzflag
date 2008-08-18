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

#include <common.h>
#include "ConfigMgr.h"
#include "Log.h"
#include "NetHandler.h"
#include "RSA.h"
#include "UserStorage.h"
#include "TimeKeeper.h"
#include "EventHandler.h"
#include <conio.h>

int main()
{
  sLog.outLog("BZAuthd starting..");

  sConfig.initialize();

  if(!sNetHandler.initialize())
    return 1;

  if(!sRSAManager.initialize())
    return 1;

  if(!sRSAManager.generateKeyPair())
    return 1;

  if(!sUserStore.initialize())
    return 1;

  /* main loop */
  while(!kbhit()) {
    TimeKeeper::setTick();
    sNetHandler.update();
    sEventHandler.update();
  }

  return 0;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8