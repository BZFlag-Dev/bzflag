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

#ifndef __SERVERMENU_H__
#define __SERVERMENU_H__

// ancestor classes
#include "MenuDefaultKey.h"
#include "HUDDialog.h"

/* system interface headers */
#include <string>
#include <vector>
#include <deque>

/* common interface headers */
#include "BzfEvent.h"
#include "ServerItem.h"
#include "ServerList.h"

/* local interface headers */
#include "HUDuiDefaultKey.h"
#include "HUDuiLabel.h"
#include "HUDuiTypeIn.h"
#include "ServerListFilter.h"


class ServerListFilterMenu;
class ServerListFilterHelpMenu;
class ServerMenu;


class ServerMenuDefaultKey final : public MenuDefaultKey
{
public:
    ServerMenuDefaultKey(ServerMenu* _menu) :
        menu(_menu), serverListFilterMenu(NULL) {}
    ~ServerMenuDefaultKey();

    bool keyPress(const BzfKeyEvent&) override;
    bool keyRelease(const BzfKeyEvent&) override;

private:
    ServerMenu* menu;
    ServerListFilterMenu* serverListFilterMenu;
};

class ServerMenu final : public HUDDialog
{
public:
    ServerMenu();
    ~ServerMenu() {}

    HUDuiDefaultKey* getDefaultKey() override
    {
        return &defaultKey;
    }
    int getSelected() const;
    void setSelected(int, bool forcerefresh=false);
    void show() override;
    void execute() override;
    void dismiss() override;
    void resize(int width, int height) override;
    void updateStatus();
    void setFindLabel(const std::string& label);

    bool getFind() const;
    void setFind(bool mode, bool clear = false);
    void setFindIndex(int index);

    void toggleFavView();
    void setFav(bool);

    static void playingCB(void*);

    static const int NumItems;

private:
    void addLabel(const char* str, const char* label);
    void setStatus(const char*, const std::vector<std::string> *parms = NULL);
    void pick();

    ServerItem& serversAt(int index);

private:
    ServerList realServerList;
    ServerList serverList;
    ServerMenuDefaultKey  defaultKey;
    HUDuiLabel* status;
    HUDuiLabel* help1;
    HUDuiLabel* help2;

    HUDuiLabel* pageLabel;
    int selectedIndex;
    unsigned int serversFound;
    unsigned int realServersFound;

    HUDuiTypeIn* search;
    bool findMode;
    bool favView;
    bool newfilter;

    ServerListFilter listFilter;

    int lastWidth, lastHeight;

    static const int NumReadouts;
};


#endif /* __SERVERMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
