/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * HUDRenderer:
 *	Encapsulates information about rendering the heads-up display.
 */

#ifndef	BZF_HUD_RENDERER_H
#define	BZF_HUD_RENDERER_H

#include "common.h"
#include "global.h"
#include "OpenGLTexFont.h"
#include "TimeKeeper.h"
#include "HUDui.h"
#include "Flag.h"
#include <string>

class BzfDisplay;
class SceneRenderer;
class MainWindow;
class Player;
enum FlagId;

const int		MaxAlerts = 3;
const int		MaxHUDMarkers = 3;
const int		HUDNumCracks = 8;
const int		HUDCrackLevels = 4;

class FlashClock {
  public:
			FlashClock();
			~FlashClock();

    void		setClock(float time);
    void		setClock(float time, float onTime, float offTime);

    bool		isOn();

  private:
    TimeKeeper		startTime;
    float		duration;
    float		onDuration;
    float		flashDuration;
};

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
    void		setFlagHelp(FlagId, float duration);
    void		initCracks();
    void		setCracks(bool showCracks);
    void		setMarker(int index, bool = true);
    void		setMarkerHeading(int index, float heading);
    void		setMarkerColor(int index, float r, float g, float b);
    void		setRestartKeyLabel(const std::string&);
    void		setRoamingLabel(const std::string&);
    void		setTimeLeft(int timeLeftInSeconds);

    void		setDim(bool);

    bool		getComposing() const;
    std::string		getComposeString() const;
    void		setComposeString(const std::string &message) const;
    void		setComposeString(const std::string &message, bool _allowEdit) const;

    void		setComposing(const std::string &prompt);
    void		setComposing(const std::string &prompt, bool _allowEdit);

    void		render(SceneRenderer&);

    void		setHunting(bool _hunting);
    bool		getHunting() const;
    void		setHuntIndicator(bool _huntIndicator);
    void		setHuntPosition(int _huntPosition);
    int			getHuntPosition() const;
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
    void		drawDeadPlayerScore(const Player*,
					float x1, float x2, float x3, float y);
    void		drawTeamScore(int team, float x, float y);

    void		makeCrack(float crackpattern[HUDNumCracks][(1 << HUDCrackLevels) + 1][2], int n, int l, float a);
    std::string		makeHelpString(const char* help) const;

  private:
    void		setBigFontSize(int width, int height);
    void		setAlertFontSize(int width, int height);
    void		setMajorFontSize(int width, int height);
    void		setMinorFontSize(int width, int height);
    void		setHeadingFontSize(int width, int height);
    void		setComposeFontSize(int width, int height);
    void		setLabelsFontSize(int width, int height);

    void		resize(bool firstTime);
    static void		resizeCallback(void*);
    static int		tankScoreCompare(const void* _a, const void* _b);
    static int		teamScoreCompare(const void* _a, const void* _b);

    class Marker {
      public:
	bool		on;
	float		heading;
	GLfloat		color[3];
    };

  private:
    const BzfDisplay*	display;
    MainWindow&		window;
    bool		firstRender;
    int			noMotionSize;
    int			maxMotionSize;
    float		headingOffset;
    GLfloat		hudColor[3];
    GLfloat		messageColor[3];
    GLfloat		warningColor[3];
    OpenGLTexFont	bigFont;
    OpenGLTexFont	alertFont;
    OpenGLTexFont	majorFont;
    OpenGLTexFont	minorFont;
    OpenGLTexFont	headingFont;
    OpenGLTexFont	composeFont;
    OpenGLTexFont	labelsFont;
    bool		playing;
    bool		roaming;
    bool		dim;
    bool		sDim;
    int			numPlayers;
    int			timeLeft;
    TimeKeeper		timeSet;
    bool		playerHasHighScore;
    bool		teamHasHighScore;
    float		heading;
    float		altitude;
    bool		altitudeTape;
    float		fps;
    float		drawTime;
    int			headingMarkSpacing;
    float		headingLabelWidth[36];
    float		altitudeMarkSpacing;
    float		altitudeLabelMaxWidth;
    float		scoreLabelWidth;
    float		killsLabelWidth;
    float		teamScoreLabelWidth;
    float		restartLabelWidth;
    float		resumeLabelWidth;
    float		cancelDestructLabelWidth;
    float		gameOverLabelWidth;
    std::string		restartLabel;
    std::string		roamingLabel;

    FlashClock		globalClock;
    FlashClock		scoreClock;

    FlashClock		alertClock[MaxAlerts];
    std::string		alertLabel[MaxAlerts];
    float		alertLabelWidth[MaxAlerts];
    const GLfloat*	alertColor[MaxAlerts];

    float		flagHelpY;
    FlashClock		flagHelpClock;
    std::string		flagHelp[int(LastFlag) - int(FirstFlag) + 1];
    int			flagHelpIndex;
    int			flagHelpLines;

    bool		showOptions;
    bool		showCompose;

    GLfloat		cracks[HUDNumCracks][(1 << HUDCrackLevels) + 1][2];
    TimeKeeper		crackStartTime;
    bool		showCracks;

    Marker		marker[MaxHUDMarkers];

    HUDuiTypeIn*	composeTypeIn;

    static const float	altitudeOffset;
    static const GLfloat black[3];
    static std::string	headingLabel[36];
    static std::string	altitudeLabel[20];
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
    static const char*	flagHelpString[int(LastFlag) - int(FirstFlag) + 1];
    bool		huntIndicator;
    bool		hunting;
    int			huntPosition;
    bool		huntSelection;
    bool		showHunt;
};

#endif // BZF_HUD_RENDERER_H
// ex: shiftwidth=2 tabstop=8
