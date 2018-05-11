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
#include "Score.h"
#include "bzfsAPI.h"
#include "WorldEventManager.h"
#include "GameKeeper.h"
#include "bzfs.h"

// bzflag library headers
#include "Pack.h"

// API headers for notification

float Score::tkKickRatio = 3.0;
int   Score::score       = 999;
bool  Score::randomRanking = false;

bool Score::KeepPlayerScores = true;
bool Score::KeepTeamScores = true;

Score::Score(): wins(0), losses(0), tks(0), handicap(0) {
  playerID = -1;
}

void Score::dump() {
  std::cout << wins << '-' << losses;
}

float Score::ranking() {
  if (randomRanking)
    return (float)bzfrand();

  // otherwise do score-based ranking
  int sum = wins + losses;
  if (sum == 0)
    return 0.5;
  float average = (float)wins/(float)sum;
  // IIRC that is how wide is the gaussian
  float penalty = (1.0f - 0.5f / sqrt((float)sum));
  return average * penalty;
}

bool Score::isTK() const {
  // arbitrary 3
  return (tks >= 3) && (tkKickRatio > 0)
    && ((wins == 0) || (tks * 100 / wins > tkKickRatio));
}

// Change a score element, then call an event
void Score::changeScoreElement(bz_eScoreElement element, int *toChange, int newValue) {
  int oldValue = *toChange;
  *toChange = newValue;
  bz_PlayerScoreChangeEventData_V1 eventData = bz_PlayerScoreChangeEventData_V1(playerID, element, oldValue, newValue);
  worldEventManager.callEvents(&eventData);
}


void Score::tK() {
  if (KeepPlayerScores) {
    changeScoreElement(bz_eTKs, &tks, tks + 1);
  }
}

void Score::killedBy() {
  if (KeepPlayerScores) {
    changeScoreElement(bz_eLosses, &losses, losses + 1);
  }
}

void Score::kill() {
  if (KeepPlayerScores) {
    changeScoreElement(bz_eWins, &wins, wins + 1);
  }
}


void Score::reset() {
  changeScoreElement(bz_eWins, &wins, 0);
  changeScoreElement(bz_eLosses, &losses, 0);
  changeScoreElement(bz_eTKs, &tks, 0);
}

void *Score::pack(void *buf) {
  buf = nboPackUShort(buf, wins);
  buf = nboPackUShort(buf, losses);
  buf = nboPackUShort(buf, tks);
  return buf;
}

bool Score::reached() const {
  return wins - losses >= score;
}

void Score::setTeamKillRatio(int _tkKickRatio) {
  tkKickRatio = (float)_tkKickRatio;
}

void Score::setWinLimit(int _score) {
  score = _score;
}

void Score::setRandomRanking() {
  randomRanking = true;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
