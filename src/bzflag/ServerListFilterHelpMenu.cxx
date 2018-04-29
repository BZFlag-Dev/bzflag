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
#include "ServerListFilterHelpMenu.h"

// system headers
#include <string.h>

/* common implementation headers */
#include "KeyManager.h"
#include "Flag.h"
#include "FontManager.h"

/* local implementation headers */
#include "HUDDialogStack.h"
#include "HUDuiLabel.h"
#include "HUDuiTypeIn.h"
#include "MainMenu.h"
#include "ServerListFilter.h"


bool ServerListFilterHelpMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  if (key.button == BzfKeyEvent::PageUp) {
    HUDDialog* oldMenu = HUDDialogStack::get()->top();
    HUDDialogStack::get()->pop();
    HUDDialogStack::get()->push(ServerListFilterHelpMenu::getServerListFilterHelpMenu(oldMenu, false));
    return true;
  }
  if (key.button == BzfKeyEvent::PageDown || key.ascii == 13) {
    HUDDialog* oldMenu = HUDDialogStack::get()->top();
    HUDDialogStack::get()->pop();
    HUDDialogStack::get()->push(ServerListFilterHelpMenu::getServerListFilterHelpMenu(oldMenu, true));
    return true;
  }
  return MenuDefaultKey::keyPress(key);
}

bool ServerListFilterHelpMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  if (key.button == BzfKeyEvent::PageUp ||
      key.button == BzfKeyEvent::PageDown || key.ascii == 13)
    return true;
  return MenuDefaultKey::keyRelease(key);
}


ServerListFilterHelpMenu::ServerListFilterHelpMenu(const char* title) : HUDDialog()
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel(title));
  listHUD.push_back(createLabel("Page Down for next page",
			     "Page Up for previous page"));


  initNavigation(listHUD, 1, 1);
}

HUDuiControl* ServerListFilterHelpMenu::createLabel(const char* string,
				    const char* label)
{
  HUDuiLabel* control = new HUDuiLabel;
  control->setFontFace(MainMenu::getFontFace());
  if (string) control->setString(string);
  if (label) control->setLabel(label);
  return control;
}

HUDuiControl* ServerListFilterHelpMenu::createInput(const std::string &label)
{
  HUDuiTypeIn* entry = new HUDuiTypeIn;
  entry->setFontFace(MainMenu::getFontFace());
  entry->setLabel(label);
  entry->setMaxLength(42);
  entry->setColorFunc(ServerListFilter::colorizeSearch);
  return entry;
}

float ServerListFilterHelpMenu::getLeftSide(int _width, int)
{
  return (float)_width * 0.1f;
}

void ServerListFilterHelpMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)_height / 23.0f;
  const float navFontSize = (float)_height / 60.0f;
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

  // How many lines of help text do we have?
  const int count = listHUD.size();

  // Determine maximum height we have to work with
  float maxHelpHeight = (float)_height - (titleHeight * 3);

  // Determine maximum font size based on that height
  float maxFontSizeForHeight = 0;
  int divisor = 40;
  do {
    divisor += 3;
    maxFontSizeForHeight = (float)_height / divisor;
  } while (
    // Minimize size of 1/100th of the screen height
    divisor < 100 &&
    // Try for fitting it to the available space
    fm.getStrHeight(MainMenu::getFontFace(), maxFontSizeForHeight, " ") * (count - 2) > maxHelpHeight
  );

  // Determine the maximum width we have to work with
  float maxHelpWidth = (float)_width * 0.9f;

  // Find the longest line
  int longestLine = 2;
  float longestLineWidth = fm.getStrLength(MainMenu::getFontFace(), maxFontSizeForHeight, ((HUDuiLabel*)listHUD[1])->getString());
  for (int i = 2; i < count; i++) {
    float lineWidth = fm.getStrLength(MainMenu::getFontFace(), maxFontSizeForHeight, ((HUDuiLabel*)listHUD[i])->getString());
    if (lineWidth > longestLineWidth) {
      longestLine = i;
      longestLineWidth = lineWidth;
    }
  }

  // Determine the maximum font size based on the available width.
  float fontSize = maxFontSizeForHeight;
  // Reusing 'divisor' and 'longestLineWidth' from above
  while (
    // Minimize size of 1/100th of the screen height
    divisor < 100 &&
    // Try for fitting it to the available space
    longestLineWidth > maxHelpWidth
  ) {
    divisor += 3;
    fontSize = (float)_height / divisor;
    longestLineWidth = fm.getStrLength(MainMenu::getFontFace(), fontSize, ((HUDuiLabel*)listHUD[longestLine])->getString());
  }

  // position focus holder off screen
  listHUD[1]->setFontSize(navFontSize);
  const float nh = fm.getStrHeight(MainMenu::getFontFace(), navFontSize, " ");
  y -= 1.25f * nh;
  listHUD[1]->setPosition(0.5f * ((float)_width + nh), y);

  // reposition options
  const float h = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
  x = ((float)_width - longestLineWidth) / 2;
  y -= 1.5f * h;

  for (int i = 2; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    y -= h;
  }
}



