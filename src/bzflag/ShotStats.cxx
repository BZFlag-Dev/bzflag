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

/* interface headers */
#include "ShotStats.h"
#include "HUDDialog.h"

/* system implementation headers */
#include <string>
#include <vector>
#include <math.h>
#include <stdio.h>

/* common implementation headers */
#include "FontManager.h"
#include "StateDatabase.h"
#include "TextUtils.h"

/* local implementation headers */
#include "ShotStatsDefaultKey.h"
#include "HUDDialog.h"
#include "HUDDialogStack.h"
#include "HUDuiControl.h"
#include "HUDuiLabel.h"
#include "ShotStatistics.h"
#include "Player.h"
#include "RemotePlayer.h"
#include "LocalPlayer.h"

/* FIXME */
extern RemotePlayer** player;
extern int curMaxPlayers;

ShotStats::ShotStats() : HUDDialog()
{
  std::vector<HUDuiControl*>& list = getControls();

  // add title
  createLabel("Shot Statistics", list);

  // key
  createLabel("Shots Hit/Fired", list);

  columns = 11;
  rows = 0;

  // section headings (upper)
  createLabel("", list);
  createLabel("", list);
  createLabel("", list);
  createLabel("", list);
  createLabel("", list);
  createLabel("", list);
  createLabel("Super", list);
  createLabel("Shock", list);
  createLabel("", list);
  createLabel("Fave.", list);
  createLabel("Best", list);
  ++rows;

  // section headings (lower)
  createLabel("Player", list);
  createLabel("Hit%", list);
  createLabel("Total", list);
  createLabel("Norm", list);
  createLabel("GM", list);
  createLabel("Laser", list);
  createLabel("Bullet", list);
  createLabel("Wave", list);
  createLabel("Thief", list);
  createLabel("Flag", list);
  createLabel("Flag", list);
  ++rows;

  // my statistics first
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank->getTeam() != ObserverTeam) {
    addStats((Player*)myTank, list);
  }

  // add statistics for each player
  for (int i = 0; i < curMaxPlayers; ++i) {
    if (player[i] && (player[i]->getTeam() != ObserverTeam)) {
      addStats((Player*)player[i], list);
    }
  }

  resize(HUDDialog::getWidth(), HUDDialog::getHeight());
  initNavigation(list, 1, 1);
}

ShotStats::~ShotStats()
{
}

void ShotStats::createLabel(const std::string &str, std::vector<HUDuiControl*>& list)
{
  HUDuiLabel* control = new HUDuiLabel;
  control->setFontFace(getFontFace());
  control->setString(str);
  list.push_back(control);
}

void ShotStats::addStats(Player* player, std::vector<HUDuiControl*>& list)
{
  const ShotStatistics* stats = player->getShotStatistics();
  createLabel(player->getCallSign(), list);

  createLabel(TextUtils::format("%2d%%", stats->getTotalPerc()), list);
  createLabel(TextUtils::format("%d/%d", stats->getTotalHit(),  stats->getTotalFired()),  list);
  createLabel(TextUtils::format("%d/%d", stats->getNormalHit(), stats->getNormalFired()), list);
  createLabel(TextUtils::format("%d/%d", stats->getGMHit(),     stats->getGMFired()),     list);
  createLabel(TextUtils::format("%d/%d", stats->getLHit(),      stats->getLFired()),      list);
  createLabel(TextUtils::format("%d/%d", stats->getSBHit(),     stats->getSBFired()),     list);
  createLabel(TextUtils::format("%d/%d", stats->getSWHit(),     stats->getSWFired()),     list);
  createLabel(TextUtils::format("%d/%d", stats->getTHHit(),     stats->getTHFired()),     list);

  std::string flagName = stats->getFavoriteFlag()->flagAbbv;
  if (flagName.empty())
    flagName = "None";
  createLabel(flagName, list);

  flagName = stats->getBestFlag()->flagAbbv;
  if (flagName.empty())
    flagName = "None";
  createLabel(flagName, list);

  ++rows;
}

const int		ShotStats::getFontFace()
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

void			ShotStats::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  // Reposition everything -- that's gonna be a challenge!

  FontManager &fm = FontManager::instance();

  // set up table
  // total width / (number of columns + 3 columns extra for player name + 2 columns margin)
  const float columnWidth = width / (columns + 5.0f);
  const float fontSize = (float) columnWidth / 6;
  const float rowHeight = fm.getStrHeight(getFontFace(), fontSize, " ") * 1.2f;

  // center title
  const float titleFontSize = (float)height / 15.0f;
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStrLength(getFontFace(), titleFontSize, title->getString());
  const float titleHeight = fm.getStrHeight(getFontFace(), titleFontSize, " ");
  const float titleY = (float)height - titleHeight;
  float x = 0.5f * ((float)width - titleWidth);
  float y = titleY;
  title->setPosition(x, y);

  // center key
  HUDuiLabel* key = (HUDuiLabel*)list[1];
  key->setFontSize(fontSize);
  const float keyCenter = ((columns / 2) + 4) * columnWidth;
  const float keyWidth = fm.getStrLength(getFontFace(), fontSize, key->getString());
  const float keyY = titleY - 2 * fm.getStrHeight(getFontFace(), fontSize, " ");
  y = keyY;
  x = keyCenter - 0.5f * keyWidth;
  key->setPosition(x, y);

  for (int i = 2; i < (int)list.size(); ++i) {
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
    list[i]->setFontSize(fontSize);
    list[i]->setPosition(x, y);
  }

}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

