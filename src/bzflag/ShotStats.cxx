/* bzflag
 * Copyright (c) 1993-2013 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface headers */
#include "ShotStats.h"

/* common implementation headers */
#include "FontManager.h"
#include "TextUtils.h"

/* local implementation headers */
#include "ShotStatsDefaultKey.h"
#include "HUDDialogStack.h"
#include "HUDuiLabel.h"
#include "LocalPlayer.h"
#include "Roster.h"

ShotStats::ShotStats() : HUDDialog()
{
  std::vector<HUDuiControl*>& listHUD = getControls();

  // add title
  createLabel("Shot Statistics", listHUD);

  // key
  createLabel("Shots Hit/Fired", listHUD);

  columns = 11;
  rows = 0;

  // section headings (upper)
  createLabel("", listHUD);
  createLabel("", listHUD);
  createLabel("", listHUD);
  createLabel("", listHUD);
  createLabel("", listHUD);
  createLabel("", listHUD);
  createLabel("Super", listHUD);
  createLabel("Shock", listHUD);
  createLabel("", listHUD);
  createLabel("Fave.", listHUD);
  createLabel("Best", listHUD);
  ++rows;

  // section headings (lower)
  createLabel("Player", listHUD);
  createLabel("Hit%", listHUD);
  createLabel("Total", listHUD);
  createLabel("Norm", listHUD);
  createLabel("GM", listHUD);
  createLabel("Laser", listHUD);
  createLabel("Bullet", listHUD);
  createLabel("Wave", listHUD);
  createLabel("Thief", listHUD);
  createLabel("Flag", listHUD);
  createLabel("Flag", listHUD);
  ++rows;

  staticLabelCount = listHUD.size();

  initNavigation(listHUD, 1, 1);
}

ShotStats::~ShotStats()
{
}

void ShotStats::refresh()
{
  if (!visible) {
    return;
  }

  std::vector<HUDuiControl*>& listHUD = getControls();

  // Delete all the controls apart from the headings
  const int count = listHUD.size();
  for (int i = staticLabelCount; i < count; i++) {
    delete listHUD[i];
  }
  listHUD.erase(listHUD.begin() + staticLabelCount, listHUD.end());

  // my statistics first
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank->getTeam() != ObserverTeam) {
    addStats((Player*)myTank, listHUD);
  }

  // add statistics for each player
  for (int i = 0; i < curMaxPlayers; ++i) {
    if (remotePlayers[i] && (remotePlayers[i]->getTeam() != ObserverTeam)) {
      addStats((Player*)remotePlayers[i], listHUD);
    }
  }

  resize(HUDDialog::getWidth(), HUDDialog::getHeight());
}

void ShotStats::createLabel(const std::string &str,
			    std::vector<HUDuiControl*> &_list)
{
  HUDuiLabel* control = new HUDuiLabel;
  control->setFontFace(getFontFace());
  control->setString(str);
  _list.push_back(control);
}

void ShotStats::addStats(Player *_player, std::vector<HUDuiControl*> &_list)
{
  const ShotStatistics* stats = _player->getShotStatistics();
  createLabel(_player->getCallSign(), _list);

  createLabel(TextUtils::format("%2d%%", stats->getTotalPerc()), _list);
  createLabel(TextUtils::format("%d/%d", stats->getTotalHit(),
				stats->getTotalFired()),  _list);
  createLabel(TextUtils::format("%d/%d", stats->getHit(Flags::Null),
				stats->getFired(Flags::Null)), _list);
  createLabel(TextUtils::format("%d/%d", stats->getHit(Flags::GuidedMissile),
				stats->getFired(Flags::GuidedMissile)), _list);
  createLabel(TextUtils::format("%d/%d", stats->getHit(Flags::Laser),
				stats->getFired(Flags::Laser)), _list);
  createLabel(TextUtils::format("%d/%d", stats->getHit(Flags::SuperBullet),
				stats->getFired(Flags::SuperBullet)), _list);
  createLabel(TextUtils::format("%d/%d", stats->getHit(Flags::ShockWave),
				stats->getFired(Flags::ShockWave)), _list);
  createLabel(TextUtils::format("%d/%d", stats->getHit(Flags::Thief),
				stats->getFired(Flags::Thief)), _list);

  std::string flagName = stats->getFavoriteFlag()->flagAbbv;
  if (flagName.empty())
    flagName = "None";
  createLabel(flagName, _list);

  flagName = stats->getBestFlag()->flagAbbv;
  if (flagName.empty())
    flagName = "None";
  createLabel(flagName, _list);

  ++rows;
}

int			ShotStats::getFontFace()
{
  // create font
  return FontManager::instance().getFaceID(BZDB.get("sansSerifFont"));
}

HUDuiDefaultKey*	ShotStats::getDefaultKey()
{
  return ShotStatsDefaultKey::getInstance();
}

void			ShotStats::execute()
{
  HUDDialogStack::get()->pop();
}

void			ShotStats::dismiss()
{
  visible = false;
}

void			ShotStats::show()
{
  visible = true;

  refresh();
}

void			ShotStats::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);

  // Reposition everything -- that's gonna be a challenge!

  FontManager &fm = FontManager::instance();

  // set up table
  // total width / (number of columns + 3 columns extra for player name + 2 columns margin)
  const float columnWidth = _width / (columns + 5.0f);
  const float fontSize = (float) columnWidth / 6;
  const float rowHeight = fm.getStrHeight(getFontFace(), fontSize, " ") * 1.2f;

  // center title
  const float titleFontSize = (float)_height / 15.0f;
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStrLength(getFontFace(), titleFontSize, title->getString());
  const float titleHeight = fm.getStrHeight(getFontFace(), titleFontSize, " ");
  const float titleY = (float)_height - titleHeight;
  float x = 0.5f * ((float)_width - titleWidth);
  float y = titleY;
  title->setPosition(x, y);

  // center key
  HUDuiLabel* key = (HUDuiLabel*)listHUD[1];
  key->setFontSize(fontSize);
  const float keyCenter = ((columns / 2) + 4) * columnWidth;
  const float keyWidth = fm.getStrLength(getFontFace(), fontSize, key->getString());
  const float keyY = titleY - 2 * fm.getStrHeight(getFontFace(), fontSize, " ");
  y = keyY;
  x = keyCenter - 0.5f * keyWidth;
  key->setPosition(x, y);

  for (int i = 2; i < (int)listHUD.size(); ++i) {
    // determine row & column (i - 2 to account for title & key)
    int row = (i - 2) / columns;
    int column = (i - 2) - (columns * row) + 1;
    // account for 3 extra columns in player name
    if (column > 1)
      column = column + 3;

    // find coordinates corresponding to this row & column
    x = column * columnWidth;
    y = keyY - (row + 1) * rowHeight;

    // move label to the specified coordinates
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
  }

}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

