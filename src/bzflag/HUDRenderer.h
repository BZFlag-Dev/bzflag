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

#ifndef	__HUDRENDERER_H__
#define	__HUDRENDERER_H__

#include "common.h"

/* system interface headers */
#include <vector>
#include <string>

/* common interface headers */
#include "global.h"
#include "TimeKeeper.h"
#include "HUDuiTypeIn.h"
#include "Flag.h"

/* local interface headers */
#include "FlashClock.h"
#include "MainWindow.h"
#include "BzfDisplay.h"
#include "SceneRenderer.h"
#include "Player.h"


const int		MaxAlerts = 3;
const int		HUDNumCracks = 8;
const int		HUDCrackLevels = 4;


class HUDMarker {
public:
  float		heading;
  GLfloat		color[3];
};
typedef std::vector<HUDMarker> MarkerList;


/**
 * HUDRenderer:
 *	Encapsulates information about rendering the heads-up display.
 */
class HUDRenderer {
public:
  HUDRenderer(const BzfDisplay*,
	      const SceneRenderer&);
  ~HUDRenderer();

  int			getNoMotionSize() const;
  int			getMaxMotionSize() const;

  void		setColor(float r, float g, float b);
  void		setPlaying(bool playing);
  void		setRoaming(bool roaming);
  void		setPlayerHasHighScore(bool = true);
  void		setTeamHasHighScore(bool = true);
  void		setHeading(float angle);
  void		setAltitude(float altitude);
  void		setAltitudeTape(bool = true);
  void		setFPS(float fps);
  void		setDrawTime(float drawTimeInseconds);
  void		setAlert(int num, const char* string, float duration,
			 bool warning = false);
  void		setFlagHelp(FlagType* desc, float duration);
  void		initCracks();
  void		setCracks(bool showCracks);
  void		addMarker(float heading, const float *color);
  void		setRestartKeyLabel(const std::string&);
  void		setRoamingLabel(const std::string&);
  void		setTimeLeft(int timeLeftInSeconds);

  void		setDim(bool);

  bool		getComposing() const;
  std::string	getComposeString() const;
  void		setComposeString(const std::string &message) const;
  void		setComposeString(const std::string &message, bool _allowEdit) const;

  void		setComposing(const std::string &prompt);
  void		setComposing(const std::string &prompt, bool _allowEdit);

  void		render(SceneRenderer&);

  void		setHunting(bool _hunting);
  bool		getHunting() const;
  void		setHuntIndicator(bool _huntIndicator);
  void		setHuntPosition(int _huntPosition);
  int		getHuntPosition() const;
  bool		getHuntSelection() const;
  void		setHuntSelection(bool _huntSelection);
  bool		getHuntIndicator() const;
  bool		getHunt() const;
  void		setHunt(bool _showHunt);

protected:
  void		hudColor3f(GLfloat, GLfloat, GLfloat);
  void		hudColor4f(GLfloat, GLfloat, GLfloat, GLfloat);
  void		hudColor3fv(const GLfloat*);
  void		hudColor4fv(const GLfloat*);
  void		hudSColor3fv(const GLfloat*);
  void		renderAlerts(void);
  void		renderStatus(void);
  void		renderCracks();
  void		renderOptions(SceneRenderer&);
  void		renderCompose(SceneRenderer&);
  void		renderBox(SceneRenderer&);
  void		renderScoreboard(void);
  void		renderTankLabels(SceneRenderer&);
  void		renderPlaying(SceneRenderer&);
  void		renderNotPlaying(SceneRenderer&);
  void		renderRoaming(SceneRenderer&);
  void		renderTimes(void);
  void		drawPlayerScore(const Player*,
				float x1, float x2, float x3, float y);
  void		drawTeamScore(int team, float x, float y);

  void		makeCrack(float crackpattern[HUDNumCracks][(1 << HUDCrackLevels) + 1][2], int n, int l, float a);
  std::string	makeHelpString(const char* help) const;

private:
  void		setBigFontSize(int width, int height);
  void		setAlertFontSize(int width, int height);
  void		setMajorFontSize(int width, int height);
  void		setMinorFontSize(int width, int height);
  void		setHeadingFontSize(int width, int height);
  void		setComposeFontSize(int width, int height);
  void		setLabelsFontSize(int width, int height);

  void		resize(bool firstTime);
  static void	resizeCallback(void*);
  static int	tankScoreCompare(const void* _a, const void* _b);
  static int	teamScoreCompare(const void* _a, const void* _b);

private:
  const BzfDisplay*	display;
  MainWindow&		window;
  bool			firstRender;
  int			noMotionSize;
  int			maxMotionSize;
  float			headingOffset;
  GLfloat		hudColor[3];
  GLfloat		messageColor[3];
  GLfloat		warningColor[3];

  int		bigFontFace;
  float		bigFontSize;
  int		alertFontFace;
  float		alertFontSize;
  int		majorFontFace;
  float		majorFontSize;
  int		minorFontFace;
  float		minorFontSize;
  int		headingFontFace;
  float		headingFontSize;
  int		composeFontFace;
  float		composeFontSize;
  int		labelsFontFace;
  float		labelsFontSize;

  bool		playing;
  bool		roaming;
  bool		dim;
  int		numPlayers;
  int		timeLeft;
  TimeKeeper	timeSet;
  bool		playerHasHighScore;
  bool		teamHasHighScore;
  float		heading;
  float		altitude;
  bool		altitudeTape;
  float		fps;
  float		drawTime;
  int		headingMarkSpacing;
  float		headingLabelWidth[36];
  float		altitudeMarkSpacing;
  float		altitudeLabelMaxWidth;
  float		scoreLabelWidth;
  float		killsLabelWidth;
  float		teamScoreLabelWidth;
  float		restartLabelWidth;
  float		resumeLabelWidth;
  float		autoPilotWidth;
  float		cancelDestructLabelWidth;
  float		gameOverLabelWidth;
  float		huntArrowWidth;
  float		huntedArrowWidth;
  std::string	restartLabel;
  std::string	roamingLabel;

  FlashClock		globalClock;
  FlashClock		scoreClock;

  FlashClock		alertClock[MaxAlerts];
  std::string		alertLabel[MaxAlerts];
  float		alertLabelWidth[MaxAlerts];
  const GLfloat*	alertColor[MaxAlerts];

  float		flagHelpY;
  FlashClock		flagHelpClock;
  int			flagHelpLines;
  std::string		flagHelpText;

  bool		showOptions;
  bool		showCompose;

  GLfloat		cracks[HUDNumCracks][(1 << HUDCrackLevels) + 1][2];
  TimeKeeper		crackStartTime;
  bool		showCracks;

  MarkerList		markers;

  HUDuiTypeIn*	composeTypeIn;

  static const float	altitudeOffset;
  static const GLfloat black[3];
  static std::string	headingLabel[36];
  static std::string	scoreSpacingLabel;
  static std::string	scoreLabel;
  static std::string	killLabel;
  static std::string	teamScoreLabel;
  static std::string	teamScoreSpacingLabel;
  static std::string	playerLabel;
  static std::string	restartLabelFormat;
  static std::string	resumeLabel;
  static std::string	cancelDestructLabel;
  static std::string	gameOverLabel;
  static std::string	autoPilotLabel;
  bool		huntIndicator;
  bool		hunting;
  int			huntPosition;
  bool		huntSelection;
  bool		showHunt;
  bool    dater;
  unsigned int lastTimeChange;
};


#endif /* __HUDRENDERER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
