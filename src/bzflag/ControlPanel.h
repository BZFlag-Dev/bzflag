/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	BZF_CONTROL_PANEL_H
#define	BZF_CONTROL_PANEL_H

#include "MainWindow.h"
#include "global.h"
#include "OpenGLTexture.h"
#include "OpenGLTexFont.h"
#include "OpenGLGState.h"
#include "BzfString.h"
#include "AList.h"

class RadarRenderer;
class SceneRenderer;

class ControlPanelMessage {
  public:
			ControlPanelMessage(const BzfString&, const GLfloat*);
  public:
    BzfString		string;
    GLfloat		color[3];
};
BZF_DEFINE_ALIST(ControlPanelMessageList, ControlPanelMessage);

class ControlPanel {
  public:
			ControlPanel(MainWindow&, SceneRenderer&);
			~ControlPanel();

    void		render(int retouch = 0);
    void		resize();

    void		addMessage(const BzfString&, const GLfloat* = NULL);
    void		setStatus(const char*);
    void		resetTeamCounts();
    void		setTeamCounts(const int* counts);
    void		setRadarRenderer(RadarRenderer*);

  private:
    // no copying!
			ControlPanel(const ControlPanel&);
    ControlPanel&	operator=(const ControlPanel&);

    void		expose();
    void		change();

    void		zoomPanel(int width, int height);

    static void		resizeCallback(void*);
    static void		exposeCallback(void*);

  private:
    MainWindow&		window;
    boolean		resized;
    int			exposed;
    int			changedMessage;
    int			changedStatus;
    int			changedCounts;
    RadarRenderer*	radarRenderer;

    int			panelWidth;
    int			panelHeight;
    int			panelFormat;
    unsigned char*	panelImage;
    int			panelZoomedImageSize;
    unsigned char*	panelZoomedImage;
    unsigned char*	origPanelZoomedImage;

    OpenGLGState	gstate;
    OpenGLTexture	background;
    OpenGLTexFont	messageFont;
    OpenGLTexFont	statusFont;
    OpenGLTexFont	countFont;
    int			width, blanking;
    float		ratio;
    float		du, dv;
    float		radarAreaUV[4];
    float		messageAreaUV[4];
    float		statusAreaUV[4];
    float		teamCountAreaUV[NumTeams][2];
    float		teamCountSizeUV[2];
    int			radarAreaPixels[4];
    int			messageAreaPixels[4];
    int			statusAreaPixels[4];
    int			teamCountAreaPixels[NumTeams][2];
    int			teamCountSizePixels[2];
    BzfString		status;
    int			teamCounts[NumTeams];
    ControlPanelMessageList	messages;
    static const int	maxLines;
};

#endif // BZF_CONTROL_PANEL_H
