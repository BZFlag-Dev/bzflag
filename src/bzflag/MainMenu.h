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

#ifndef	__MAINMENU_H__
#define	__MAINMENU_H__

#include "common.h"

/* common interface headers */
#include "OpenGLTexFont.h"

/* local interface headers */
#include "HUDDialog.h"
#include "HUDui.h"
#include "JoinMenu.h"
#include "OptionsMenu.h"
#include "QuitMenu.h"


/** MainMenu is the main menu
 */
class MainMenu : public HUDDialog {
public:
  MainMenu();
  ~MainMenu();

  HUDuiDefaultKey* getDefaultKey();
  void execute();
  void resize(int width, int height);

  static const OpenGLTexFont& getFont();

private:
  OpenGLTexFont	font;
  JoinMenu* joinMenu;
  OptionsMenu*	optionsMenu;
  QuitMenu* quitMenu;
  static OpenGLTexFont* mainFont;
};


#endif /* __MAINMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
