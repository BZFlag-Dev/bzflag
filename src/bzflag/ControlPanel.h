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

#ifndef	BZF_CONTROL_PANEL_H
#define	BZF_CONTROL_PANEL_H

#include "MainWindow.h"
#include "global.h"
#include "OpenGLTexture.h"
#include "OpenGLTexFont.h"
#include "OpenGLGState.h"
#include "BzfString.h"
#include "AList.h"
#include "resources.h"

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
			ControlPanel(MainWindow&, SceneRenderer&, ResourceDatabase *_resources);
			~ControlPanel();

	void		setControlColor(const GLfloat *color = NULL);
    void		render(SceneRenderer&);
    void		resize();

    void		setNumberOfFrameBuffers(int);

    void		addMessage(const BzfString&, const GLfloat* = NULL);
	void		setMessagesOffset(int offset, int whence);
    void		setStatus(const char*);
    void		setRadarRenderer(RadarRenderer*);

  private:
    // no copying!
			ControlPanel(const ControlPanel&);
    ControlPanel&	operator=(const ControlPanel&);

    void		expose();
    void		change();

    static void		resizeCallback(void*);
    static void		exposeCallback(void*);

  private:
    MainWindow&		window;
    boolean		resized;
    boolean		blend;
    int			numBuffers;
    int			exposed;
    int			changedMessage;
    RadarRenderer*	radarRenderer;

    int			panelWidth;
    int			panelHeight;

    OpenGLTexFont	messageFont;
    float		du, dv;
    int			radarAreaPixels[4];
    int			messageAreaPixels[4];
    ControlPanelMessageList	messages;
    GLfloat		teamColor[3];
    static int		messagesOffset;
    static const int	maxScrollPages;
    static const int	maxLines;
    GLfloat				background[4];
};

#endif // BZF_CONTROL_PANEL_H
// ex: shiftwidth=2 tabstop=8
