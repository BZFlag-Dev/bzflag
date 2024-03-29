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

#ifndef __JOINMENU_H__
#define __JOINMENU_H__

#include "common.h"

/* system interface headers */
#include <vector>
#include <string>

/* common interface headers */
#include "global.h"

/* local interface headers */
#include "HUDDialog.h"
#include "HUDuiDefaultKey.h"
#include "HUDuiControl.h"
#include "HUDuiLabel.h"
#include "HUDuiList.h"
#include "HUDuiTypeIn.h"
#include "HUDuiTextureLabel.h"

class ServerStartMenu;
class ServerMenu;

class JoinMenu : public HUDDialog
{
public:
    JoinMenu();
    ~JoinMenu();

    HUDuiDefaultKey*    getDefaultKey();

    void        show();
    void        execute();
    void        dismiss();
    void        resize(int width, int height);
    void        updateTeamTexture() const;

private:
    static void     teamCallback(HUDuiControl*, const void*);
    TeamColor       getTeam() const;
    void        setTeam(TeamColor);
    void        setStatus(const char*, const std::vector<std::string> *parms = NULL);
    void        setFailedMessage(const char* msg);
    void        centerLabelHorizontally(HUDuiLabel* label);
    void        loadInfo();

private:
    float       center;
    HUDuiTypeIn*    callsign;
    HUDuiTypeIn*    password;
    HUDuiTypeIn*    motto;
    HUDuiList*      team;
    HUDuiTextureLabel*  teamIcon;
    HUDuiTypeIn*    server;
    HUDuiTypeIn*    port;
    HUDuiLabel*     status;
    HUDuiLabel*     startServer;
    HUDuiLabel*     findServer;
    HUDuiLabel*     connectLabel;
    HUDuiLabel*     failedMessage;
    ServerStartMenu*    serverStartMenu;
    ServerMenu*     serverMenu;
    static JoinMenu*    activeMenu;
};


#endif /* __JOINMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
