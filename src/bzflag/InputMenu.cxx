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

/* interface header */
#include "InputMenu.h"

/* common implementation headers */
#include "StateDatabase.h"
#include "FontManager.h"

/* local implementation headers */
#include "MainMenu.h"
#include "HUDDialogStack.h"
#include "LocalPlayer.h"
#include "playing.h"
#include "HUDui.h"

InputMenu::InputMenu() : keyboardMapMenu(nullptr), joystickTestMenu(nullptr)
{
    std::string currentJoystickDevice = BZDB.get("joystickname");
    // cache font face ID
    int fontFace = MainMenu::getFontFace();
    // add controls
    std::vector<HUDuiControl*>& listHUD = getControls();

    HUDuiLabel* label = new HUDuiLabel;
    label->setFontFace(fontFace);
    label->setString("Input Settings");
    listHUD.push_back(label);

    keyMapping = new HUDuiLabel;
    keyMapping->setFontFace(fontFace);
    keyMapping->setLabel("Change Key Mapping");
    listHUD.push_back(keyMapping);

    HUDuiList* option;

    activeInput = new HUDuiList;
    activeInput->setFontFace(fontFace);
    activeInput->setLabel("Active input device:");
    activeInput->setCallback(callback, "A");
    std::vector<std::string>* options = &activeInput->getList();
    options->push_back("Auto");
    options->push_back(LocalPlayer::getInputMethodName(LocalPlayer::Keyboard));
    options->push_back(LocalPlayer::getInputMethodName(LocalPlayer::Mouse));
    options->push_back(LocalPlayer::getInputMethodName(LocalPlayer::Joystick));
    activeInput->update();
    listHUD.push_back(activeInput);

    option = new HUDuiList;
    options = &option->getList();
    // set joystick Device
    option->setFontFace(fontFace);
    option->setLabel("Joystick Device:");
    option->setCallback(callback, "J");
    options = &option->getList();
    options->push_back(std::string("Off"));
    std::vector<std::string> joystickDevices;
    getMainWindow()->getJoyDevices(joystickDevices);
    int i;
    for (i = 0; i < (int)joystickDevices.size(); i++)
        options->push_back(joystickDevices[i]);
    joystickDevices.erase(joystickDevices.begin(), joystickDevices.end());
    for (i = 0; i < (int)options->size(); i++)
    {
        if ((*options)[i].compare(currentJoystickDevice) == 0)
        {
            option->setIndex(i);
            break;
        }
    }
    option->update();
    listHUD.push_back(option);

    option = new HUDuiList;
    // axis settings
    jsx = option;
    option->setFontFace(fontFace);
    option->setLabel("Joystick X Axis:");
    option->setCallback(callback, "X");
    listHUD.push_back(option);
    option = new HUDuiList;
    jsy = option;
    option->setFontFace(fontFace);
    option->setLabel("Joystick Y Axis:");
    option->setCallback(callback, "Y");
    listHUD.push_back(option);
    fillJSOptions();

    option = new HUDuiList;
    // joystick range settings
    option->setFontFace(fontFace);
    option->setLabel("Joystick Range Limit:");
    option->setCallback(callback, "T");
    options = &option->getList();
    char percentText[5];
    for(auto i = 25; i <= 100; ++i)
    {
        snprintf(percentText, 20, "%i%%", i);
        options->push_back(percentText);
    }
    for (i = 0; i < (int)options->size(); i++)
    {
        std::string currentOption = (*options)[i];
        if (BZDB.get("jsRangeMax") + "%" == currentOption)
            option->setIndex(i);
    }
    option->update();
    listHUD.push_back(option);
    option = new HUDuiList;
    option->setFontFace(fontFace);
    option->setLabel("Joystick Dead Zone:");
    option->setCallback(callback, "B");
    options = &option->getList();
    for(auto i = 0; i <= 20; ++i)
    {
        snprintf(percentText, 20, "%i%%", i);
        options->push_back(percentText);
    }
    for (i = 0; i < (int)options->size(); i++)
    {
        std::string currentOption = (*options)[i];
        if (BZDB.get("jsRangeMin") + "%" == currentOption)
            option->setIndex(i);
    }
    option->update();
    listHUD.push_back(option);
    option = new HUDuiList;
    option->setFontFace(fontFace);
    option->setLabel("Stretch Joystick Range Corners:");
    option->setCallback(callback, "S");
    options = &option->getList();
    options->push_back(std::string("No"));
    options->push_back(std::string("Yes"));
    option->setIndex(BZDB.isTrue("jsStretchCorners") ? 1 : 0);
    option->update();
    listHUD.push_back(option);
    option = new HUDuiList;
    option->setFontFace(fontFace);
    option->setLabel("Joystick Ramp Type:");
    option->setCallback(callback, "R");
    options = &option->getList();
    options->push_back(std::string("Linear"));
    options->push_back(std::string("Exponential (Squared)"));
    options->push_back(std::string("Exponential (Cubed)"));
    if(BZDB.get("jsRampType") == "squared")
        option->setIndex(1);
    else if(BZDB.get("jsRampType") == "cubed")
        option->setIndex(2);
    else
        option->setIndex(0);
    option->update();
    listHUD.push_back(option);

    option = new HUDuiList;
    // force feedback
    option->setFontFace(fontFace);
    option->setLabel("Force Feedback:");
    option->setCallback(callback, "F");
    options = &option->getList();
    options->push_back(std::string("None"));
    options->push_back(std::string("Rumble"));
    options->push_back(std::string("Directional"));
    for (i = 0; i < (int)options->size(); i++)
    {
        std::string currentOption = (*options)[i];
        if (BZDB.get("forceFeedback") == currentOption)
            option->setIndex(i);
    }
    option->update();
    listHUD.push_back(option);
    joystickTest = new HUDuiLabel;
    joystickTest->setFontFace(fontFace);
    joystickTest->setLabel("Test Joystick Range");
    listHUD.push_back(joystickTest);

    option = new HUDuiList;
    // confine mouse
    option->setFontFace(fontFace);
    option->setLabel("Confine mouse:");
    option->setCallback(callback, "G");
    options = &option->getList();
    options->push_back(std::string("No"));
    options->push_back(std::string("Window"));
    options->push_back(std::string("MotionBox"));
    if (getMainWindow()->isGrabEnabled())
        option->setIndex(1);
    else if (BZDB.isTrue("mouseClamp"))
        option->setIndex(2);
    else
        option->setIndex(0);
    option->update();
    listHUD.push_back(option);

    // set maxmotion size
    option = new HUDuiList;
    option->setFontFace(fontFace);
    option->setLabel("Mouse Box Size:");
    option->setCallback(callback, "M");
    option->createSlider(22);
    option->update();
    listHUD.push_back(option);

    option = new HUDuiList;
    // jump while typing on/off
    option->setFontFace(fontFace);
    option->setLabel("Jump while typing:");
    option->setCallback(callback, "H");
    options = &option->getList();
    options->push_back(std::string("No"));
    options->push_back(std::string("Yes"));
    option->setIndex(BZDB.isTrue("jumpTyping") ? 1 : 0);
    option->update();
    listHUD.push_back(option);

    initNavigation(listHUD, 1,listHUD.size()-1);
}

