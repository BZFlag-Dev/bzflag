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

#ifndef	BZF_CONTROL_PANEL_H
#define	BZF_CONTROL_PANEL_H

#if defined(_MSC_VER)
  #pragma warning(disable: 4786)
#endif

#include "common.h"
#include "MainWindow.h"
#include "global.h"
#include "OpenGLGState.h"
#include <string>
#include <vector>
#include <deque>

class RadarRenderer;
class SceneRenderer;

struct ControlPanelMessage {
			ControlPanelMessage(const std::string&);
    void		breakLines(float maxLength, int fontFace, float fontSize);

    std::string		string;
    std::vector<std::string>	lines;
    int numlines;
};

class ControlPanel {
  public:
			ControlPanel(MainWindow&, SceneRenderer&);
			~ControlPanel();

    void		setControlColor(const GLfloat *color = NULL);
    void		render(SceneRenderer&);
    void		resize();
    void		invalidate();

    void		setNumberOfFrameBuffers(int);

    void		addMessage(const std::string&, const int mode = 3);
    void		setMessagesOffset(int offset, int whence);
    void		setMessagesMode(int _messageMode);
    int		getMessagesMode() {return messageMode;};
    void		setStatus(const char*);
    void		setRadarRenderer(RadarRenderer*);

  private:
    // no copying!
			ControlPanel(const ControlPanel&);
    ControlPanel&	operator=(const ControlPanel&);

    static void		resizeCallback(void*);
    static void		exposeCallback(void*);
    static void		bzdbCallback(const std::string& name, void* data);

    enum MessageModes {
      MessageAll = 0,
      MessageChat = 1,
      MessageServer = 2,
      MessageMisc = 3,
      MessageModeCount
    };
    bool tabsOnRight;
    std::vector<const char *> *tabs;
    std::vector<float> tabTextWidth;
    long totalTabWidth;


    MainWindow&		window;
    bool		resized;
    int			numBuffers;
    int			exposed;
    int			changedMessage;
    RadarRenderer*	radarRenderer;
    SceneRenderer*	renderer;

    int			fontFace;
    float		fontSize;
    float		du, dv;
    int			radarAreaPixels[4];
    int			messageAreaPixels[4];
    std::deque<ControlPanelMessage>	messages[MessageModeCount];
    int messageMode;
    GLfloat		teamColor[3];
    static int		messagesOffset;
    static const int	maxScrollPages;
    int			maxLines;
    float		margin;
    float		lineHeight;
    bool		unRead[MessageModeCount];
};

#endif // BZF_CONTROL_PANEL_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
