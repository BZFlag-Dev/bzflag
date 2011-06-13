/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __FONTOPTIONSMENU_H__
#define __FONTOPTIONSMENU_H__

#include "common.h"

/* common interface headers */
#include "HUDDialog.h"

/* local interface headers */
#include "HUDuiDefaultKey.h"
#include "MenuDefaultKey.h"


class LocalFontFace;


/** this class provides font options
 */
class FontOptionsMenu : public HUDDialog {
  public:
    FontOptionsMenu();
    ~FontOptionsMenu();

    HUDuiDefaultKey* getDefaultKey() {
      return MenuDefaultKey::getInstance();
    }
    void execute();
    void resize(int width, int height);
    static void callback(HUDuiControl* w, void* data);

    LocalFontFace* consoleFont;
    LocalFontFace* serifFont;
    LocalFontFace* sansSerifFont;

  private:
    static FontOptionsMenu* fontOptionsMenu;
};


#endif /* __FONTOPTIONSMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