InputMenu::~InputMenu()
{
    delete keyboardMapMenu;
    delete joystickTestMenu;
}

void InputMenu::fillJSOptions()
{
    std::vector<std::string>* xoptions = &jsx->getList();
    std::vector<std::string>* yoptions = &jsy->getList();
    std::vector<std::string> joystickAxes;
    getMainWindow()->getJoyDeviceAxes(joystickAxes);
    const auto xAxisInverted = BZDB.evalInt("jsInvertAxes") % 2 == 1;
    const auto yAxisInverted = BZDB.evalInt("jsInvertAxes") > 1;

    xoptions->clear();
    yoptions->clear();

    if(joystickAxes.empty())
        joystickAxes.push_back("N/A");
    int i;
    for (i = 0; i < (int)joystickAxes.size(); i++)
    {
        xoptions->push_back(joystickAxes[i]);
        yoptions->push_back(joystickAxes[i]);
        if(joystickAxes[i] != "N/A")
        {
            xoptions->push_back(joystickAxes[i] + " (Inverted)");
            yoptions->push_back(joystickAxes[i] + " (Inverted)");
        }
    }
    bool found = false;
    for (i = 0; i < (int)xoptions->size(); i++)
    {
        bool currentOptionInverted = false;
        std::string currentOption = (*xoptions)[i];

        // could also be inverted
        if(currentOption.length() >= 12
                && currentOption.substr(currentOption.length() - 10, std::string::npos) == "(Inverted)")
        {
            currentOption = currentOption.substr(0, currentOption.length() - 11);
            currentOptionInverted = true;
        }

        if (BZDB.get("jsXAxis") == currentOption && currentOptionInverted == xAxisInverted)
        {
            jsx->setIndex(i);
            found = true;
        }
    }
    if (!found)
        jsx->setIndex(0);
    jsx->update();
    found = false;
    for (i = 0; i < (int)yoptions->size(); i++)
    {
        bool currentOptionInverted = false;
        std::string currentOption = (*yoptions)[i];

        // could also be inverted
        if(currentOption.length() >= 12
                && currentOption.substr(currentOption.length() - 10, std::string::npos) == "(Inverted)")
        {
            currentOption = currentOption.substr(0, currentOption.length() - 11);
            currentOptionInverted = true;
        }

        if (BZDB.get("jsYAxis") == currentOption && currentOptionInverted == yAxisInverted)
        {
            jsy->setIndex(i);
            found = true;
        }
    }
    if (!found)
    {
        if (yoptions->size() > 1)
            jsy->setIndex(1);
        else
            jsy->setIndex(0);
    }
    jsy->update();
}

