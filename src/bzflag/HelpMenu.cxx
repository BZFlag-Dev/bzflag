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
#include "HelpMenu.h"

/* common implementation headers */
#include "Flag.h"
#include "FontManager.h"

/* local implementation headers */
#include "HUDDialogStack.h"
#include "HUDuiLabel.h"
#include "MainMenu.h"

/* help menus */
#include "HelpKeymapMenu.h"
#include "HelpCreditsMenu.h"
#include "HelpInstructionsMenu.h"
#include "HelpFlagsMenu.h"


/* local functions to generate the text for instructional help */
static std::vector<std::string> GetGeneralText();
static std::vector<std::string> GetEnvironmentText();
static std::vector<std::string> GetFlagsText();
static std::vector<std::string> GetReadoutsIText();
static std::vector<std::string> GetReadoutsIIText();


bool HelpMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  if (key.button == BzfKeyEvent::PageUp) {
    HUDDialog* oldMenu = HUDDialogStack::get()->top();
    HUDDialogStack::get()->pop();
    HUDDialogStack::get()->push(HelpMenu::getHelpMenu(oldMenu, false));
    return true;
  }
  if (key.button == BzfKeyEvent::PageDown || key.ascii == 13) {
    HUDDialog* oldMenu = HUDDialogStack::get()->top();
    HUDDialogStack::get()->pop();
    HUDDialogStack::get()->push(HelpMenu::getHelpMenu(oldMenu, true));
    return true;
  }
  return MenuDefaultKey::keyPress(key);
}

bool HelpMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  if (key.button == BzfKeyEvent::PageUp ||
      key.button == BzfKeyEvent::PageDown || key.ascii == 13)
    return true;
  return MenuDefaultKey::keyRelease(key);
}


HelpMenu::HelpMenu(const char* title) : HUDDialog()
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel(title));
  listHUD.push_back(createLabel("Page Down for next page",
			     "Page Up for previous page"));


  initNavigation(listHUD, 1, 1);
}

HUDuiControl* HelpMenu::createLabel(const char* string,
				    const char* label)
{
  HUDuiLabel* control = new HUDuiLabel;
  control->setFontFace(MainMenu::getFontFace());
  if (string) control->setString(string);
  if (label) control->setLabel(label);
  return control;
}

float HelpMenu::getLeftSide(int, int _height)
{
  return (float)_height / 6.0f;
}

void HelpMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)_height / 23.0f;
  const float fontSize = (float)_height / 100.0f;
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

  // position focus holder off screen
  listHUD[1]->setFontSize(fontSize);
  const float h = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
  y -= 1.25f * h;
  listHUD[1]->setPosition(0.5f * ((float)_width + h), y);

  // reposition options
  x = getLeftSide(_width, _height);
  y -= 1.5f * h;
  const int count = (const int)listHUD.size();
  for (int i = 2; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    y -= 1.0f * h;
  }
}


//
// help menu getter
//

static const int numHelpMenus = 9;
HelpMenu** HelpMenu::helpMenus = NULL;

HelpMenu* HelpMenu::getHelpMenu(HUDDialog* dialog, bool next)
{
  if (!helpMenus) {
    helpMenus = new HelpMenu*[numHelpMenus];
    helpMenus[0] = new HelpKeymapMenu;
    helpMenus[1] = new HelpInstructionsMenu("General", GetGeneralText());
    helpMenus[2] = new HelpInstructionsMenu("Environment", GetEnvironmentText());
    helpMenus[3] = new HelpInstructionsMenu("Flags", GetFlagsText());
    helpMenus[4] = new HelpFlagsMenu(FlagGood);
    helpMenus[5] = new HelpFlagsMenu(FlagBad);
    helpMenus[6] = new HelpInstructionsMenu("Readouts I", GetReadoutsIText());
    helpMenus[7] = new HelpInstructionsMenu("Readouts II", GetReadoutsIIText());
    helpMenus[8] = new HelpCreditsMenu;
  }
  for (int i = 0; i < numHelpMenus; i++) {
    if (dialog == helpMenus[i]) {
      if (next) {
	return helpMenus[(i + 1) % numHelpMenus];
      } else {
	return helpMenus[(i - 1 + numHelpMenus) % numHelpMenus];
      }
    }
  }
  return next ? helpMenus[0] : helpMenus[numHelpMenus - 1];
}

