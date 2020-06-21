/* bzflag
 * Copyright (c) 1993-2020 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __JOYSTICKTESTMENU_H__
#define __JOYSTICKTESTMENU_H__

#include "common.h"

/* common interface headers */
#include "HUDDialog.h"

/* local interface headers */
#include "MenuDefaultKey.h"
#include "HUDuiControl.h"
#include "HUDuiList.h"
#include "HUDuiDefaultKey.h"
#include "HUDuiJSTestLabel.h"


/** this class provides a visual test of the joystick range
 */
class JoystickTestMenu : public HUDDialog
{
public:
    JoystickTestMenu();
    ~JoystickTestMenu() { };

    HUDuiDefaultKey* getDefaultKey()
    {
        return MenuDefaultKey::getInstance();
    }
    void execute() { };
    void resize(int width, int height);

private:
    HUDuiLabel* titleLabel;
    HUDuiLabel* fakeLabel;
    HUDuiJSTestLabel* jsTestLabel;
};


#endif /* __JOYSTICKTESTMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
