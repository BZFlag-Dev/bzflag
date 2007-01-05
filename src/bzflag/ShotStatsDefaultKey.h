/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	__SHOTSTATSDEFAULTKEY_H__
#define	__SHOTSTATSDEFAULTKEY_H__

/* common interface headers */
#include "BzfEvent.h"

/* local interface headers */
#include "HUDuiDefaultKey.h"

class ShotStatsDefaultKey : public HUDuiDefaultKey {
public:
  ShotStatsDefaultKey();
  ~ShotStatsDefaultKey();

  bool keyPress(const BzfKeyEvent&);
  bool keyRelease(const BzfKeyEvent&);
  static ShotStatsDefaultKey* getInstance();

private:
  static ShotStatsDefaultKey instance;
};


#endif /* __SHOTSTATSDEFAULTKEY_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