void			HelpMenu::done()
{
  if (helpMenus) {
    for (int i = 0; i < numHelpMenus; i++) {
      delete helpMenus[i];
    }
    delete[] helpMenus;
    helpMenus = NULL;
  }
}


/*
 * Instructional Help Text
 */
static std::vector<std::string> GetGeneralText()
{
  std::vector<std::string> retval;
  retval.push_back("BZFlag is a multi-player networked tank battle game.  There are five teams:");
  retval.push_back("red, green, blue, purple, and rogues (rogue tanks are black).  Destroying a");
  retval.push_back("player on another team scores a win, while being destroyed or destroying a");
  retval.push_back("teammate scores a loss.  Individual and aggregate team scores are tallied.");
  retval.push_back("Rogues have no teammates (not even other rogues), so they cannot shoot");
  retval.push_back("teammates and they don't have a team score.");
  retval.push_back("");
  retval.push_back("There are three styles of play, determined by the server configuration:  capture-");
  retval.push_back("the-flag, rabbit-chase and free-for-all.  In free-for-all the object is simply to get the");
  retval.push_back("highest score by shooting opponents.  In rabbit chase, the white tank tries to stay alive");
  retval.push_back("while all other tanks try to hunt and kill it. The object in capture-the-flag is to");
  retval.push_back("capture enemy flags while preventing opponents from capturing yours.  In this");
  retval.push_back("style, each team (but not rogues) has a team base and each team with at least");
  retval.push_back("one player has a team flag which has the color of the team.  To capture a flag,");
  retval.push_back("you must grab it and bring it back to your team base (you must be on the ground");
  retval.push_back("in your base to register the capture).  Capturing a flag destroys all the players");
  retval.push_back("on that team and gives your team score a bonus;  the players will restart on");
  retval.push_back("their team base.  Taking your flag onto an enemy base counts as a capture against");
  retval.push_back("your team but not for the enemy team.");
  return retval;
}

static std::vector<std::string> GetEnvironmentText()
{
  std::vector<std::string> retval;
  retval.push_back("The world environment contains an outer wall and several buildings.");
  retval.push_back("You cannot go outside the outer wall (you can't even jump over it).");
  retval.push_back("You cannot normally drive or shoot through buildings.");
  retval.push_back("");
  retval.push_back("The server may be configured to include teleporters:  large transparent");
  retval.push_back("black slabs.  Objects entering one side of a teleporter are instantly");
  retval.push_back("moved to one side of another (or possibly the same) teleporter.  The");
  retval.push_back("teleport is reversible;  reentering the same side of the destination");
  retval.push_back("teleporter brings you back to where you started.  Teleport connections");
  retval.push_back("are fixed at the start of the game and don't change during the game.");
  retval.push_back("The connections are always the same in the capture-the-flag style.");
  retval.push_back("Each side of a teleporter teleports independently of the other side.");
  retval.push_back("It's possible for a teleporter to teleport to the opposite side of");
  retval.push_back("itself.  Such a thru-teleporter acts almost as if it wasn't there.");
  retval.push_back("A teleporter can also teleport to the same side of itself.  This is a");
  retval.push_back("reverse teleporter.  Shooting at a reverse teleporter is likely to be");
  retval.push_back("self destructive;  shooting a laser at one is invariably fatal.");
  return retval;
}

