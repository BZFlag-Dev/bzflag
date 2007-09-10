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

#ifndef	__LOGINMENU_H__
#define	__LOGINMENU_H__

#include "common.h"

/* local interface headers */
#include "HUDDialog.h"
#include "HUDuiLabel.h"
#include "HUDuiTypeIn.h"

class LoginMenu : public HUDDialog {
 public:
  LoginMenu();
  ~LoginMenu();

  HUDuiDefaultKey  *getDefaultKey();

  void		    execute();
  void		    resize(int width, int height);

 private:
  HUDuiTypeIn*	    username;
  HUDuiTypeIn*	    password;
  HUDuiLabel*	    login;
};

#endif /* __LOGINMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
