/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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
#ifdef HAVE_KRB5
#include "LoginMenu.h"
#endif
#include "OptionsMenu.h"
#include "QuitMenu.h"
#include "HUDuiImage.h"
#include "playing.h"
#include "HUDui.h"

MainMenu::MainMenu() : HUDDialog(), joinMenu(NULL),
#ifdef HAVE_KRB5
		       loginMenu(NULL),
#endif
		       optionsMenu(NULL), quitMenu(NULL)
{
}

void	  MainMenu::createControls()
{
  TextureManager &tm = TextureManager::instance();
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiControl* label;
  HUDuiImage* textureLabel;

  // clear controls
  for (unsigned int i = 0; i < listHUD.size(); i++)
    delete listHUD[i];
  listHUD.erase(listHUD.begin(), listHUD.end());

  // load title
  int title = tm.getTextureID("title");

  // add controls
  std::vector<HUDuiElement*>& listEle = getElements();
  textureLabel = new HUDuiImage;
  textureLabel->setTexture(title);
  listEle.push_back(textureLabel);

  label = createLabel("Up/Down arrows to move, Enter to select, Esc to dismiss");
  listHUD.push_back(label);

  join = createLabel("Join Game");
  listHUD.push_back(join);

#ifdef HAVE_KRB5
  login = createLabel("Login");
  listHUD.push_back(login);
#endif

  options = createLabel("Options");
  listHUD.push_back(options);

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
  initNavigation(listHUD, 1, (int)listHUD.size() - 1);

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
#ifdef HAVE_KRB5
  delete loginMenu;
#endif
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
  HUDuiControl* _focus = HUDui::getFocus();
  if (_focus == join) {
    if (!joinMenu) joinMenu = new JoinMenu;
    HUDDialogStack::get()->push(joinMenu);
#ifdef HAVE_KRB5
  } else if (_focus == login) {
    if (!loginMenu) loginMenu = new LoginMenu;
    HUDDialogStack::get()->push(loginMenu);
#endif
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

  // use a big font
  const float titleSize = (float)_height / 8.0f;
  const float tinyFontSize = (float)_height / 54.0f;
  const float fontSize = (float)_height / 22.0f;
  FontManager &fm = FontManager::instance();
  int fontFace = getFontFace();

  // reposition title
  std::vector<HUDuiElement*>& listEle = getElements();
  HUDuiImage* title = (HUDuiImage*)listEle[0];
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
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiLabel* hint = (HUDuiLabel*)listHUD[0];
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
  const int count = (const int)listHUD.size();
  for (int i = 1; i < count; i++) {
    HUDuiLabel* label = (HUDuiLabel*)listHUD[i];
    label->setFontSize(fontSize);
    label->setPosition(x, y);
    y -= 1.2f * fm.getStrHeight(fontFace, fontSize, label->getString());
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

