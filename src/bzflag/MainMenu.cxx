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
  HUDuiControl* label;
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

  label = createLabel("Up/Down arrows to move, Enter to select, Esc to dismiss");
  list.push_back(label);

  join = createLabel("Join");
  list.push_back(join);

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
  control->setFont(font);
  control->setString(string);
  return control;
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
  if (focus == join) {
    if (!joinMenu) joinMenu = new JoinMenu;
    HUDDialogStack::get()->push(joinMenu);
  }
  else if (focus == options) {
    if (!optionsMenu) optionsMenu = new OptionsMenu;
    HUDDialogStack::get()->push(optionsMenu);
  }
  else if (focus == help) {
    HUDDialogStack::get()->push(HelpMenu::getHelpMenu());
  }
  else if (focus == leave) {
    leaveGame();
    // myTank should be NULL now, recreate menu
    createControls();
  }
  else if (focus == quit) {
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
  HUDuiLabel* hint = (HUDuiLabel*)list[1];
  hint->setFontSize(tinyFontWidth, tinyFontHeight);
  const OpenGLTexFont& hintFont = hint->getFont();
  const float hintWidth = hintFont.getWidth(hint->getString());
  y -= 1.25f * tinyFontHeight;
  hint->setPosition(0.5f * ((float)width - hintWidth), y);
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

