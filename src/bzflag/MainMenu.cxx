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
#include "texture.h"
#include "TextureManager.h"

/* local implementation headers */
#include "HelpMenu.h"
#include "HUDDialogStack.h"
#include "LocalPlayer.h"

/* from playing.cxx */
void leaveGame();

OpenGLTexFont*		MainMenu::mainFont = NULL;

MainMenu::MainMenu() : HUDDialog(), joinMenu(NULL),
				optionsMenu(NULL), quitMenu(NULL)
{
  // create font
  font = TextureFont::getTextureFont(TextureFont::HelveticaBold, true);
  mainFont = &font;

  // add controls
  createControls();
}

void	  MainMenu::createControls()
{
  TextureManager &tm = TextureManager::instance();
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* label;
  HUDuiTextureLabel* textureLabel;

  // clear controls
  list.erase(list.begin(), list.end());

  // load title
  int title = tm.getTextureID( "title" );
  
  // add controls
  textureLabel = new HUDuiTextureLabel;
  textureLabel->setFont(font);
  textureLabel->setTexture(title);
  textureLabel->setString("BZFlag");
  list.push_back(textureLabel);

  label = new HUDuiLabel;
  label->setFont(font);
  label->setString("Up/Down arrows to move, Enter to select, Esc to dismiss");
  list.push_back(label);

  label = new HUDuiLabel;
  label->setFont(font);
  label->setString("Join Game");
  list.push_back(label);

  label = new HUDuiLabel;
  label->setFont(font);
  label->setString("Options");
  list.push_back(label);

  label = new HUDuiLabel;
  label->setFont(font);
  label->setString("Help");
  list.push_back(label);

  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (!(myTank == NULL)) {
    label = new HUDuiLabel;
    label->setFont(font);
    label->setString("Leave Game");
    list.push_back(label);
  }

  label = new HUDuiLabel;
  label->setFont(font);
  label->setString("Quit");
  list.push_back(label);

  resize(HUDDialog::getWidth(), HUDDialog::getHeight());
  initNavigation(list, 2, list.size() - 1);

  // set focus back at the top in case the item we had selected does not exist anymore
  list[2]->setFocus();
}

MainMenu::~MainMenu()
{
  mainFont = NULL;
  delete joinMenu;
  delete optionsMenu;
  delete quitMenu;
  HelpMenu::done();
}

const OpenGLTexFont&	MainMenu::getFont()
{
  return *mainFont;
}

HUDuiDefaultKey*	MainMenu::getDefaultKey()
{
  return MenuDefaultKey::getInstance();
}

void			MainMenu::execute()
{
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiControl* focus = HUDui::getFocus();
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (focus == list[2]) {
    if (!joinMenu) joinMenu = new JoinMenu;
    HUDDialogStack::get()->push(joinMenu);
  }
  else if (focus == list[3]) {
    if (!optionsMenu) optionsMenu = new OptionsMenu;
    HUDDialogStack::get()->push(optionsMenu);
  }
  else if (focus == list[4]) {
    HUDDialogStack::get()->push(HelpMenu::getHelpMenu());
  }
  // this menu item only exists if you're connected to a game
  else if ((focus == list[5]) && (myTank != NULL)) {
    leaveGame();
    // myTank should be NULL now, recreate menu
    createControls();
  }
  else if (focus == list[list.size() - 1]) {
    if (!quitMenu) quitMenu = new QuitMenu;
    HUDDialogStack::get()->push(quitMenu);
  }
}

void			MainMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  // use a big font
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;
  const float fontWidth = (float)height / 12.0f;
  const float fontHeight = (float)height / 12.0f;
  const float tinyFontWidth = (float)height / 36.0f;
  const float tinyFontHeight = (float)height / 36.0f;

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontWidth, titleFontHeight);
  const OpenGLTexFont& titleFont = title->getFont();
  const float titleWidth = titleFont.getWidth(title->getString());
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleFont.getHeight();
  title->setPosition(x, y);

  // reposition instructions
  HUDuiLabel* help = (HUDuiLabel*)list[1];
  help->setFontSize(tinyFontWidth, tinyFontHeight);
  const OpenGLTexFont& helpFont = help->getFont();
  const float helpWidth = helpFont.getWidth(help->getString());
  y -= 1.25f * tinyFontHeight;
  help->setPosition(0.5f * ((float)width - helpWidth), y);
  y -= 1.5f * fontHeight;

  // reposition menu items
  x += 0.5f * fontHeight;
  const int count = list.size();
  for (int i = 2; i < count; i++) {
    HUDuiLabel* label = (HUDuiLabel*)list[i];
    label->setFontSize(fontWidth, fontHeight);
    label->setPosition(x, y);
    y -= 1.2f * fontHeight;
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

