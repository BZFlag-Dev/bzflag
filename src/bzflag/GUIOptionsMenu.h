/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __GUIOPTIONSMENU_H__
#define __GUIOPTIONSMENU_H__

#include "common.h"

/* common interface headers */
#include "HUDDialog.h"

/* local interface headers */
#include "MenuDefaultKey.h"
#include "HUDuiDefaultKey.h"


/** this class provides options for setting the gui
 */
class GUIOptionsMenu : public HUDDialog
{
public:
    GUIOptionsMenu();
    ~GUIOptionsMenu();

    HUDuiDefaultKey* getDefaultKey()
    {
        return MenuDefaultKey::getInstance();
    }
    void execute();
    void resize(int width, int height);
    static void callback(HUDuiControl* w, const void* data);

    static const int maxRadarSize = 30;

private:
};


#endif /* __GUIOPTIONSMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