//
// ServerListFilterHelp1Menu
//

class ServerListFilterHelp1Menu : public ServerListFilterHelpMenu {
public:
  ServerListFilterHelp1Menu();
  ~ServerListFilterHelp1Menu() {}
};

ServerListFilterHelp1Menu::ServerListFilterHelp1Menu() : ServerListFilterHelpMenu("Structure of a filter (Part 1)")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("By default, when using the \"search\" function in the server list, it will search for the"));
  listHUD.push_back(createLabel("exact string of text you provided in both the server address and the description. Only"));
  listHUD.push_back(createLabel("servers that have that string in the address or description will be shown."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("But there are also powerful filters you can use. These allow filtering the server list"));
  listHUD.push_back(createLabel("based on criteria such as player counts, shot count, address, description, and"));
  listHUD.push_back(createLabel("various server settings (such as jumping and ricochet). For instance, this filter"));
  listHUD.push_back(createLabel("would find any 2 or 3 shot servers that have at least one player online:"));
  listHUD.push_back(createLabel("    /p>1,s>1,s<4,# busy - 2 or 3 shots"));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Notice that the line begins with a forward slash. This tells the server list that this"));
  listHUD.push_back(createLabel("is an advanced filter. Multiple statements after the slash are then split apart by a"));
  listHUD.push_back(createLabel("comma, letting the filter know they are separate items. These are combined together"));
  listHUD.push_back(createLabel("with an \"and\" statement internally. So, with the example above, you can think of it as"));
  listHUD.push_back(createLabel("\"Player count must be greater than 1 AND shot count must be greater than 1 AND"));
  listHUD.push_back(createLabel("shot count must be less than 4.\""));
}

//
// ServerListFilterHelp2Menu
//

class ServerListFilterHelp2Menu : public ServerListFilterHelpMenu {
public:
  ServerListFilterHelp2Menu();
  ~ServerListFilterHelp2Menu() {}
};

ServerListFilterHelp2Menu::ServerListFilterHelp2Menu() : ServerListFilterHelpMenu("Structure of a filter (Part 2)")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("Multiple sets of filters can be listed by starting another set with a forward slash."));
  listHUD.push_back(createLabel("These separate filter \"sets\" will be handled independently, so any servers that"));
  listHUD.push_back(createLabel("match any of the filter sets show up (effectively meaning they are joined with an"));
  listHUD.push_back(createLabel("\"or\" statement internally)."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Filters are currently limited to 42 characters due to limitations with the menu"));
  listHUD.push_back(createLabel("system. Future versions of the game should be able to lift this restriction."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("The next pages will describe the various filters. The text within the inital set of"));
  listHUD.push_back(createLabel("parenthesis for each item will include the short and longer forms of the filter."));
  listHUD.push_back(createLabel("Either can be used in your filter, but the shorter form is recommended. Also, the"));
  listHUD.push_back(createLabel("filters are case sensitive, so make sure you write them exactly as shown."));
}


//
// ServerListFilterHelp3Menu
//

class ServerListFilterHelp3Menu : public ServerListFilterHelpMenu {
public:
  ServerListFilterHelp3Menu();
  ~ServerListFilterHelp3Menu() {}
};

ServerListFilterHelp3Menu::ServerListFilterHelp3Menu() : ServerListFilterHelpMenu("Pattern filters (Part 1)")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("At present, you are limited to having only one of each of the following per filter set."));
  listHUD.push_back(createLabel(" If you use two or more within a single filter set, only the last one will take effect."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("There are two possible operators that would immediately follow the filter name:"));
  listHUD.push_back(createLabel(") glob: A pattern with wildcards in the form of an asterisk. If no asterisk is"));
  listHUD.push_back(createLabel("    specifically included, it will surround the search string with asterisks. If"));
  listHUD.push_back(createLabel("    you include a wildcard, it will not automatically surround the search string"));
  listHUD.push_back(createLabel("    with asterisks."));
  listHUD.push_back(createLabel("] regex: Uses regular expressions. This allows for complex patterns, but the creation"));
  listHUD.push_back(createLabel("    of these expressions is beyond the scope of this document."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Filters:"));
  listHUD.push_back(createLabel("  Address (a or addr or address) - Looks for the pattern inside the address (hostname"));
  listHUD.push_back(createLabel("    and port)"));
  listHUD.push_back(createLabel("  Description (d or desc or description) - Looks for the pattern inside the server"));
  listHUD.push_back(createLabel("    description"));
  listHUD.push_back(createLabel("  Address/Description (ad or addrdesc) - Looks for the pattern in either the address"));
  listHUD.push_back(createLabel("    or description"));
}



//
// ServerListFilterHelp4Menu
//

class ServerListFilterHelp4Menu : public ServerListFilterHelpMenu {
public:
  ServerListFilterHelp4Menu();
  ~ServerListFilterHelp4Menu() {}
};

ServerListFilterHelp4Menu::ServerListFilterHelp4Menu() : ServerListFilterHelpMenu("Pattern filters (Part 2)")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("Here are some examples of pattern filters."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Find servers running on port 5155 or 5156:"));
  listHUD.push_back(createLabel(ServerListFilter::colorizeSearch("  /a]515[56]").c_str()));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Find servers with \"league\" in the description:"));
  listHUD.push_back(createLabel(ServerListFilter::colorizeSearch("  /d)league").c_str()));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Find servers with a description starting with \"league\":"));
  listHUD.push_back(createLabel(ServerListFilter::colorizeSearch("  /d)league*").c_str()));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Find servers with \"official\" and \"match\" in the description, in that order:"));
  listHUD.push_back(createLabel(ServerListFilter::colorizeSearch("  /d)*official*match*").c_str()));
}



