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
#include "OptionsMenu.h"

/* common implementation headers */
#include "FontManager.h"
#include "StateDatabase.h"

/* local implementation headers */
#include "MainMenu.h"
#include "HUDDialogStack.h"
#include "HUDui.h"
#include "BundleMgr.h"
#include "World.h"
#include "sound.h"
#include "bzflag.h"
#include "ConfigFileManager.h"
#include "clientConfig.h"

OptionsMenu::OptionsMenu() : guiOptionsMenu(NULL), effectsMenu(NULL),
    cacheMenu(NULL), saveWorldMenu(NULL),
    inputMenu(NULL),
    displayMenu(NULL)
{
    // cache font face ID
    int fontFace = MainMenu::getFontFace();

    // add controls
    std::vector<HUDuiControl*>& listHUD = getControls();
    HUDuiList* option;
    std::vector<std::string>* options;

    HUDuiLabel* label = new HUDuiLabel;
    label->setFontFace(fontFace);
    label->setString("Options");
    listHUD.push_back(label);

    inputSetting = label = new HUDuiLabel;
    label->setFontFace(fontFace);
    label->setLabel("Input Settings");
    listHUD.push_back(label);

    displaySetting = label = new HUDuiLabel;
    label->setFontFace(fontFace);
    label->setLabel("Display Settings");
    listHUD.push_back(label);

    effectsOptions = label = new HUDuiLabel;
    label->setFontFace(fontFace);
    label->setLabel("Effects Settings");
    listHUD.push_back(label);

    guiOptions = label = new HUDuiLabel;
    label->setFontFace(fontFace);
    label->setLabel("GUI Settings");
    listHUD.push_back(label);

    cacheOptions = label = new HUDuiLabel;
    label->setFontFace(fontFace);
    label->setLabel("Cache Settings");
    listHUD.push_back(label);

    // set locale
    option = new HUDuiList;
    option->setFontFace(fontFace);
    option->setLabel("Locale:");
    option->setCallback(callback, "L");
    options = &option->getList();
    std::vector<std::string> locales;
    if (BundleMgr::getLocaleList(&locales) == true)
    {
        options->push_back(std::string("English"));
        for (int i = 0; i < (int)locales.size(); i++)
            options->push_back(locales[i]);
        locales.erase(locales.begin(), locales.end());
    }
    else
    {
        // Something failed when trying to compile a list
        // of all the locales.
        options->push_back(std::string("Default"));
    }

    for (int i = 0; i < (int)options->size(); i++)
    {
        if ((*options)[i].compare(World::getLocale()) == 0)
        {
            option->setIndex(i);
            break;
        }
    }
    option->update();
    listHUD.push_back(option);

    // Sound Volume
    option = new HUDuiList;
    option->setFontFace(MainMenu::getFontFace());
    option->setLabel("Sound Volume:");
    option->setCallback(callback, "s");
    options = &option->getList();
    if (isSoundOpen())
    {
        options->push_back(std::string("Off"));
        option->createSlider(10);
    }
    else
        options->push_back(std::string("Unavailable"));
    option->update();
    listHUD.push_back(option);

    // Remotes Sounds
    option = new HUDuiList;
    option->setFontFace(MainMenu::getFontFace());
    option->setLabel("Remote Sounds:");
    option->setCallback(callback, "r");
    options = &option->getList();
    options->push_back(std::string("Off"));
    options->push_back(std::string("On"));
    option->update();
    listHUD.push_back(option);

    option = new HUDuiList;
    option->setFontFace(fontFace);
    option->setLabel("Save Settings On Exit:");
    option->setCallback(callback, "S");
    options = &option->getList();
    options->push_back(std::string("No"));
    options->push_back(std::string("Yes"));
    option->update();
    listHUD.push_back(option);

    option = new HUDuiList;
    option->setFontFace(fontFace);
    option->setLabel("Save identity:");
    option->setCallback(callback, "i");
    options = &option->getList();
    options->push_back(std::string("No"));
    options->push_back(std::string("Username only"));
    options->push_back(std::string("Username and password"));
    option->update();
    listHUD.push_back(option);

    save = label = new HUDuiLabel;
    label->setFontFace(fontFace);
    label->setLabel("Save Settings");
    listHUD.push_back(save);

    saveWorld = label = new HUDuiLabel;
    label->setFontFace(fontFace);
    label->setLabel("Save World");
    listHUD.push_back(label);

    initNavigation(listHUD, 1, listHUD.size()-1);
}