void            InputMenu::execute()
{
    HUDuiControl* _focus = HUDui::getFocus();
    if (_focus == keyMapping)
    {
        if (!keyboardMapMenu) keyboardMapMenu = new KeyboardMapMenu;
        HUDDialogStack::get()->push(keyboardMapMenu);
    }
    else if (_focus == joystickTest)
    {
        if (!joystickTestMenu) joystickTestMenu = new JoystickTestMenu;
        HUDDialogStack::get()->push(joystickTestMenu);
    }
}

void            InputMenu::callback(HUDuiControl* w, const void* data)
{
    HUDuiList* listHUD = (HUDuiList*)w;
    std::vector<std::string> *options = &listHUD->getList();
    std::string selectedOption = (*options)[listHUD->getIndex()];
    InputMenu *menu = (InputMenu *) HUDDialogStack::get()->top();
    switch (((const char*)data)[0])
    {

    /* Joystick name */
    case 'J':
        BZDB.set("joystickname", selectedOption);
        getMainWindow()->initJoystick(selectedOption);
        // re-fill all of the joystick-specific options lists
        if (menu)
            menu->fillJSOptions();
        break;

    /* Joystick x-axis */
    case 'X':
    {
        auto selectedAxis = selectedOption;
        auto xAxisInverted = false;
        const auto oldInvertAxes = BZDB.evalInt("jsInvertAxes");

        if(selectedOption.length() >= 12
                && selectedOption.substr(selectedOption.length() - 10, std::string::npos) == "(Inverted)")
        {
            selectedAxis = selectedAxis.substr(0, selectedAxis.length() - 11);
            xAxisInverted = true;
        }

        BZDB.set("jsXAxis", selectedAxis);
        getMainWindow()->setJoyXAxis(selectedAxis);

        if(xAxisInverted) // X axis inversion needs to be enabled
        {
            if(oldInvertAxes % 2 != 1) // it wasn't already enabled
            {
                BZDB.setInt("jsInvertAxes", oldInvertAxes == 2 ? 3 : 1); // preserve the Y enabled value
            }
        }
        else // X axis inversion needs to be disabled
        {
            if(oldInvertAxes % 2 == 1) // it was enabled previously
            {
                BZDB.setInt("jsInvertAxes", oldInvertAxes == 3 ? 2 : 0); // preserve the Y enabled value
            }
        }

        break;
    }

    /* Joystick y-axis */
    case 'Y':
    {
        auto selectedAxis = selectedOption;
        auto yAxisInverted = false;
        const auto oldInvertAxes = BZDB.evalInt("jsInvertAxes");

        if(selectedOption.length() >= 12
                && selectedOption.substr(selectedOption.length() - 10, std::string::npos) == "(Inverted)")
        {
            selectedAxis = selectedAxis.substr(0, selectedAxis.length() - 11);
            yAxisInverted = true;
        }

        BZDB.set("jsYAxis", selectedAxis);
        getMainWindow()->setJoyYAxis(selectedAxis);

        if(yAxisInverted) // Y axis inversion needs to be enabled
        {
            if(oldInvertAxes < 2) // it wasn't already enabled
            {
                BZDB.setInt("jsInvertAxes", oldInvertAxes == 1 ? 3 : 2); // preserve the X enabled value
            }
        }
        else // Y axis inversion needs to be disabled
        {
            if(oldInvertAxes > 1) // it was enabled previously
            {
                BZDB.setInt("jsInvertAxes", oldInvertAxes == 3 ? 1 : 0); // preserve the X enabled value
            }
        }

        break;
    }

    /* Joystick axes inversion */
    case 'I':
    {
        BZDB.setInt("jsInvertAxes", listHUD->getIndex());
        break;
    }

    /* Active input device */
    case 'A':
    {
        LocalPlayer*   myTank = LocalPlayer::getMyTank();
        // Are we forced to use one input device, or do we allow it to change automatically?
        if (selectedOption == "Auto")
            BZDB.set("allowInputChange", "1");
        else
        {
            BZDB.set("allowInputChange", "0");
            BZDB.set("activeInputDevice", selectedOption);
            // Set the current input device to whatever we're forced to
            if (myTank)
                myTank->setInputMethod(BZDB.get("activeInputDevice"));
        }
    }
    break;

    /* Grab mouse */
    case 'G':
    {
        const int index = listHUD->getIndex();
        // Window
        const bool grabbing = (index == 1);
        BZDB.set("mousegrab", grabbing ? "1" : "0");
        getMainWindow()->enableGrabMouse(grabbing);

        // Mouse Box
        const bool clamped = (index == 2);
        BZDB.set("mouseClamp", clamped ? "1" : "0");
    }
    break;


    case 'M':
    {
        SceneRenderer* renderer = getSceneRenderer();
        if (renderer != nullptr)
            renderer->setMaxMotionFactor(listHUD->getIndex() - 11);
    }
    break;

    /* Jump while typing */
    case 'H':
    {
        bool jump = (selectedOption == "Yes");
        BZDB.setBool("jumpTyping", jump ? true : false);
    }
    break;

    /* Force Feedback */
    case 'F':
        BZDB.set("forceFeedback", selectedOption);
        break;

    /* Joystick Range Limit */
    case 'T':
        BZDB.set("jsRangeMax", selectedOption.substr(0, selectedOption.length() - 1));
        break;

    /* Joystick Dead Zone */
    case 'B':
        BZDB.set("jsRangeMin", selectedOption.substr(0, selectedOption.length() - 1));
        break;

    /* Stretch Joystick Range Corners */
    case 'S':
        BZDB.setBool("jsStretchCorners", selectedOption == "Yes" ? true : false);
        break;

    /* Joystick Ramp Type */
    case 'R':
        if(selectedOption == "Exponential (Squared)")
            BZDB.set("jsRampType", "squared");
        else if(selectedOption == "Exponential (Cubed)")
            BZDB.set("jsRampType", "cubed");
        else
            BZDB.set("jsRampType", "linear");
        break;

    }
}