//
// ServerListFilterHelp5Menu
//

class ServerListFilterHelp5Menu : public ServerListFilterHelpMenu {
public:
  ServerListFilterHelp5Menu();
  ~ServerListFilterHelp5Menu() {}
};

ServerListFilterHelp5Menu::ServerListFilterHelp5Menu() : ServerListFilterHelpMenu("Boolean filters (Part 1)")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("Boolean filters are either true or false. You prefix the filter name with either a +"));
  listHUD.push_back(createLabel("or a -. If a +, it will match if the boolean is true. If -, it will match if the"));
  listHUD.push_back(createLabel("boolean is false."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Game mode filters (include at most one positive game mode filter per filter set,"));
  listHUD.push_back(createLabel("otherwise you will find nothing):"));
  listHUD.push_back(createLabel("  FFA (F or ffa) - Server is running Free-For-All with teams"));
  listHUD.push_back(createLabel("  OpenFFA (O or offa) - Server is running Open Free-For-All without teams"));
  listHUD.push_back(createLabel("  CTF (C or ctf) - Server is running Capture-The-Flag"));
  listHUD.push_back(createLabel("  Rabbit (R or rabbit) - Server is running Rabbit Chase"));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Miscellaneous filters:"));
  listHUD.push_back(createLabel("  Jump (j or jump) - Tanks can jump without extra flags"));
  listHUD.push_back(createLabel("  Ricochet (r or rico) - Bullets ricochet off obstacles"));
  listHUD.push_back(createLabel("  Handicap (h or handicap) - Tank performance is affected by score"));
  listHUD.push_back(createLabel("  Replay (P or replay) - Server is a replay server"));
  listHUD.push_back(createLabel("  Inertia (I or inertia) - Tanks have inertia"));
  listHUD.push_back(createLabel("  Antidote (a or antidote) - Antidote flags spawn to drop bad flags"));
  listHUD.push_back(createLabel("  Favorite (F or favorite) - Server is on your favorites list"));
}



//
// ServerListFilterHelp6Menu
//

class ServerListFilterHelp6Menu : public ServerListFilterHelpMenu {
public:
  ServerListFilterHelp6Menu();
  ~ServerListFilterHelp6Menu() {}
};

ServerListFilterHelp6Menu::ServerListFilterHelp6Menu() : ServerListFilterHelpMenu("Boolean filters (Part 2)")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("Here are some examples of boolean filters."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Find FFA servers in your favorites list:"));
  listHUD.push_back(createLabel(ServerListFilter::colorizeSearch("  /+ffa,+favorite").c_str()));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Find servers that allow jumping but where bullets do not ricochet:"));
  listHUD.push_back(createLabel(ServerListFilter::colorizeSearch("  /+j,-r").c_str()));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Show only CTF and rabbit chase servers:"));
  listHUD.push_back(createLabel(ServerListFilter::colorizeSearch("  /-ffa,-offa").c_str()));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Use multiple filter sets to show only CTF and rabbit chase servers:"));
  listHUD.push_back(createLabel(ServerListFilter::colorizeSearch("  /+ctf/+rabbit").c_str()));
}



//
// ServerListFilterHelp7Menu
//

class ServerListFilterHelp7Menu : public ServerListFilterHelpMenu {
public:
  ServerListFilterHelp7Menu();
  ~ServerListFilterHelp7Menu() {}
};

ServerListFilterHelp7Menu::ServerListFilterHelp7Menu() : ServerListFilterHelpMenu("Range filters (part 1)")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("Range filters limit some value to a numeric boundary. It could be an exact value,"));
  listHUD.push_back(createLabel("or have a minimum and/or maximum. The format of a range filter is:"));
  listHUD.push_back(createLabel("  <filter><operator><number>."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Possible operators:"));
  listHUD.push_back(createLabel("  <  less than"));
  listHUD.push_back(createLabel("  <= less than or equal to"));
  listHUD.push_back(createLabel("  >  greater than"));
  listHUD.push_back(createLabel("  >= greater than or equal to"));
  listHUD.push_back(createLabel("  =  equal to"));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Miscellaneous filters:"));
  listHUD.push_back(createLabel("  Shots (s or shots) - Maximum number of active shots per tank"));
  listHUD.push_back(createLabel("  Players (p or players) - Current number of players (excluding observers)"));
  listHUD.push_back(createLabel("  Free slots (f or freeSlots) - Available player slots (excluding observers)"));
  listHUD.push_back(createLabel("  Valid teams (vt or validTeams) - Number of teams enabled (excluding observers)"));
  listHUD.push_back(createLabel("  Max time (mt or maxTime) - Time limit on the game"));
  listHUD.push_back(createLabel("  Max players (mp or maxPlayers) - Maximum player count"));
  listHUD.push_back(createLabel("  Max team score (mts or maxTeamScore) - Team score that will end the game"));
  listHUD.push_back(createLabel("  Max player score (mps or maxPlayerScore) - Player score that will end the game"));
  listHUD.push_back(createLabel("  Shake wins (sw or shakeWins) - Number of kills to drop a bad flag"));
  listHUD.push_back(createLabel("  Shake time (st or shakeTime) - Number of seconds to drop a bad flag"));
}


//
// ServerListFilterHelp8Menu
//

class ServerListFilterHelp8Menu : public ServerListFilterHelpMenu {
public:
  ServerListFilterHelp8Menu();
  ~ServerListFilterHelp8Menu() {}
};

ServerListFilterHelp8Menu::ServerListFilterHelp8Menu() : ServerListFilterHelpMenu("Range filters (part 2)")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("Current amount of players by team:"));
  listHUD.push_back(createLabel("  Rogue count (Rp or roguePlayers)"));
  listHUD.push_back(createLabel("  Red count (rp or redPlayers)"));
  listHUD.push_back(createLabel("  Green count (gp or greenPlayers)"));
  listHUD.push_back(createLabel("  Blue count (bp or bluePlayers)"));
  listHUD.push_back(createLabel("  Purple count (pp or purplePlayers)"));
  listHUD.push_back(createLabel("  Observer count (op or observerPlayers)"));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Maximum amount of players allowed on a server by team:"));
  listHUD.push_back(createLabel("  Rogue max (Rm or rogueMax)"));
  listHUD.push_back(createLabel("  Red max (rm or redMax)"));
  listHUD.push_back(createLabel("  Green max (gm or greenMax)"));
  listHUD.push_back(createLabel("  Blue max (bm or blueMax)"));
  listHUD.push_back(createLabel("  Purple max (pm or purpleMax)"));
  listHUD.push_back(createLabel("  Observer max (om or observerMax)"));
}


