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

#ifndef	__HUDRENDERER_H__
#define	__HUDRENDERER_H__

#include "common.h"

/* system interface headers */
#include <vector>
#include <string>

/* common interface headers */
#include "TimeKeeper.h"
#include "HUDuiTypeIn.h"
#include "Flag.h"

/* local interface headers */
#include "FlashClock.h"
#include "MainWindow.h"
#include "BzfDisplay.h"
#include "SceneRenderer.h"
#include "Player.h"
#include "ScoreboardRenderer.h"


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
  HUDRenderer(const BzfDisplay*, const SceneRenderer&);
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
  void		setFrameTriangleCount(int tpf);
  void		setFrameRadarTriangleCount(int rtpf);
  void		setAlert(int num, const char* string, float duration,
			 bool warning = false);
  void		setFlagHelp(FlagType* desc, float duration);
  void		initCracks();
  void		setCracks(bool showCracks);
  void		addMarker(float heading, const float *color);
  void		setRestartKeyLabel(const std::string&);
  void		setTimeLeft(uint32_t timeLeftInSeconds);

  void		setDim(bool);

  bool		getComposing() const;
  std::string	getComposeString() const;
  void		setComposeString(const std::string &message) const;
  void		setComposeString(const std::string &message, bool _allowEdit) const;

  void		setComposing(const std::string &prompt);
  void		setComposing(const std::string &prompt, bool _allowEdit);

  void		render(SceneRenderer&);
  ScoreboardRenderer *getScoreboard();

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
  void		renderTankLabels(SceneRenderer&);
  void		renderPlaying(SceneRenderer&);
  void		renderNotPlaying(SceneRenderer&);
  void		renderRoaming(SceneRenderer&);
  void		renderTimes(void);
  void		renderShots(const Player*);

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
  ScoreboardRenderer* scoreboard;
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
  float   majorFontHeight;
  float   alertFontHeight;

  bool		playing;
  bool		roaming;
  bool		dim;
  int		numPlayers;
  uint32_t	timeLeft;
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
  float		restartLabelWidth;
  float		resumeLabelWidth;
  float		autoPilotWidth;
  float		cancelDestructLabelWidth;
  float		gameOverLabelWidth;
  float		huntArrowWidth;
  float		huntedArrowWidth;
  float		tkWarnRatio;
  std::string	restartLabel;

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
  static std::string	restartLabelFormat;
  static std::string	resumeLabel;
  static std::string	cancelDestructLabel;
  static std::string	gameOverLabel;
  static std::string	autoPilotLabel;
  bool			dater;
  unsigned int		lastTimeChange;
  int			triangleCount;
  int			radarTriangleCount;
};


#endif /* __HUDRENDERER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
