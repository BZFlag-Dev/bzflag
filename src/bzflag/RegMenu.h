/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	__REGMENU_H__
#define	__REGMENU_H__

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
#include "HUDuiImage.h"

class RegConnectSocket;

class RegMenu : public HUDDialog {
  public:
			RegMenu();
			~RegMenu();

    HUDuiDefaultKey*	getDefaultKey();

    void		show();
    void		execute();
    void		dismiss();
    void		resize(int width, int height);

    void    update();
    static void playingCB(void*);

  private:
    void		setStatus(const char*, const std::vector<std::string> *parms = NULL);
    void		setFailedMessage(const char* msg);
    void		loadInfo();

  private:
    friend class RegConnectSocket;

    float		center;
    HUDuiLabel*   reg_label;
    HUDuiTypeIn*	callsign;
    HUDuiTypeIn*	password;
    HUDuiLabel*		status;
    HUDuiLabel*		failedMessage;

    RegConnectSocket *regSocket;
    int phase;
};

class SocketHandler;
extern SocketHandler authSockHandler;

#endif /* __JOINMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
