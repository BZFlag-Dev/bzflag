/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __CACHEMENU_H__
#define __CACHEMENU_H__

#include "common.h"

/* common interface headers */
#include "HUDDialog.h"

/* local interface headers */
#include "MenuDefaultKey.h"
#include "HUDuiDefaultKey.h"
#include "HUDuiTypeIn.h"


/** this class provides options for setting the gui
 */
class CacheMenu : public HUDDialog {
  public:
    CacheMenu();
    ~CacheMenu();

    HUDuiDefaultKey* getDefaultKey()
    {
      return MenuDefaultKey::getInstance();
    }
    void execute();
    void resize(int width, int height);
    static void callback(HUDuiControl* w, void* data);

  private:
    HUDuiTypeIn* cacheSize;
    HUDuiControl* updateDownloadCache;
    HUDuiControl* clearDownloadCache;
    HUDuiControl* clearServerListCache;
};


#endif /* __CACHEMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