OptionsMenu::~OptionsMenu()
{
    delete guiOptionsMenu;
    delete effectsMenu;
    delete cacheMenu;
    delete saveWorldMenu;
    delete inputMenu;
    delete displayMenu;
}

void OptionsMenu::execute()
{
    HUDuiControl* _focus = HUDui::getFocus();
    if (_focus == guiOptions)
    {
        if (!guiOptionsMenu) guiOptionsMenu = new GUIOptionsMenu;
        HUDDialogStack::get()->push(guiOptionsMenu);
    }
    else if (_focus == effectsOptions)
    {
        if (!effectsMenu) effectsMenu = new EffectsMenu;
        HUDDialogStack::get()->push(effectsMenu);
    }
    else if (_focus == cacheOptions)
    {
        if (!cacheMenu) cacheMenu = new CacheMenu;
        HUDDialogStack::get()->push(cacheMenu);
    }
    else if (_focus == saveWorld)
    {
        if (!saveWorldMenu) saveWorldMenu = new SaveWorldMenu;
        HUDDialogStack::get()->push(saveWorldMenu);
    }
    else if (_focus == inputSetting)
    {
        if (!inputMenu) inputMenu = new InputMenu;
        HUDDialogStack::get()->push(inputMenu);
    }
    else if (_focus == displaySetting)
    {
        if (!displayMenu) displayMenu = new DisplayMenu;
        HUDDialogStack::get()->push(displayMenu);
    }
    else if (_focus == save)
    {
        // save resources
        dumpResources();
        if (alternateConfig == "")
            CFGMGR.write(getCurrentConfigFileName());
        else
            CFGMGR.write(alternateConfig);
    }
}

void OptionsMenu::resize(int _width, int _height)
{
    int i;
    HUDDialog::resize(_width, _height);

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

    // reposition options in two columns
    x = 0.5f * (float)_width;
    y -= 0.6f * titleHeight;
    const int count = listHUD.size();
    const float h = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
    for (i = 1; i < count; i++)
    {
        HUDuiControl *ctl = listHUD[i];
        ctl->setFontSize(fontSize);
        ctl->setPosition(x, y);
        if (i == 5 || i == 6 || i == 8 || i == 11)
            y -= 1.75f * h;
        else
            y -= 1.0f * h;
    }



    // load current settings
    i = 7;

    // sound
    ((HUDuiList*)listHUD[i++])->setIndex(getSoundVolume());
    ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("remoteSounds") ? 1 : 0);

    ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("saveSettings"));
    ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("saveIdentity"));
}

void OptionsMenu::callback(HUDuiControl* w, const void* data)
{
    HUDuiList* listHUD = (HUDuiList*)w;

    switch (((const char*)data)[0])
    {
    case 'L':
    {
        std::vector<std::string>* options = &listHUD->getList();
        std::string locale = (*options)[listHUD->getIndex()];

        World::setLocale(locale);
        BZDB.set("locale", locale);
        World::getBundleMgr()->getBundle(locale, true);

        OptionsMenu *menu = (OptionsMenu *)HUDDialogStack::get()->top();
        if (menu)
            menu->resize(menu->getWidth(), menu->getHeight());
        break;
    }
    case 's':
    {
        BZDB.set("volume", TextUtils::format("%d", listHUD->getIndex()));
        setSoundVolume(listHUD->getIndex());
        break;
    }
    case 'r':
    {
        BZDB.setBool("remoteSounds", (listHUD->getIndex() == 0) ? false : true);
        break;
    }
    case 'S':   // save settings
    {
        BZDB.setInt("saveSettings", listHUD->getIndex());
        break;
    }
    case 'i':   // save identity
    {
        BZDB.setInt("saveIdentity", listHUD->getIndex());
        break;
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