void            InputMenu::resize(int _width, int _height)
{
    HUDDialog::resize(_width, _height);
    int i;

    // use a big font for title, smaller font for the rest
    const float titleFontSize = (float)_height / 15.0f;
    const float fontSize = (float)_height / 45.0f;
    FontManager &fm = FontManager::instance();

    // reposition title
    std::vector<HUDuiControl*>& listHUD = getControls();
    HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
    title->setFontSize(titleFontSize);
    const float titleWidth = fm.getStrLength(MainMenu::getFontFace(), titleFontSize, title->getString());
    const float titleHeight = fm.getStrHeight(MainMenu::getFontFace(), titleFontSize, " ");
    float x = 0.5f * ((float)_width - titleWidth);
    float y = (float)_height - titleHeight;
    title->setPosition(x, y);

    // reposition options
    x = 0.5f * ((float)_width);
    y -= 0.6f * titleHeight;
    const float h = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
    const int count = listHUD.size();
    for (i = 1; i < count; i++)
    {
        listHUD[i]->setFontSize(fontSize);
        listHUD[i]->setPosition(x, y);
        // Add extra space after Change Key Mapping, Active input device, Test Joystick Range, and Mouse Box Size
        if (i == 1 || i == 2 || i == 11 || i == 13)
            y -= 1.75f * h;
        else
            y -= 1.0f * h;
    }

    // load current settings
    std::vector<std::string> *options = &activeInput->getList();
    for (i = 0; i < (int)options->size(); i++)
    {
        std::string currentOption = (*options)[i];
        if (BZDB.get("activeInputDevice") == currentOption)
            activeInput->setIndex(i);
    }
    if (BZDB.isTrue("allowInputChange"))
        activeInput->setIndex(0);

    SceneRenderer* renderer = getSceneRenderer();
    if (renderer != nullptr)
        ((HUDuiList*)listHUD[13])->setIndex(renderer->getMaxMotionFactor() + 11);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
