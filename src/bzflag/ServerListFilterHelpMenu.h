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

#ifndef __SERVERLISTFILTERHELPMENU_H__
#define __SERVERLISTFILTERHELPMENU_H__

// Inherits from
#include "MenuDefaultKey.h"
#include "HUDDialog.h"

/* common interface headers */
#include "BzfEvent.h"

/* local interface headers */
#include "HUDuiDefaultKey.h"
#include "HUDuiControl.h"


class MenuDefaultKey;

class ServerListFilterHelpMenuDefaultKey final : public MenuDefaultKey
{
public:
    ServerListFilterHelpMenuDefaultKey() {}
    ~ServerListFilterHelpMenuDefaultKey() {}

    bool keyPress(const BzfKeyEvent&) override;
    bool keyRelease(const BzfKeyEvent&) override;
};


class ServerListFilterHelpMenu : public HUDDialog
{
public:
    ServerListFilterHelpMenu(const char* title = "Server List Filter Help");
    ~ServerListFilterHelpMenu()
    {
    }

    HUDuiDefaultKey* getDefaultKey() override final
    {
        return &defaultKey;
    }
    void execute() override final
    {
    }
    void resize(int width, int height) override final;

    static ServerListFilterHelpMenu* getServerListFilterHelpMenu(HUDDialog* = NULL, bool next = true);
    static void done();

protected:
    HUDuiControl* createLabel(const char* string, const char* label = NULL);
    HUDuiControl* createInput(const std::string &);
    virtual float getLeftSide(int width, int height);

private:
    ServerListFilterHelpMenuDefaultKey defaultKey;
    static ServerListFilterHelpMenu** serverListFilterHelpMenus;
};


#endif /* __HELPMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
