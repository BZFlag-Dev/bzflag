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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bzfgl.h"
#include "global.h"
#include "ControlPanel.h"
#include "SceneRenderer.h"
#include "MainWindow.h"
#include "BzfWindow.h"
#include "RadarRenderer.h"
#include "texture.h"
#include "ErrorHandler.h"
#include "Team.h"
#include "OpenGLGState.h"

void			printFatalError(const char* fmt, ...);

//
// ControlPanelMessage
//

ControlPanelMessage::ControlPanelMessage(const BzfString& _string,
				const GLfloat* _color) :
				string(_string)
{
  static const GLfloat defaultColor[3] = { 1.0f, 1.0f, 1.0f };
  if (!_color) _color = defaultColor;
  color[0] = _color[0];
  color[1] = _color[1];
  color[2] = _color[2];
}

//
// ControlPanel
//

const int		ControlPanel::maxLines = 30;
const int		ControlPanel::maxScrollPages = 4;
int				ControlPanel::messagesOffset = 0;
extern void		printMissingDataDirectoryError(const char*);

ControlPanel::ControlPanel(MainWindow& _mainWindow, SceneRenderer& renderer) :
				window(_mainWindow),
				resized(False),
				numBuffers(2),
				radarRenderer(NULL)
{
  setControlColor();
  // make sure we're notified when MainWindow resizes or is exposed
  window.getWindow()->addResizeCallback(resizeCallback, this);
  window.getWindow()->addExposeCallback(exposeCallback, this);

  // load message font texture
  messageFont = TextureFont::getTextureFont(TextureFont::FixedBold, True);
  statusFont = messageFont;
  countFont = messageFont;

  blend = renderer.useBlending();

  const float iWidth = 256.0f;
  const float iHeight = 62.0f;
  panelWidth = (int)iWidth;
  panelHeight = (int)iHeight;
  const float dx = 1.0f / iWidth;
  const float dy = 1.0f / iHeight;
  radarAreaUV[0] = dx * 5.0f;
  radarAreaUV[1] = dy * 5.0f;
  radarAreaUV[2] = dx * 42.0f;
  radarAreaUV[3] = dy * 42.0f;
  messageAreaUV[0] = dx * 57.0f;
  messageAreaUV[1] = dy * 5.0f;
  messageAreaUV[2] = dx * 180.0f;
  messageAreaUV[3] = dy * 42.0f;
  statusAreaUV[0] = dx * 3.0f;
  statusAreaUV[1] = dy * 54.0f;
  statusAreaUV[2] = dx * 250.0f;
  statusAreaUV[3] = dy * 5.0f;

  teamCountAreaUV[0][0] = dx * 242.0f;
  teamCountAreaUV[0][1] = dy * 6.0f;
  teamCountAreaUV[1][0] = dx * 242.0f;
  teamCountAreaUV[1][1] = dy * 42.0f;
  teamCountAreaUV[2][0] = dx * 242.0f;
  teamCountAreaUV[2][1] = dy * 33.0f;
  teamCountAreaUV[3][0] = dx * 242.0f;
  teamCountAreaUV[3][1] = dy * 24.0f;
  teamCountAreaUV[4][0] = dx * 242.0f;
  teamCountAreaUV[4][1] = dy * 15.0f;

  teamCountSizeUV[0] = dx * 10.0f;
  teamCountSizeUV[1] = dy * 5.0f;

  // set panel ratio in main window.  ratio is panel width to height.
  // compute ratio based on the fact that the radar should be square.
  ratio = (iHeight / iWidth) *
		1.0f /
		 1.0f;
  window.setPanelRatio(ratio);

  // other initialization
  width = 1;
  blanking = 0;
  radarAreaPixels[0] = 0;
  radarAreaPixels[1] = 0;
  radarAreaPixels[2] = 0;
  radarAreaPixels[3] = 0;
  messageAreaPixels[0] = 0;
  messageAreaPixels[1] = 0;
  messageAreaPixels[2] = 0;
  messageAreaPixels[3] = 0;
  statusAreaPixels[0] = 0;
  statusAreaPixels[1] = 0;
  statusAreaPixels[2] = 0;
  statusAreaPixels[3] = 0;
  teamCountAreaPixels[0][0] = 0;
  teamCountAreaPixels[0][1] = 0;
  teamCountAreaPixels[1][0] = 0;
  teamCountAreaPixels[1][1] = 0;
  teamCountAreaPixels[2][0] = 0;
  teamCountAreaPixels[2][1] = 0;
  teamCountAreaPixels[3][0] = 0;
  teamCountAreaPixels[3][1] = 0;
  teamCountAreaPixels[4][0] = 0;
  teamCountAreaPixels[4][1] = 0;
  teamCountSizePixels[0] = 0;
  teamCountSizePixels[1] = 0;
  resetTeamCounts();
  expose();
}

