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

#ifndef	__MENUDEFAULTKEY_H__
#define	__MENUDEFAULTKEY_H__

/* common interface headers */
#include "BzfEvent.h"

/* local interface headers */
#include "HUDuiDefaultKey.h"

class MenuDefaultKey : public HUDuiDefaultKey {
public:
  MenuDefaultKey();
  ~MenuDefaultKey();

  bool keyPress(const BzfKeyEvent&);
  bool keyRelease(const BzfKeyEvent&);

  static MenuDefaultKey* getInstance();

private:
  static MenuDefaultKey instance;
};


#endif /* __MENUDEFAULTKEY_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
