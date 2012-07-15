/* bzflag
 * Copyright (c) 1993-2012 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __SCORE_H__
#define __SCORE_H__

// bzflag global header
#include "global.h"

class Score {
 public:
  Score();
  int playerID;

  void  dump();
  /** Take into account the quality of player wins/(wins+loss)
      Try to penalize winning casuality
  */
  float ranking();
  bool  isTK() const;
  void  tK();
  void  killedBy();
  void  kill();
  void *pack(void *buf);
  bool  reached() const;
  int	getWins() const {return wins;}
  int	getLosses() const {return losses;}
  int	getTKs() const {return tks;}

  void	setWins(int v){wins = v;}
  void	setLosses(int v){losses = v;}
  void	setTKs(int v){tks = v;}

  static bool KeepPlayerScores;
  static bool KeepTeamScores;

  static void setTeamKillRatio(int _tkKickRatio);
  static void setWinLimit(int _score);
  static void setRandomRanking();

  int   getHandicap() const;
 private:
  // player's score
  int wins, losses, tks;
  // Tk index
  static float tkKickRatio;
  static int   score;
  static bool  randomRanking;
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