ControlPanel::~ControlPanel()
{
  // don't notify me anymore (cos you can't wake the dead!)
  window.getWindow()->removeResizeCallback(resizeCallback, this);
  window.getWindow()->removeExposeCallback(exposeCallback, this);
}

void			ControlPanel::setControlColor(const GLfloat *color)
{
	if (color)
		memcpy(teamColor, color, 3 * sizeof(float));
	else {
		memset(teamColor, 0, 3 * sizeof(float));
	}
}

void			ControlPanel::render(SceneRenderer& renderer)
{
  if (!resized) resize();

  int i, j;
  const int x = window.getOriginX();
  const int y = window.getOriginY();
  const int w = window.getWidth();
  const int h = window.getPanelHeight();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, (double)w, 0.0, window.getHeight(), -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  OpenGLGState::resetState();
  glScissor(x, y, w, h);

  boolean doClear = True;
  if (exposed > 0) {
    exposed--;
    doClear = False;

    if (w != width) {
	glColor3f(0.666667f, 0.666667f, 0.666667f);
    glBegin(GL_QUADS);
	glVertex2i(0, 0);
	glVertex2i(w, 0);
	glVertex2i(w, h);
	glVertex2i(0, h);
      glEnd();
    }
  }

  if (changedCounts) {
    changedCounts--;
  }

    // erase player count areas
    // always erase because the colors could be different from the
    // ones in the panel image.
    if (doClear) {
      static const GLfloat colors[][3] = {
				{ 0.0f, 0.0f, 0.0f },
				{ 0.7f, 0.0f, 0.0f },
				{ 0.0f, 0.7f, 0.0f },
				{ 0.0f, 0.0f, 0.7f },
				{ 0.7f, 0.0f, 0.7f },
				};
      OpenGLGState::resetState();
      for (i = 0; i < NumTeams; i++) {
	glColor3fv(colors[i]);
	glRecti(teamCountAreaPixels[i][0] + 1,
		teamCountAreaPixels[i][1] + 1,
		teamCountAreaPixels[i][0] + teamCountSizePixels[0] - 1,
		teamCountAreaPixels[i][1] + teamCountSizePixels[1] - 1);
      }
    }

    // draw player counts
    for (i = 0; i < NumTeams; i++) {
      if (teamCounts[i] != 0) {
		  if (i == 0)
			glColor3f(1.0f, 1.0f, 1.0f);
		  else
			glColor3f(0.0f, 0.0f, 0.0f);

		char buf[10];
		sprintf(buf, "%d", teamCounts[i]);
		const float w = 0.5f * countFont.getWidth(buf);
		countFont.draw(buf,
			(float)teamCountAreaPixels[i][0] +
				0.5f * (float)teamCountSizePixels[0] - w,
			(float)teamCountAreaPixels[i][1] +
				0.5f * (float)teamCountSizePixels[1] +
				countFont.getBaselineFromCenter());
      }
  }

  // removed status completely

  if (changedMessage) {
    changedMessage--;
  }

    const float lineHeight = messageFont.getSpacing();
    const float margin = lineHeight / 4.0f;
    float fx = messageAreaPixels[0] + margin;
    float fy = messageAreaPixels[1] + margin + messageFont.getDescent() + 1.0f;
  glScissor(x + messageAreaPixels[0] + 0,
	    y + messageAreaPixels[1] + 0,
	    messageAreaPixels[2] - (int)margin + 2,
	    messageAreaPixels[3] - (int)margin + 2);

  OpenGLGState::resetState();
  // nice blended messages background
  if(renderer.useBlending())
    glEnable(GL_BLEND);
  glColor4f(0.0f, 0.0f, 0.0, 0.3f );
      glRecti(messageAreaPixels[0] + 1,
	      messageAreaPixels[1] + 1,
	      messageAreaPixels[0] + messageAreaPixels[2] - 1,
	      messageAreaPixels[1] + messageAreaPixels[3] - 1);
  if(renderer.useBlending())
    glDisable(GL_BLEND);

    // draw messages
    // Code added to allow line-wrap -- just the basics so please modify
    //  for +'s at the beginning of wrapped lines, etc.
    //
    // It works by calculating the chars in the current mode's message
    //  area, calculating the number of lines the message will use, then
    //  moving up the proper number of lines and displaying downward -- that
    //  is, it kinda backtracks for each line that will wrap.
    //
    //  messageAreaPixels[2] = Width of Message Window in Pixels
    //  maxLines             = Max messages lines that can be displayed
    //  maxScrollPages       = This number * maxLines is the total maximum
    //                         lines of messages (and scrollback)
    // The font is fixed, so getWidth() returns the same for any char.
    const int lineCharWidth=(int)(messageAreaPixels[2] / (messageFont.getWidth("-")));

    i=messages.getLength()-1;
    if (messagesOffset>0) {
	if (i-messagesOffset > 0) i-=messagesOffset;
	else i=0;
    }
    for (j = 0; i>=0 && j<maxLines; i--) {
      glColor3fv(messages[i].color);

      // get message and its length
      const char* msg = messages[i].string.getString();
      int lineLen = messages[i].string.getLength();

      // compute lines in message
      const int numLines = (lineLen + lineCharWidth - 1) / lineCharWidth;

      // draw each line
      int y = numLines - 1;
      while (lineLen > 0) {
	assert(y >= 0);

	// how many characters will fit?
	int n = lineLen;
	if (n > lineCharWidth)
	  n = lineCharWidth;

	// only draw message if inside message area
	if (j + y < maxLines)
	  messageFont.draw(msg, n, fx, fy + y * lineHeight);

	// account for portion drawn (or skipped)
	msg += n;
	lineLen -= n;

	// next line
	y--;
      }
      j += numLines;
      fy += (lineHeight * numLines);
    }
  glScissor(x + messageAreaPixels[0] + 0,
	    y + messageAreaPixels[1] + 0,
	    messageAreaPixels[2] - (int)margin + 2,
	    messageAreaPixels[3] - (int)margin + 2);
  OpenGLGState::resetState();
  //  nice border
  glColor3f(teamColor[0], teamColor[1], teamColor[2] );
  glBegin(GL_LINE_LOOP); {
    glVertex2f((float) (x + messageAreaPixels[0] + 1), (float) (y + messageAreaPixels[1] + 1));
    glVertex2f((float) (x + messageAreaPixels[0] + messageAreaPixels[2] - (int)margin), (float) (y + messageAreaPixels[1] + 1));
    glVertex2f((float) (x + messageAreaPixels[0] + messageAreaPixels[2] - (int)margin), (float) (y + messageAreaPixels[1] + messageAreaPixels[3] - (int)margin));
    glVertex2f((float) (x + messageAreaPixels[0] + 1), (float) (y + messageAreaPixels[1] + messageAreaPixels[3] - (int)margin));
  } glEnd();
  // some engines miss the corners
  glBegin(GL_POINTS); {
    glVertex2f((float) (x + messageAreaPixels[0] + 1), (float) (y + messageAreaPixels[1] + 1));
    glVertex2f((float) (x + messageAreaPixels[0] + messageAreaPixels[2] - (int)margin), (float) (y + messageAreaPixels[1] + 1));
    glVertex2f((float) (x + messageAreaPixels[0] + messageAreaPixels[2] - (int)margin), (float) (y + messageAreaPixels[1] + messageAreaPixels[3] - (int)margin));
    glVertex2f((float) (x + messageAreaPixels[0] + 1), (float) (y + messageAreaPixels[1] + messageAreaPixels[3] - (int)margin));
  } glEnd();

  glPopMatrix();
}