//
// ServerListFilterHelp9Menu
//

class ServerListFilterHelp9Menu : public ServerListFilterHelpMenu {
public:
  ServerListFilterHelp9Menu();
  ~ServerListFilterHelp9Menu() {}
};

ServerListFilterHelp9Menu::ServerListFilterHelp9Menu() : ServerListFilterHelpMenu("Range filters (part 3)")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("Available player slots by team:"));
  listHUD.push_back(createLabel("  Rogue free (Rf or rogueFree)"));
  listHUD.push_back(createLabel("  Red free (rf or redFree)"));
  listHUD.push_back(createLabel("  Green free (gf or greenFree)"));
  listHUD.push_back(createLabel("  Blue free (bf or blueFree)"));
  listHUD.push_back(createLabel("  Purple free (pf or purpleFree)"));
  listHUD.push_back(createLabel("  Observer free (of or observerFree)"));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Here are some examples of range filters."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Find servers with observers online:"));
  listHUD.push_back(createLabel(ServerListFilter::colorizeSearch("  /op>0").c_str()));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Find servers with between 1 and 5 players:"));
  listHUD.push_back(createLabel(ServerListFilter::colorizeSearch("  /p>=1,p<=5").c_str()));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("More than 3 shots, at least 5 players, and no rogue allowed:"));
  listHUD.push_back(createLabel(ServerListFilter::colorizeSearch("  /s>3,p>=5,Rm=0").c_str()));
}


