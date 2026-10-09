/* bzflag
 * Copyright (c) 1993-2025 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __HELPMENU_H__
#define __HELPMENU_H__

// Inherits from
#include "MenuDefaultKey.h"

/* common interface headers */
#include "BzfEvent.h"

/* local interface headers */
#include "HUDDialog.h"
#include "HUDuiDefaultKey.h"
#include "HUDuiControl.h"


class MenuDefaultKey;

class HelpMenuDefaultKey final : public MenuDefaultKey
{
public:
    HelpMenuDefaultKey() {}
    ~HelpMenuDefaultKey() {}

    bool keyPress(const BzfKeyEvent&) override;
    bool keyRelease(const BzfKeyEvent&) override;
};


class HelpMenu : public HUDDialog
{
public:
    HelpMenu(const char* title = "Help");
    ~HelpMenu()
    {
    }

    HUDuiDefaultKey* getDefaultKey() override final
    {
        return &defaultKey;
    }
    void execute() override final
    {
    }
    void resize(int width, int height) override;

    static HelpMenu* getHelpMenu(HUDDialog* = NULL, bool next = true);
    static void done();

protected:
    HUDuiControl* createLabel(const char* string,
                              const char* label = NULL);
    virtual float getLeftSide(int width, int height);

private:
    HelpMenuDefaultKey defaultKey;
    static HelpMenu** helpMenus;
};


#endif /* __HELPMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
