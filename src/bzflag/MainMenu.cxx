/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
#include "FontSizer.h"
#include "HelpMenu.h"
#include "HUDDialogStack.h"
#include "LocalPlayer.h"
#include "JoinMenu.h"
#include "OptionsMenu.h"
#include "QuitMenu.h"
#include "HUDuiImage.h"
#include "HUDuiLabel.h"
#include "playing.h"

MainMenu::MainMenu() : HUDDialog(), joinMenu(NULL), optionsMenu(NULL), quitMenu(NULL)
{
}

void	  MainMenu::createControls()
{
  TextureManager &tm = TextureManager::instance();
  HUDuiControl* label;
  HUDuiImage* textureLabel;

  // clear controls
  std::vector<HUDuiElement*>& listHUD = getElements();
  for (unsigned int i = 0; i < listHUD.size(); i++)
    delete listHUD[i];
  listHUD.clear();
  getNav().clear();

  // load title
  int title = tm.getTextureID("title");

  // add controls
  textureLabel = new HUDuiImage;
  textureLabel->setTexture(title);
  addControl(textureLabel);

  label = createLabel("Up/Down arrows to move, Enter to select, Esc to dismiss");
  addControl(label, false);

  join = createLabel("Join Game");
  addControl(join);

  options = createLabel("Options");
  addControl(options);

  help = createLabel("Help");
  addControl(help);

  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (!(myTank == NULL)) {
    leave = createLabel("Leave Game");
    addControl(leave);
  } else {
    leave = NULL;
  }

  quit = createLabel("Quit");
  addControl(quit);

  initNavigation();
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
  // destroy submenus
  delete joinMenu;
  delete optionsMenu;
  delete quitMenu;
  HelpMenu::done();
}

const int		MainMenu::getFontFace()
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
  HUDuiControl* _focus = getNav().get();
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
  } else if (_focus == quit) {
    if (!quitMenu) quitMenu = new QuitMenu;
    HUDDialogStack::get()->push(quitMenu);
  }
}

void			MainMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);
  FontSizer fs = FontSizer(_width, _height);

  std::vector<HUDuiElement*>& listHUD = getElements();
  HUDuiLabel* hint = (HUDuiLabel*)listHUD[1];
  FontManager &fm = FontManager::instance();
  int fontFace = getFontFace();

  // main menu title, use a big font
  fs.setMin(0, 5);
  const float titleSize = fs.getFontSize(fontFace, "titleFontSize");

  // main menu instructions
  fs.setMin(20, 20);
  const float tinyFontSize = fs.getFontSize(fontFace, "hudFontSize");

  // main menu items
  fs.setMin(10, 10);
  const float fontSize = fs.getFontSize(fontFace, "headerFontSize");

  // reposition title
  HUDuiImage* title = (HUDuiImage*)listHUD[0];
  title->setSize((float)_width, titleSize);
  // scale appropriately to center properly
  TextureManager &tm = TextureManager::instance();
  float texHeight = (float)tm.getInfo(title->getTexture()).y;
  float texWidth = (float)tm.getInfo(title->getTexture()).x;
  float titleWidth = (texWidth / texHeight) * titleSize;
  title->setSize(titleWidth, titleSize);
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleSize * 1.25f;
  title->setPosition(x, y);

  // reposition instructions
  hint->setFontSize(tinyFontSize);
  const float hintWidth = fm.getStringWidth(fontFace, tinyFontSize, hint->getString().c_str());
  y -= 1.25f * fm.getStringHeight(fontFace, tinyFontSize);
  hint->setPosition(0.5f * ((float)_width - hintWidth), y);
  y -= 1.5f * fm.getStringHeight(fontFace, fontSize);

  // reposition menu items ("Options" is centered, rest aligned to it)
  const float firstWidth
    = fm.getStringWidth(fontFace, fontSize,
		      ((HUDuiLabel*)listHUD[3])->getString().c_str());
  x = 0.5f * ((float)_width - firstWidth);
  const int count = (const int)listHUD.size();
  for (int i = 2; i < count; i++) {
    HUDuiLabel* label = (HUDuiLabel*)listHUD[i];
    label->setFontSize(fontSize);
    label->setPosition(x, y);
    y -= 1.2f * fm.getStringHeight(fontFace, fontSize);
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