//
// ServerListFilterHelp10Menu
//

class ServerListFilterHelp10Menu : public ServerListFilterHelpMenu {
public:
  ServerListFilterHelp10Menu();
  ~ServerListFilterHelp10Menu() {}
};

ServerListFilterHelp10Menu::ServerListFilterHelp10Menu() : ServerListFilterHelpMenu("Comments")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("Comments begin with a # to document the purpose of a filter. This is entirely optional."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Example:"));
  listHUD.push_back(createLabel(ServerListFilter::colorizeSearch("  /op>3,# More than 3 observers").c_str()));
}


//
// ServerListFilterHelp11Menu
//

class ServerListFilterHelp11Menu : public ServerListFilterHelpMenu {
public:
  ServerListFilterHelp11Menu();
  ~ServerListFilterHelp11Menu() {}
};

ServerListFilterHelp11Menu::ServerListFilterHelp11Menu() : ServerListFilterHelpMenu("Combined examples")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("Now we will combine the various filter types to show some more advanced examples."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Find CTF maps with two teams:"));
  listHUD.push_back(createLabel(ServerListFilter::colorizeSearch("  /vt=2,+ctf").c_str()));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Find league servers with at least one player or one observer (notice the second forward"));
  listHUD.push_back(createLabel("slash that starts a new filter set):"));
  listHUD.push_back(createLabel(ServerListFilter::colorizeSearch("  /d)league,p>0/d)league,op>0").c_str()));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Find a rabbit chase server with ricochet and a shot count of 3:"));
  listHUD.push_back(createLabel(ServerListFilter::colorizeSearch("  /+rabbit,+rico,shots=3").c_str()));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Short name version of the above filter (notice the capital R for rabbit chase):"));
  listHUD.push_back(createLabel(ServerListFilter::colorizeSearch("  /+R,+r,s=3").c_str()));
}


//
// help menu getter
//

static const int numServerListFilterHelpMenus = 11;
ServerListFilterHelpMenu** ServerListFilterHelpMenu::serverListFilterHelpMenus = NULL;

ServerListFilterHelpMenu* ServerListFilterHelpMenu::getServerListFilterHelpMenu(HUDDialog* dialog, bool next)
{
  if (!serverListFilterHelpMenus) {
    serverListFilterHelpMenus = new ServerListFilterHelpMenu*[numServerListFilterHelpMenus];
    serverListFilterHelpMenus[0] = new ServerListFilterHelp1Menu;
    serverListFilterHelpMenus[1] = new ServerListFilterHelp2Menu;
    serverListFilterHelpMenus[2] = new ServerListFilterHelp3Menu;
    serverListFilterHelpMenus[3] = new ServerListFilterHelp4Menu;
    serverListFilterHelpMenus[4] = new ServerListFilterHelp5Menu;
    serverListFilterHelpMenus[5] = new ServerListFilterHelp6Menu;
    serverListFilterHelpMenus[6] = new ServerListFilterHelp7Menu;
    serverListFilterHelpMenus[7] = new ServerListFilterHelp8Menu;
    serverListFilterHelpMenus[8] = new ServerListFilterHelp9Menu;
    serverListFilterHelpMenus[9] = new ServerListFilterHelp10Menu;
    serverListFilterHelpMenus[10] = new ServerListFilterHelp11Menu;
  }
  for (int i = 0; i < numServerListFilterHelpMenus; i++) {
    if (dialog == serverListFilterHelpMenus[i]) {
      if (next) {
	return serverListFilterHelpMenus[(i + 1) % numServerListFilterHelpMenus];
      } else {
	return serverListFilterHelpMenus[(i - 1 + numServerListFilterHelpMenus) % numServerListFilterHelpMenus];
      }
    }
  }
  return next ? serverListFilterHelpMenus[0] : serverListFilterHelpMenus[numServerListFilterHelpMenus - 1];
}

void			ServerListFilterHelpMenu::done()
{
  if (serverListFilterHelpMenus) {
    for (int i = 0; i < numServerListFilterHelpMenus; i++) {
      delete serverListFilterHelpMenus[i];
    }
    delete[] serverListFilterHelpMenus;
    serverListFilterHelpMenus = NULL;
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
