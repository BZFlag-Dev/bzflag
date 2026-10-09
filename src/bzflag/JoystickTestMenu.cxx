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

/* interface header */
#include "JoystickTestMenu.h"

/* common implementation headers */
#include "StateDatabase.h"
#include "FontManager.h"

/* local implementation headers */
#include "MainMenu.h"
#include "HUDDialogStack.h"
#include "LocalPlayer.h"
#include "playing.h"
#include "HUDui.h"

JoystickTestMenu::JoystickTestMenu()
{
    std::string currentJoystickDevice = BZDB.get("joystickname");
    // cache font face ID
    int fontFace = MainMenu::getFontFace();
    // add controls
    std::vector<HUDuiControl*>& listHUD = getControls();

    titleLabel = new HUDuiLabel;
    titleLabel->setFontFace(fontFace);
    titleLabel->setString("Test Joystick Range");
    listHUD.push_back(titleLabel);

    // make a fake menu entry to hide offscreen, since something needs to have focus
    fakeLabel = new HUDuiLabel;
    fakeLabel->setFontFace(fontFace);
    fakeLabel->setString("If you can see this, my worst nightmares have come true.");
    listHUD.push_back(fakeLabel);

    fakeLabel->setPrev(fakeLabel);
    fakeLabel->setNext(fakeLabel);
    setFocus(fakeLabel);

    jsTestLabel = new HUDuiJSTestLabel;
    listHUD.push_back(jsTestLabel);
}

void JoystickTestMenu::resize(int _width, int _height)
{
    HUDDialog::resize(_width, _height);

    // use a big font for title
    const auto titleFontSize = float(_height) / 15.0f;
    const auto fontSize = float(_height) / 45.0f;
    FontManager &fm = FontManager::instance();

    // reposition title
    titleLabel->setFontSize(titleFontSize);
    const auto titleWidth = fm.getStrLength(MainMenu::getFontFace(), titleFontSize, titleLabel->getString());
    const auto titleHeight = fm.getStrHeight(MainMenu::getFontFace(), titleFontSize, " ");
    titleLabel->setPosition(0.5f * ((float)_width - titleWidth), (float)_height - titleHeight);

    // reposition and resize joystick test area
    const auto testAreaSize = float(_height) - titleHeight - 0.6f * titleHeight;
    jsTestLabel->setPosition(0.5f * (float(_width) - testAreaSize),
                             fm.getStrHeight(MainMenu::getFontFace(), fontSize, " "));
    jsTestLabel->setSize(testAreaSize, testAreaSize);

    // reposition fake label off-screen
    fakeLabel->setFontSize(fontSize);
    fakeLabel->setPosition(0, _height * 2.0f);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
