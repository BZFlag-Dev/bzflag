/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "MainMenu.h"

/* system implementationheaders */
#include <string>
#include <vector>

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
#include "HUDuiControl.h"
#include "HUDuiLabel.h"
#include "HUDuiTextureLabel.h"

/* from playing.cxx */
void leaveGame();

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
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiControl* label;
  HUDuiTextureLabel* textureLabel;

  // clear controls
  for (unsigned int i = 0; i < list.size(); i++)
    delete list[i];
  list.erase(list.begin(), list.end());

  // load title
  int title = tm.getTextureID("title");

  // add controls
  textureLabel = new HUDuiTextureLabel;
  textureLabel->setFontFace(getFontFace());
  textureLabel->setTexture(title);
  textureLabel->setString("BZFlag");
  list.push_back(textureLabel);

  label = createLabel("Up/Down arrows to move, Enter to select, Esc to dismiss");
  list.push_back(label);

  join = createLabel("Join Game");
  list.push_back(join);

#ifdef HAVE_KRB5
  login = createLabel("Login");
  list.push_back(login);
#endif

  options = createLabel("Options");
  list.push_back(options);

  help = createLabel("Help");
  list.push_back(help);

  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (!(myTank == NULL)) {
    leave = createLabel("Leave Game");
    list.push_back(leave);
  } else {
    leave = NULL;
  }

  quit = createLabel("Quit");
  list.push_back(quit);

  resize(HUDDialog::getWidth(), HUDDialog::getHeight());
  initNavigation(list, 2, list.size() - 1);

  // set focus back at the top in case the item we had selected does not exist anymore
  list[2]->setFocus();
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
  std::vector<HUDuiControl *>& list = getControls();
  for (unsigned int i = 0; i < list.size(); i++)
    delete list[i];
  list.erase(list.begin(), list.end());

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
  HUDuiControl* focus = HUDui::getFocus();
  if (focus == join) {
    if (!joinMenu) joinMenu = new JoinMenu;
    HUDDialogStack::get()->push(joinMenu);
#ifdef HAVE_KRB5
  } else if (focus == login) {
    if (!loginMenu) loginMenu = new LoginMenu;
    HUDDialogStack::get()->push(loginMenu);
#endif
  } else if (focus == options) {
    if (!optionsMenu) optionsMenu = new OptionsMenu;
    HUDDialogStack::get()->push(optionsMenu);
  } else if (focus == help) {
    HUDDialogStack::get()->push(HelpMenu::getHelpMenu());
  } else if (focus == leave) {
    leaveGame();
    // myTank should be NULL now, recreate menu
    createControls();
  } else if (focus == quit) {
    if (!quitMenu) quitMenu = new QuitMenu;
    HUDDialogStack::get()->push(quitMenu);
  }
}

void			MainMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  // use a big font
  const float titleFontSize = (float)height / 8.0f;
  const float tinyFontSize = (float)height / 54.0f;
  const float fontSize = (float)height / 22.0f;
  FontManager &fm = FontManager::instance();
  int fontFace = getFontFace();

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontSize);
  // scale appropriately to center properly
  TextureManager &tm = TextureManager::instance();
  float texHeight = (float)tm.getInfo(((HUDuiTextureLabel*)title)->getTexture()).y;
  float texWidth = (float)tm.getInfo(((HUDuiTextureLabel*)title)->getTexture()).x;
  float titleWidth = (texWidth / texHeight) * titleFontSize;
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleFontSize * 1.25f;
  title->setPosition(x, y);

  // reposition instructions
  HUDuiLabel* hint = (HUDuiLabel*)list[1];
  hint->setFontSize(tinyFontSize);
  const float hintWidth = fm.getStrLength(fontFace, tinyFontSize, hint->getString());
  y -= 1.25f * fm.getStrHeight(fontFace, tinyFontSize, hint->getString());
  hint->setPosition(0.5f * ((float)width - hintWidth), y);
  y -= 1.5f * fm.getStrHeight(fontFace, fontSize, hint->getString());

  // reposition menu items (first is centered, rest aligned to the first)
  const float firstWidth = fm.getStrLength(fontFace, fontSize, ((HUDuiLabel*)list[2])->getString());
  x = 0.5f * ((float)width - firstWidth);
  const int count = list.size();
  for (int i = 2; i < count; i++) {
    HUDuiLabel* label = (HUDuiLabel*)list[i];
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

