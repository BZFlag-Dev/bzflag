/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "callbacks.h"

/* local headers */
#include "LocalPlayer.h"
#include "HUDRenderer.h"
#include "ParseColor.h"
#include "Team.h"
#include "playing.h"

void setFlagHelp(const std::string& name, void*)
{
    if (LocalPlayer::getMyTank() == NULL)
        return;
    static const float FlagHelpDuration = 60.0f;
    if (BZDB.isTrue(name))
        hud->setFlagHelp(LocalPlayer::getMyTank()->getFlag(), FlagHelpDuration);
    else
        hud->setFlagHelp(Flags::Null, 0.0);
}

void setShptBrightness(const std::string& name, void*)
{
    Team::updateShotColors();
}

void setDepthBuffer(const std::string& name, void*)
{
    /* if zbuffer was set and not available, unset it */
    if (BZDB.isTrue(name))
    {
        GLint value;
        glGetIntegerv(GL_DEPTH_BITS, &value);
        if (value == 0)
        {
            // temporarily remove ourself
            BZDB.removeCallback(name, setDepthBuffer, NULL);
            BZDB.set(name, "0");
            // add it again
            BZDB.addCallback(name, setDepthBuffer, NULL);
        }
    }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
