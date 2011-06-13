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

#ifndef __TEXTOPTIONSMENU_H__
#define __TEXTOPTIONSMENU_H__

#include "common.h"

/* common interface headers */
#include "HUDDialog.h"

/* local interface headers */
#include "HUDuiDefaultKey.h"
#include "MenuDefaultKey.h"


class FontOptionsMenu;


/** this class provides text related options
 */
class TextOptionsMenu : public HUDDialog {
  public:
    TextOptionsMenu();
    ~TextOptionsMenu();

    HUDuiDefaultKey* getDefaultKey() {
      return MenuDefaultKey::getInstance();
    }
    void execute();
    void resize(int width, int height);
    static void callback(HUDuiControl* w, void* data);

  private:
    HUDuiControl* fontOptions;
    FontOptionsMenu* fontMenu;
};


#endif /* __TEXTOPTIONSMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
