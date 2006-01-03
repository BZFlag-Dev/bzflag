/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __INPUTMENU_H__
#define __INPUTMENU_H__

#include "common.h"

/* common interface headers */
#include "HUDDialog.h"

/* local interface headers */
#include "MenuDefaultKey.h"
#include "KeyboardMapMenu.h"
#include "HUDuiControl.h"
#include "HUDuiList.h"
#include "HUDuiDefaultKey.h"


/** this class provides options for setting the gui
 */
class InputMenu : public HUDDialog {
public:
  InputMenu();
  ~InputMenu();

  HUDuiDefaultKey* getDefaultKey()
  {
    return MenuDefaultKey::getInstance();
  }
  void execute();
  void resize(int width, int height);
  static void callback(HUDuiControl* w, void* data);

  void fillJSOptions();

private:
  HUDuiControl*	   keyMapping;
  HUDuiList*	   activeInput;
  HUDuiList*	   jsx;
  HUDuiList*	   jsy;
  KeyboardMapMenu* keyboardMapMenu;
};


#endif /* __INPUTMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
