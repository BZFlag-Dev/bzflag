/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
#include "FontSizer.h"
#include "ShotStatsDefaultKey.h"
#include "HUDDialogStack.h"
#include "HUDuiLabel.h"
#include "LocalPlayer.h"
#include "Roster.h"
#include "LocalFontFace.h"


ShotStats::ShotStats() : HUDDialog()
{
  // create font
  fontFace = LocalFontFace::create("sansSerifFont");

  // add title
  createLabel("Shot Statistics");

  // key
  createLabel("Shots Hit/Fired", true);

  columns = 11;
  rows = 0;

  // section headings (upper)
  createLabel("");
  createLabel("");
  createLabel("");
  createLabel("");
  createLabel("");
  createLabel("");
  createLabel("Super");
  createLabel("Shock");
  createLabel("");
  createLabel("Fave.");
  createLabel("Best");
  ++rows;

  // section headings (lower)
  createLabel("Player");
  createLabel("Hit%");
  createLabel("Total");
  createLabel("Norm");
  createLabel("GM");
  createLabel("Laser");
  createLabel("Bullet");
  createLabel("Wave");
  createLabel("Thief");
  createLabel("Flag");
  createLabel("Flag");
  ++rows;

  // my statistics first
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank->getTeam() != ObserverTeam) {
    addStats((Player*)myTank);
  }

  // add statistics for each player
  for (int i = 0; i < curMaxPlayers; ++i) {
    if (remotePlayers[i] && (remotePlayers[i]->getTeam() != ObserverTeam)) {
      addStats((Player*)remotePlayers[i]);
    }
  }

  resize(HUDDialog::getWidth(), HUDDialog::getHeight());
  initNavigation();
}

ShotStats::~ShotStats()
{
  LocalFontFace::release(fontFace);
}

void ShotStats::createLabel(const std::string &str, bool navigable)
{
  HUDuiLabel* control = new HUDuiLabel;
  control->setFontFace(fontFace);
  control->setString(str);
  addControl(control, navigable);
}

void ShotStats::addStats(Player *_player)
{
  const ShotStatistics* stats = _player->getShotStatistics();
  createLabel(_player->getCallSign());

  createLabel(TextUtils::format("%2d%%", stats->getTotalPerc()));
  createLabel(TextUtils::format("%d/%d", stats->getTotalHit(),
				stats->getTotalFired()));
  createLabel(TextUtils::format("%d/%d", stats->getNormalHit(),
				stats->getNormalFired()));
  createLabel(TextUtils::format("%d/%d", stats->getGMHit(),
				stats->getGMFired()));
  createLabel(TextUtils::format("%d/%d", stats->getLHit(),
				stats->getLFired()));
  createLabel(TextUtils::format("%d/%d", stats->getSBHit(),
				stats->getSBFired()));
  createLabel(TextUtils::format("%d/%d", stats->getSWHit(),
				stats->getSWFired()));
  createLabel(TextUtils::format("%d/%d", stats->getTHHit(),
				stats->getTHFired()));

  std::string flagName = stats->getFavoriteFlag()->flagAbbv;
  if (flagName.empty())
    flagName = "None";
  createLabel(flagName);

  flagName = stats->getBestFlag()->flagAbbv;
  if (flagName.empty())
    flagName = "None";
  createLabel(flagName);

  ++rows;
}

HUDuiDefaultKey*	ShotStats::getDefaultKey()
{
  return ShotStatsDefaultKey::getInstance();
}

void			ShotStats::execute()
{
  HUDDialogStack::get()->pop();
}

void			ShotStats::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);
  FontSizer fs = FontSizer(_width, _height);

  // Reposition everything -- that's gonna be a challenge!

  FontManager &fm = FontManager::instance();

  // set up table
  // total width / (number of columns + 3 columns extra for player name + 2 columns margin)
  const float columnWidth = _width / (columns + 5.0f);
  const float fontSize = (float) columnWidth / 6;
  const float rowHeight = fm.getStringHeight(fontFace->getFMFace(), fontSize) * 1.2f;

  // center title
  fs.setMin(0, (int)(1.0 / BZDB.eval("headerFontSize") / 2.0));
  const float titleFontSize = fs.getFontSize(fontFace->getFMFace(), "headerFontSize");

  std::vector<HUDuiElement*>& listHUD = getElements();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);

  const float titleWidth = fm.getStringWidth(fontFace->getFMFace(), titleFontSize, title->getString().c_str());
  const float titleHeight = fm.getStringHeight(fontFace->getFMFace(), titleFontSize);
  const float titleY = (float)_height - titleHeight;
  float x = 0.5f * ((float)_width - titleWidth);
  float y = titleY;
  title->setPosition(x, y);

  // center key
  HUDuiLabel* key = (HUDuiLabel*)listHUD[1];
  key->setFontSize(fontSize);
  const float keyCenter = ((columns / 2) + 4) * columnWidth;
  const float keyWidth = fm.getStringWidth(fontFace->getFMFace(), fontSize, key->getString().c_str());
  const float keyY = titleY - 2 * fm.getStringHeight(fontFace->getFMFace(), fontSize);
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
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