void			ControlPanel::resize()
{
  // get important metrics
  float w = (float)window.getWidth();
  const float h = (float)window.getPanelHeight();

  // if h/w ratio is significantly less than correct ratio then we'll
  // put blank areas to the left and right of the panel to keep its ratio.
  blanking = 0;
  if (h / w < 0.9f * ratio) {
    blanking = (int)(0.5f * (w - h / ratio) + 0.5f);
    w = h / ratio;
  }
  width = (int)w;

  // compute areas in pixels
  radarAreaPixels[0] = (int)(w * radarAreaUV[0]) + blanking;
  radarAreaPixels[1] = (int)(h * radarAreaUV[1]);
  radarAreaPixels[2] = (int)(w * radarAreaUV[2]);
  radarAreaPixels[3] = (int)(h * radarAreaUV[3]);
  messageAreaPixels[0] = (int)(w * messageAreaUV[0]) + blanking;
  messageAreaPixels[1] = (int)(h * messageAreaUV[1]);
  messageAreaPixels[2] = (int)(w * messageAreaUV[2]);
  messageAreaPixels[3] = (int)(h * messageAreaUV[3]);
  statusAreaPixels[0] = (int)(w * statusAreaUV[0]) + blanking;
  statusAreaPixels[1] = (int)(h * statusAreaUV[1]);
  statusAreaPixels[2] = (int)(w * statusAreaUV[2]);
  statusAreaPixels[3] = (int)(h * statusAreaUV[3]);
  teamCountAreaPixels[0][0] = (int)(w * teamCountAreaUV[0][0]) + blanking;
  teamCountAreaPixels[0][1] = (int)(h * teamCountAreaUV[0][1]);
  teamCountAreaPixels[1][0] = (int)(w * teamCountAreaUV[1][0]) + blanking;
  teamCountAreaPixels[1][1] = (int)(h * teamCountAreaUV[1][1]);
  teamCountAreaPixels[2][0] = (int)(w * teamCountAreaUV[2][0]) + blanking;
  teamCountAreaPixels[2][1] = (int)(h * teamCountAreaUV[2][1]);
  teamCountAreaPixels[3][0] = (int)(w * teamCountAreaUV[3][0]) + blanking;
  teamCountAreaPixels[3][1] = (int)(h * teamCountAreaUV[3][1]);
  teamCountAreaPixels[4][0] = (int)(w * teamCountAreaUV[4][0]) + blanking;
  teamCountAreaPixels[4][1] = (int)(h * teamCountAreaUV[4][1]);
  teamCountSizePixels[0] = (int)(w * teamCountSizeUV[0]);
  teamCountSizePixels[1] = (int)(h * teamCountSizeUV[1]);

  // if radar connected then resize it
  if (radarRenderer)
    radarRenderer->setShape(radarAreaPixels[0], radarAreaPixels[1],
				radarAreaPixels[2], radarAreaPixels[3]);

  // pick font size, room for 9 lines plus 4/9 line margin
  const float fontSize = (float)messageAreaPixels[3] / 9.4444f;
  messageFont.setSize(fontSize, fontSize);

  // set font sizes for status and player counts
  statusFont.setSize(0.9f * (float)statusAreaPixels[3],
		     0.9f * (float)statusAreaPixels[3]);
  countFont.setSize(0.9f * (float)statusAreaPixels[3],
		    0.9f * (float)statusAreaPixels[3]);

  if (statusAreaPixels[3] >= 4) {
    const int margin = (int)(0.25f * (float)statusAreaPixels[3] + 0.5f);
    statusAreaPixels[0] += margin;
    statusAreaPixels[1] += margin;
    statusAreaPixels[2] -= 2 * margin;
    statusAreaPixels[3] -= 2 * margin;
  }

  // note that we've been resized at least once
  resized = True;
  expose();
}

