/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
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

#include "global.h"
#include "common.h"
#include "BzfString.h"
#include "OpenGLTexFont.h"
#include "TimeKeeper.h"
#include "HUDui.h"

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

    boolean		isOn();

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
    void		setPlaying(boolean playing);
    void		setRoaming(boolean roaming);
    void		setPlayerHasHighScore(boolean = True);
    void		setTeamHasHighScore(boolean = True);
    void		setHeading(float angle);
    void		setAltitude(float altitude);
    void		setAltitudeTape(boolean = True);
    void		setFPS(float fps);
    void		setDrawTime(float drawTimeInseconds);
    void		setAlert(int num, const char* string, float duration,
						boolean warning = False);
    void		setFlagHelp(FlagId, float duration);
    void		setCracks(boolean showCracks);
    void		setMarker(int index, boolean = True);
    void		setMarkerHeading(int index, float heading);
    void		setMarkerColor(int index, float r, float g, float b);
    void		setRestartKeyLabel(const BzfString&);
    void		setRoamingLabel(const BzfString&);
    void		setTimeLeft(int timeLeftInSeconds);

    void		setDim(boolean);

    boolean		getComposing() const;
    BzfString		getComposeString() const;
    void		setComposeString(const BzfString &message) const;

    void		setComposing(const BzfString &prompt);

    void		render(SceneRenderer&);

  protected:
    void		hudColor3f(GLfloat, GLfloat, GLfloat);
    void		hudColor4f(GLfloat, GLfloat, GLfloat, GLfloat);
    void		hudColor3fv(const GLfloat*);
    void		hudColor4fv(const GLfloat*);
    void		hudSColor3fv(const GLfloat*);
    void		renderAlerts(SceneRenderer&);
    void		renderStatus(SceneRenderer&);
    void		renderCracks(SceneRenderer&);
    void		renderOptions(SceneRenderer&);
    void		renderCompose(SceneRenderer&);
    void		renderScoreboard(SceneRenderer&);
    void		renderPlaying(SceneRenderer&);
    void		renderNotPlaying(SceneRenderer&);
    void		renderRoaming(SceneRenderer&);
    void		renderTimes(SceneRenderer&);
    void		drawPlayerScore(const Player*,
					float x1, float x2, float x3, float y);
    void		drawDeadPlayerScore(const Player*,
					float x1, float x2, float x3, float y);
    void		drawTeamScore(int team, float x, float y);

    void		makeCrack(int n, int l, float a);
    BzfString		makeHelpString(const char* help) const;

  private:
    void		setBigFontSize(int width, int height);
    void		setAlertFontSize(int width, int height);
    void		setMajorFontSize(int width, int height);
    void		setMinorFontSize(int width, int height);
    void		setHeadingFontSize(int width, int height);
    void		setComposeFontSize(int width, int height);

    void		resize(boolean firstTime);
    static void		resizeCallback(void*);
    static int		tankScoreCompare(const void* _a, const void* _b);
    static int		teamScoreCompare(const void* _a, const void* _b);

    class Marker {
      public:
	boolean		on;
	float		heading;
	GLfloat		color[3];
    };

  private:
    const BzfDisplay*	display;
    MainWindow&		window;
    boolean		firstRender;
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
    boolean		playing;
    boolean		roaming;
    boolean		dim;
    boolean		sDim;
    int			numPlayers;
    int			timeLeft;
    TimeKeeper		timeSet;
    boolean		playerHasHighScore;
    boolean		teamHasHighScore;
    float		heading;
    float		altitude;
    boolean		altitudeTape;
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
    float		gameOverLabelWidth;
    BzfString		restartLabel;
    BzfString		roamingLabel;

    FlashClock		globalClock;
    FlashClock		scoreClock;

    FlashClock		alertClock[MaxAlerts];
    BzfString		alertLabel[MaxAlerts];
    float		alertLabelWidth[MaxAlerts];
    const GLfloat*	alertColor[MaxAlerts];

    float		flagHelpY;
    FlashClock		flagHelpClock;
    BzfString		flagHelp[int(LastFlag) - int(FirstFlag) + 1];
    int			flagHelpIndex;
    int			flagHelpLines;

    boolean		showOptions;
    boolean		showCompose;

    GLfloat		cracks[HUDNumCracks][(1 << HUDCrackLevels) + 1][2];
    boolean		showCracks;

    Marker		marker[MaxHUDMarkers];

    HUDuiTypeIn*	composeTypeIn;

    static const float	altitudeOffset;
    static const GLfloat black[3];
    static BzfString	headingLabel[36];
    static BzfString	altitudeLabel[20];
    static BzfString	scoreSpacingLabel;
    static BzfString	scoreLabel;
    static BzfString	killLabel;
    static BzfString	teamScoreLabel;
    static BzfString	teamScoreSpacingLabel;
    static BzfString	playerLabel;
    static BzfString	restartLabelFormat;
    static BzfString	resumeLabel;
    static BzfString	gameOverLabel;
    static const char*	flagHelpString[int(LastFlag) - int(FirstFlag) + 1];
};

#endif // BZF_HUD_RENDERER_H
// ex: shiftwidth=2 tabstop=8
