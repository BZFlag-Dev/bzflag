/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __SERVERCOMMANDKEY_H__
#define __SERVERCOMMANDKEY_H__

#include <string>
#include "HUDui.h"
#include "Address.h"

class ServerCommandKey : public HUDuiDefaultKey {
public:
  ServerCommandKey();
  bool		keyPress(const BzfKeyEvent&);
  bool		keyRelease(const BzfKeyEvent&);
  void		init();
  void		adminInit();
  void		nonAdminInit();
private:
  std::string		makePattern(const InAddr& address);
  void		updatePrompt();

private:
  enum Mode {
    Kick,
    BanIp,
    Ban1,
    Ban2,
    Ban3,
    Unban,
    Showgroup,
    Setgroup,
    Removegroup,
    Ghost,
    Banlist,
    Playerlist,
    FlagReset,
    FlagUnusedReset,
    FlagUp,
    FlagShow,
    FlagHistory,
    IdleStats,
    LagStats,
    Report,
    LagWarn,
    GameOver,
    CountDown,
    SuperKill,
    Shutdown,
    Register,
    Identify,
    Setpass,
    Grouplist,
    Groupperms,
    Vote,
    Poll,
    Veto,
    Password, // leave this as the last item
  };

  Mode mode;
  int startIndex;
  const int numModes;
  const int numNonAdminModes;
  static const Mode nonAdminModes[7];


};

#endif