void			ControlPanel::resizeCallback(void* self)
{
  ((ControlPanel*)self)->resize();
}

void			ControlPanel::setNumberOfFrameBuffers(int n)
{
  numBuffers = n;
  expose();
}

void			ControlPanel::expose()
{
  exposed = numBuffers;
  changedMessage = numBuffers;
  changedStatus = numBuffers;
  changedCounts = numBuffers;
}

void			ControlPanel::exposeCallback(void* self)
{
  ((ControlPanel*)self)->expose();
}

void			ControlPanel::setMessagesOffset(int offset, int whence)
{	// offset = offset from whence (offset of 0 is the bottom/most recent)
	// whence = 0, 1, or 2 (akin to SEEK_SET, SEEK_CUR, SEEK_END)
  switch (whence) {
    case 0:
      if (offset < messages.getLength()) messagesOffset=offset;
	 else messagesOffset=messages.getLength()-1;
	 break;
    case 1:
      if (offset>0)
	   {
	if (messagesOffset+offset < messages.getLength())
	     messagesOffset+=offset;
	   else messagesOffset=messages.getLength()-1;
	   }
	 else if (offset<0)
	   {
	if (messagesOffset+offset >= 0)
	     messagesOffset+=offset;
	   else messagesOffset=0;
	   }
	 break;
    case 2:
      if (offset<0)
	   {
	if (messages.getLength()-offset >= 0) messagesOffset+=offset;
	   else messagesOffset=0;
	}
	 break;
  }
  changedMessage = 2;
}

void			ControlPanel::addMessage(const BzfString& line,
						const GLfloat* color)
{
  ControlPanelMessage item(line, color);

  if (messages.getLength() < maxLines * maxScrollPages) {
    // not full yet so just append it
    messages.append(item);
  }
  else {
    // rotate list and replace oldest (in newest position after rotate)
    messages.rotate(messages.getLength() - 1, 0);
    messages[messages.getLength() - 1] = item;
  }

  // this stuff has no effect on win32 (there's no console)
  extern boolean echoToConsole;
  if (echoToConsole)
    fprintf(stdout, "%s\n", (const char*)line);
    fflush(stdout);

  changedMessage = numBuffers;
}

void			ControlPanel::setStatus(const char* _status)
{
  status = _status;
  changedStatus = numBuffers;
}

void			ControlPanel::resetTeamCounts()
{
  for (int i = 0; i < NumTeams; i++)
    teamCounts[i] = 0;
  changedCounts = numBuffers;
}

void			ControlPanel::setTeamCounts(const int* counts)
{
  for (int i = 0; i < NumTeams; i++)
    teamCounts[i] = counts[i];
  changedCounts = numBuffers;
}

void			ControlPanel::setRadarRenderer(RadarRenderer* rr)
{
  radarRenderer = rr;
}
// ex: shiftwidth=2 tabstop=8
