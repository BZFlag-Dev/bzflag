/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	__SCOREBOARDRENDERER_H__
#define	__SCOREBOARDRENDERER_H__

#include "common.h"

/* system interface headers */
#include <string>

/* common interface headers */
#include "bzfgl.h"

/* local interface headers */
#include "Player.h"


/**
 * ScoreboardRenderer:
 *	Encapsulates information about rendering the scoreboard display.
 */
class ScoreboardRenderer {
public:
  ScoreboardRenderer();
  ~ScoreboardRenderer();

  void		setDim(bool);
  void    setWindowSize (float x, float y, float width, float height);
  void		render();
  void		setHunting(bool _hunting);
  bool		getHunting() const;
  void		setHuntIndicator(bool _huntIndicator);
  void		setHuntPosition(int _huntPosition);
  int		  getHuntPosition() const;
  bool		getHuntSelection() const;
  void		setHuntSelection(bool _huntSelection);
  bool		getHuntIndicator() const;
  bool		getHunt() const;
  void		setHunt(bool _showHunt);
  
protected:
  void		hudColor3fv(const GLfloat*);
  void		renderTeamScores (float y, float x, float dy);
  void		renderScoreboard(void);
  void	  renderCtfFlags (void);
  void		drawPlayerScore(const Player*,
				  float x1, float x2, float x3, float xs, float y);

private:
  void		setMinorFontSize(float height);
  void		setLabelsFontSize(float height);
  static int	tankScoreCompare(const void* _a, const void* _b);
  static int	teamScoreCompare(const void* _a, const void* _b);

private:
  float winX;
  float winY;
  float winWidth;
  float winHeight;

  GLfloat		messageColor[3];
  int		  minorFontFace;
  float		minorFontSize;
  int		  labelsFontFace;
  float		labelsFontSize;

  bool		dim;
  float		scoreLabelWidth;
  float		killsLabelWidth;
  float		teamScoreLabelWidth;
  float		teamCountLabelWidth;
  float		huntArrowWidth;
  float		huntedArrowWidth;
  float		tkWarnRatio;

  static std::string	scoreSpacingLabel;
  static std::string	scoreLabel;
  static std::string	killSpacingLabel;
  static std::string	killLabel;
  static std::string	teamScoreSpacingLabel;
  static std::string	playerLabel;
  static std::string	teamCountSpacingLabel;
  bool		huntIndicator;
  bool		hunting;
  int			huntPosition;
  bool		huntSelection;
  bool		showHunt;
};


#endif /* __SCOREBOARDRENDERER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
