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

#ifndef	BZF_CONTROL_PANEL_H
#define	BZF_CONTROL_PANEL_H

#if defined(_WIN32)
	#pragma warning(disable: 4786)
#endif

#include "MainWindow.h"
#include "global.h"
#include "OpenGLTexture.h"
#include "OpenGLTexFont.h"
#include "OpenGLGState.h"
#include <string>
#include <vector>

class RadarRenderer;
class SceneRenderer;

class ControlPanelMessage {
  public:
			ControlPanelMessage(const std::string&);
  public:
    std::string		string;
    int			rawLength;
};

class ControlPanel {
  public:
			ControlPanel(MainWindow&, SceneRenderer&);
			~ControlPanel();

    void		setControlColor(const GLfloat *color = NULL);
    void		render(SceneRenderer&);
    void		resize();

    void		setNumberOfFrameBuffers(int);

    void		addMessage(const std::string&);
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
    bool		resized;
    bool		blend;
    int			numBuffers;
    int			exposed;
    int			changedMessage;
    RadarRenderer*	radarRenderer;
    SceneRenderer*	renderer;

    OpenGLTexFont	messageFont;
    float		du, dv;
    int			radarAreaPixels[4];
    int			messageAreaPixels[4];
    std::vector<ControlPanelMessage>	messages;
    GLfloat		teamColor[3];
    static int		messagesOffset;
    static const int	maxScrollPages;
    int			maxLines;
};

#endif // BZF_CONTROL_PANEL_H
// ex: shiftwidth=2 tabstop=8
