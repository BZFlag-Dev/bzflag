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

#ifndef	__OPTIONSMENU_H__
#define	__OPTIONSMENU_H__

/* local interface headers */
#include "HUDDialog.h"
#include "MenuDefaultKey.h"
#include "HUDui.h"
#include "FormatMenu.h"
#include "GUIOptionsMenu.h"
#include "SaveWorldMenu.h"
#include "InputMenu.h"


class OptionsMenu : public HUDDialog {
public:
  OptionsMenu();
  ~OptionsMenu();

  HUDuiDefaultKey* getDefaultKey()
  { 
    return MenuDefaultKey::getInstance(); 
  }
  void		execute();
  void		resize(int width, int height);

  static void	callback(HUDuiControl* w, void* data);
  static int	gammaToIndex(float);
  static float	indexToGamma(int);

private:
  HUDuiControl*	videoFormat;
  HUDuiControl*	guiOptions;
  HUDuiControl*	clearCache;
  HUDuiControl*	saveWorld;
  HUDuiControl*	inputSetting;
  FormatMenu*		formatMenu;
  GUIOptionsMenu*	guiOptionsMenu;
  SaveWorldMenu*	saveWorldMenu;
  InputMenu*            inputMenu;
};


#endif /* __OPTIONSMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