static std::vector<std::string> GetFlagsText()
{
  std::vector<std::string> retval;
  retval.push_back("Flags come in two varieties:  team flags and super flags.  Team flags");
  retval.push_back("are used only in the capture-the-flag style.  The server may also be");
  retval.push_back("configured to supply super flags, which give your tank some advantage");
  retval.push_back("or disadvantage.  You normally can't tell which until you pick one up,");
  retval.push_back("but good flags generally outnumber bad flags two to one.");
  retval.push_back("");
  retval.push_back("Team flags are not allowed to be in Bad Places.  Bad Places are:  on");
  retval.push_back("a building or on an enemy base.  Team flags dropped in a Bad Place are");
  retval.push_back("moved to a safety position.  Captured flags are placed back on their");
  retval.push_back("team base.  Super flags dropped above a building always disappear.");
  retval.push_back("");
  retval.push_back("A random good super flag will remain for up to 4 possessions.  After");
  retval.push_back("that it'll disappear and will eventually be replaced by a new random");
  retval.push_back("flag.  Bad random super flags disappear after the first possession.");
  retval.push_back("Bad super flags can't normally be dropped.  The server can be set to");
  retval.push_back("automatically drop the flag for you after some time, after you destroy");
  retval.push_back("a certain number of enemies, and/or when you grab an antidote flag.");
  retval.push_back("Antidote flags are yellow and only appear when you have a bad flag.");
  return retval;
}

static std::vector<std::string> GetReadoutsIText()
{
  std::vector<std::string> retval;
  retval.push_back("The radar is on the left side of the control panel.  It shows an overhead");
  retval.push_back("x-ray view of the game.  Buildings and the outer wall are shown in light");
  retval.push_back("blue.  Team bases are outlined in the team color.  Teleporters are short");
  retval.push_back("yellow lines.  Tanks are dots in the tank's team color, except rogues are");
  retval.push_back("yellow.  The size of the tank's dot is a rough indication of the tank's");
  retval.push_back("altitude:  higher tanks have larger dots.  Flags are small crosses.  Team");
  retval.push_back("flags are in the team color, superflags are white, and the antidote flag");
  retval.push_back("is yellow.  Shots are small dots (or lines or circles, for lasers and");
  retval.push_back("shock waves, respectively).  Your tank is always dead center and forward");
  retval.push_back("is always up on the radar.  The yellow V is your field of view.  North");
  retval.push_back("is indicated by the letter N.");
  retval.push_back("");
  retval.push_back("The heads-up-display (HUD) has several displays.  The two boxes in the");
  retval.push_back("center of the view are the motion control boxes;  within the small box");
  retval.push_back("your tank won't move, outside the large box you don't move any faster than");
  retval.push_back("at the edge of the large box.  Moving the mouse above or below the small");
  retval.push_back("box moves forward or backward, respectively.  Similarly for left and right.");
  retval.push_back("The distance away from the small box determines the speed.");
  return retval;
}

static std::vector<std::string> GetReadoutsIIText()
{
  std::vector<std::string> retval;
  retval.push_back("Above the larger box is a tape showing your current heading.  North is");
  retval.push_back("0, east is 90, etc.  If jumping is allowed or you have the jumping flag,");
  retval.push_back("an altitude tape appears to the right of the larger box.");
  retval.push_back("");
  retval.push_back("Small colored diamonds or arrows may appear on the heading tape.  An");
  retval.push_back("arrow pointing left means that a particular flag is to your left, an");
  retval.push_back("arrow pointing right means that the flag is to your right, and a diamond");
  retval.push_back("indicates the heading to the flag by its position on the heading tape.");
  retval.push_back("In capture-the-flag mode a marker always shows where your team flag is.");
  retval.push_back("A yellow marker shows the way to the antidote flag.");
  retval.push_back("");
  retval.push_back("At the top of the display are, from left to right, your callsign and");
  retval.push_back("score, your status, and the flag you have.  Your callsign is in the");
  retval.push_back("color of your team.  Your status is one of:  ready, dead, sealed, zoned");
  retval.push_back("or reloading (showing the time until reloaded).  It can also show the");
  retval.push_back("time until a bad flag is dropped (if there's a time limit).");
  retval.push_back("");
  retval.push_back("Other informational messages may occasionally flash on the HUD.");
  return retval;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
