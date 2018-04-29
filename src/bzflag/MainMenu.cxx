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

/* interface header */
#include "MainMenu.h"

/* common implementation headers */
#include "TextureManager.h"
#include "FontManager.h"

/* local implementation headers */
#include "HelpMenu.h"
#include "HUDDialogStack.h"
#include "LocalPlayer.h"
#include "JoinMenu.h"
#include "OptionsMenu.h"
#include "QuitMenu.h"
#include "HUDuiTextureLabel.h"
#include "ConfigFileManager.h"
#include "clientConfig.h"
#include "bzflag.h"
#include "playing.h"
#include "HUDui.h"

MainMenu::MainMenu() : HUDDialog(),
		       join(), options(), help(), leave(), save(), quit(),
		       joinMenu(NULL), optionsMenu(NULL), quitMenu(NULL)
{
}

void	  MainMenu::createControls()
{
  TextureManager &tm = TextureManager::instance();
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiControl* label;
  HUDuiTextureLabel* textureLabel;

  // clear controls
  for (unsigned int i = 0; i < listHUD.size(); i++)
    delete listHUD[i];
  listHUD.erase(listHUD.begin(), listHUD.end());

  // load title
  int title = tm.getTextureID("title");

  // add controls
  textureLabel = new HUDuiTextureLabel;
  textureLabel->setFontFace(getFontFace());
  textureLabel->setTexture(title);
  textureLabel->setString("BZFlag");
  listHUD.push_back(textureLabel);

  label = createLabel("Up/Down arrows to move, Enter to select, Esc to dismiss");
  listHUD.push_back(label);

  join = createLabel("Join Game");
  listHUD.push_back(join);

  options = createLabel("Options");
  listHUD.push_back(options);

  save = createLabel("Save Settings");
  listHUD.push_back(save);

  help = createLabel("Help");
  listHUD.push_back(help);

  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (!(myTank == NULL)) {
    leave = createLabel("Leave Game");
    listHUD.push_back(leave);
  } else {
    leave = NULL;
  }

  quit = createLabel("Quit");
  listHUD.push_back(quit);

  resize(HUDDialog::getWidth(), HUDDialog::getHeight());
  initNavigation(listHUD, 2, listHUD.size() - 1);

  // set focus back at the top in case the item we had selected does not exist anymore
  listHUD[2]->setFocus();
}

HUDuiControl* MainMenu::createLabel(const char* string)
{
  HUDuiLabel* control = new HUDuiLabel;
  control->setFontFace(getFontFace());
  control->setString(string);
  return control;
}

MainMenu::~MainMenu()
{
  // clear controls
  std::vector<HUDuiControl *>& listHUD = getControls();
  for (unsigned int i = 0; i < listHUD.size(); i++)
    delete listHUD[i];
  listHUD.erase(listHUD.begin(), listHUD.end());

  // destroy submenus
  delete joinMenu;
  delete optionsMenu;
  delete quitMenu;
  HelpMenu::done();
}

int			MainMenu::getFontFace()
{
  // create font
  return FontManager::instance().getFaceID(BZDB.get("sansSerifFont"));
}

HUDuiDefaultKey*	MainMenu::getDefaultKey()
{
  return MenuDefaultKey::getInstance();
}

void			MainMenu::execute()
{
  HUDuiControl* _focus = HUDui::getFocus();
  if (_focus == join) {
    if (!joinMenu) joinMenu = new JoinMenu;
    HUDDialogStack::get()->push(joinMenu);
  } else if (_focus == options) {
    if (!optionsMenu) optionsMenu = new OptionsMenu;
    HUDDialogStack::get()->push(optionsMenu);
  } else if (_focus == help) {
    HUDDialogStack::get()->push(HelpMenu::getHelpMenu());
  } else if (_focus == leave) {
    leaveGame();
    // myTank should be NULL now, recreate menu
    createControls();
  } else if (_focus == save) {
    // save resources
    dumpResources();
    if (alternateConfig == "")
      CFGMGR.write(getCurrentConfigFileName());
    else
      CFGMGR.write(alternateConfig);
  } else if (_focus == quit) {
    if (!quitMenu) quitMenu = new QuitMenu;
    HUDDialogStack::get()->push(quitMenu);
  }
}

void			MainMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);

  // use a big font
  const float titleFontSize = (float)_height / 8.0f;
  const float tinyFontSize = (float)_height / 54.0f;
  const float fontSize = (float)_height / 22.0f;
  FontManager &fm = FontManager::instance();
  int fontFace = getFontFace();

  // reposition title
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  // scale appropriately to center properly
  TextureManager &tm = TextureManager::instance();
  float texHeight = (float)tm.getInfo(((HUDuiTextureLabel*)title)->getTexture()).y;
  float texWidth = (float)tm.getInfo(((HUDuiTextureLabel*)title)->getTexture()).x;
  float titleWidth = (texWidth / texHeight) * titleFontSize;
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleFontSize * 1.25f;
  title->setPosition(x, y);

  // reposition instructions
  HUDuiLabel* hint = (HUDuiLabel*)listHUD[1];
  hint->setFontSize(tinyFontSize);
  const float hintWidth = fm.getStrLength(fontFace, tinyFontSize, hint->getString());
  y -= 1.25f * fm.getStrHeight(fontFace, tinyFontSize, hint->getString());
  hint->setPosition(0.5f * ((float)_width - hintWidth), y);
  y -= 1.5f * fm.getStrHeight(fontFace, fontSize, hint->getString());

  // reposition menu items (first is centered, rest aligned to the first)
  const float firstWidth
    = fm.getStrLength(fontFace, fontSize,
		      ((HUDuiLabel*)listHUD[2])->getString());
  x = 0.5f * ((float)_width - firstWidth);
  const int count = listHUD.size();
  for (int i = 2; i < count; i++) {
    HUDuiLabel* label = (HUDuiLabel*)listHUD[i];
    label->setFontSize(fontSize);
    label->setPosition(x, y);
    y -= 1.2f * fm.getStrHeight(fontFace, fontSize, label->getString());
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
