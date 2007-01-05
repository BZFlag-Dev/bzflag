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

#ifndef	__OPTIONSMENU_H__
#define	__OPTIONSMENU_H__

#include "common.h"

/* local interface headers */
#include "HUDDialog.h"
#include "MenuDefaultKey.h"
#include "HUDuiDefaultKey.h"
#include "HUDuiControl.h"
#include "GUIOptionsMenu.h"
#include "EffectsMenu.h"
#include "CacheMenu.h"
#include "SaveWorldMenu.h"
#include "InputMenu.h"
#include "AudioMenu.h"
#include "DisplayMenu.h"

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

private:
  HUDuiControl*	guiOptions;
  HUDuiControl*	effectsOptions;
  HUDuiControl*	cacheOptions;
  HUDuiControl*	saveWorld;
  HUDuiControl* saveSettings;
  HUDuiControl*	inputSetting;
  HUDuiControl*	audioSetting;
  HUDuiControl*	displaySetting;
  GUIOptionsMenu*	guiOptionsMenu;
  EffectsMenu*		effectsMenu;
  CacheMenu*		cacheMenu;
  SaveWorldMenu*	saveWorldMenu;
  InputMenu*		inputMenu;
  AudioMenu*		audioMenu;
  DisplayMenu*		displayMenu;
};


#endif /* __OPTIONSMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
