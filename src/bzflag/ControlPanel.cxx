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

ControlPanelMessage::ControlPanelMessage(const std::string& _string,
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
				resized(false),
				numBuffers(2),
				radarRenderer(NULL)
{
  setControlColor();
  // make sure we're notified when MainWindow resizes or is exposed
  window.getWindow()->addResizeCallback(resizeCallback, this);
  window.getWindow()->addExposeCallback(exposeCallback, this);

  blend = renderer.useBlending();

  // other initialization
  radarAreaPixels[0] = 0;
  radarAreaPixels[1] = 0;
  radarAreaPixels[2] = 0;
  radarAreaPixels[3] = 0;
  messageAreaPixels[0] = 0;
  messageAreaPixels[1] = 0;
  messageAreaPixels[2] = 0;
  messageAreaPixels[3] = 0;
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
  if (!changedMessage && renderer.getPanelOpacity() == 1.0f)
    return;

  int i, j;
  const int x = window.getOriginX();
  const int y = window.getOriginY();
  const int w = window.getWidth();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, (double)w, 0.0, window.getHeight(), -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  OpenGLGState::resetState();

  const float lineHeight = messageFont.getSpacing();
  const float margin = lineHeight / 4.0f;

  if (changedMessage > 0) {
    changedMessage--;
  }
  float fx = messageAreaPixels[0] + margin;
  float fy = messageAreaPixels[1] + margin + messageFont.getDescent() + 1.0f;
  glScissor(x + messageAreaPixels[0],
      y + messageAreaPixels[1],
      messageAreaPixels[2],
      messageAreaPixels[3]);

  OpenGLGState::resetState();
  if (renderer.getPanelOpacity() > 0.0f) {
  // nice blended messages background
    if (renderer.useBlending() && renderer.getPanelOpacity() < 1.0f)
	glEnable(GL_BLEND);
    glColor4f(0.0f, 0.0f, 0.0f, renderer.getPanelOpacity());
    glRecti(messageAreaPixels[0],
    messageAreaPixels[1],
    messageAreaPixels[0] + messageAreaPixels[2],
    messageAreaPixels[1] + messageAreaPixels[3]);
    if (renderer.useBlending() && renderer.getPanelOpacity() < 1.0f)
	glDisable(GL_BLEND);
  }

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
  const int lineCharWidth = (int)(messageAreaPixels[2] / (messageFont.getWidth("-")));

  i = messages.size() - 1;
  if (messagesOffset>0) {
    if (i-messagesOffset > 0)
      i-=messagesOffset;
    else
      i=0;
  }
  for (j = 0; i>=0 && j<maxLines; i--) {
    glColor3fv(messages[i].color);

    // get message and its length
    const char* msg = messages[i].string.c_str();
    int lineLen = messages[i].string.length();

    // compute lines in message
    const int numLines = (lineLen + lineCharWidth - 1) / lineCharWidth;

    // draw each line
    int msgy = numLines - 1;
    while (lineLen > 0) {
      assert(msgy >= 0);

      // how many characters will fit?
      int n = lineLen;
      if (n > lineCharWidth)
        n = lineCharWidth;

      // only draw message if inside message area
      if (j + msgy < maxLines)
        messageFont.draw(msg, n, fx, fy + msgy * lineHeight);

      // account for portion drawn (or skipped)
      msg += n;
      lineLen -= n;

      // next line
      msgy--;
    }
    j += numLines;
    fy += (lineHeight * numLines);
  }
  glScissor(x + messageAreaPixels[0] - 1,
      y + messageAreaPixels[1] - 1,
      messageAreaPixels[2] + 2,
      messageAreaPixels[3] + 2);
  OpenGLGState::resetState();

  //  nice border
  glColor3f(teamColor[0], teamColor[1], teamColor[2] );
  glBegin(GL_LINE_LOOP); {
    glVertex2f((float) (x + messageAreaPixels[0] - 0.9f),
	(float) (y + messageAreaPixels[1] - 1));
    glVertex2f((float) (x + messageAreaPixels[0] - 1 + messageAreaPixels[2] + 1.1f),
	(float) (y + messageAreaPixels[1] - 1));
    glVertex2f((float) (x + messageAreaPixels[0] - 1 + messageAreaPixels[2] + 1.1f),
	(float) (y + messageAreaPixels[1] - 1 + messageAreaPixels[3] + 1.1f));
    glVertex2f((float) (x + messageAreaPixels[0] - 0.9f),
	(float) (y + messageAreaPixels[1] - 1 + messageAreaPixels[3] + 1.1f));
  } glEnd();
  // some engines miss the corners
  glBegin(GL_POINTS); {
    glVertex2f((float) (x + messageAreaPixels[0] - 0.9f),
	(float) (y + messageAreaPixels[1] - 1));
    glVertex2f((float) (x + messageAreaPixels[0] - 1 + messageAreaPixels[2] + 1.1f),
	(float) (y + messageAreaPixels[1] - 1));
    glVertex2f((float) (x + messageAreaPixels[0] - 1 + messageAreaPixels[2] + 1.1f),
	(float) (y + messageAreaPixels[1] - 1 + messageAreaPixels[3] + 1.1f));
    glVertex2f((float) (x + messageAreaPixels[0] - 0.9f),
       	(float) (y + messageAreaPixels[1] - 1 + messageAreaPixels[3] + 1.1f));
  } glEnd();

  // border for radar
  glScissor(x + radarAreaPixels[0] - 1,
      y + radarAreaPixels[1] - 1,
      radarAreaPixels[2] + 2,
      radarAreaPixels[3] + 2);
  OpenGLGState::resetState();

  // nice border
  glColor3f(teamColor[0], teamColor[1], teamColor[2] );
  glBegin(GL_LINE_LOOP); {
    glVertex2f((float) (x + radarAreaPixels[0] - 0.9f),
	(float) (y + radarAreaPixels[1] - 1));
    glVertex2f((float) (x + radarAreaPixels[0] - 1 + radarAreaPixels[2] + 1.1f),
	(float) (y + radarAreaPixels[1] - 1));
    glVertex2f((float) (x + radarAreaPixels[0] - 1 + radarAreaPixels[2] + 1.1f),
	(float) (y + radarAreaPixels[1] - 1 + radarAreaPixels[3] + 1.1f));
    glVertex2f((float) (x + radarAreaPixels[0] - 0.9f),
	(float) (y + radarAreaPixels[1] - 1 + radarAreaPixels[3] + 1.1f));
  } glEnd();
  glBegin(GL_POINTS); {
    glVertex2f((float) (x + radarAreaPixels[0] - 0.9f),
	(float) (y + radarAreaPixels[1] - 1));
    glVertex2f((float) (x + radarAreaPixels[0] - 1 + radarAreaPixels[2] + 1.1f),
	(float) (y + radarAreaPixels[1] - 1));
    glVertex2f((float) (x + radarAreaPixels[0] - 1 + radarAreaPixels[2] + 1.1f),
	(float) (y + radarAreaPixels[1] - 1 + radarAreaPixels[3] + 1.1f));
    glVertex2f((float) (x + radarAreaPixels[0] - 0.9f),
       	(float) (y + radarAreaPixels[1] - 1 + radarAreaPixels[3] + 1.1f));
  } glEnd();
  glPopMatrix();
}

void			ControlPanel::resize()
{
  float radarSpace, radarSize;
  // get important metrics
  const float w = (float)window.getWidth();
  const float h = (float)window.getHeight();
  const float opacity = SceneRenderer::getInstance()->getPanelOpacity();
  radarSize = float(window.getHeight() - window.getViewHeight());
  if (opacity == 1.0f) {
    radarSize = float(window.getHeight() - window.getViewHeight());
    radarSpace = 0.0f;
  }
  else {
    radarSize = h / 4.0f;
    radarSpace = 3.0f * w / MinY;
  }

  // compute areas in pixels x,y,w,h
  // leave off 1 pixel for the border
  radarAreaPixels[0] = radarAreaPixels[1] = (int)radarSpace + 1;
  radarAreaPixels[2] = radarAreaPixels[3] = (int)(radarSize - (radarSpace * 2.0f)) - 2;
  messageAreaPixels[0] = (int)radarSize + 1;
  messageAreaPixels[1] = radarAreaPixels[1];
  messageAreaPixels[2] = (int)(w - radarSize - radarSpace) - 2;
  messageAreaPixels[3] = radarAreaPixels[3];

  // if radar connected then resize it
  if (radarRenderer)
    radarRenderer->setShape(radarAreaPixels[0], radarAreaPixels[1],
				radarAreaPixels[2], radarAreaPixels[3]);

  const float addSize = SceneRenderer::getInstance()->useBigFont() ? 5.0f : 0.0f;
  const float fontSize = addSize + (float)messageAreaPixels[3] / 12.4444f;
  if (fontSize > 10.0f)
    messageFont = TextureFont::getTextureFont(TextureFont::FixedBold, true);
  else
    messageFont = TextureFont::getTextureFont(TextureFont::Fixed, true);
  // pick font size, room for 20 lines plus 4/9 line margin
  //const float fontSize = (float)messageAreaPixels[3] / 9.4444f;
  messageFont.setSize(fontSize, fontSize);

  // note that we've been resized at least once
  resized = true;
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
}

void			ControlPanel::exposeCallback(void* self)
{
  ((ControlPanel*)self)->expose();
}

void			ControlPanel::setMessagesOffset(int offset, int whence)
{
  // offset = offset from whence (offset of 0 is the bottom/most recent)
  // whence = 0, 1, or 2 (akin to SEEK_SET, SEEK_CUR, SEEK_END)
  switch (whence) {
    case 0:
      if (offset < messages.size())
	messagesOffset=offset;
      else
	messagesOffset=messages.size()-1;
      break;
    case 1:
      if (offset>0) {
	if (messagesOffset+offset < messages.size())
	  messagesOffset+=offset;
	else
	  messagesOffset=messages.size()-1;
      }
      else if (offset<0) {
	if (messagesOffset+offset >= 0)
	  messagesOffset+=offset;
	else
	  messagesOffset=0;
      }
      break;
    case 2:
      if (offset<0) {
	if (messages.size()-offset >= 0)
	  messagesOffset+=offset;
	else
	  messagesOffset=0;
      }
      break;
  }
  changedMessage = numBuffers;
}

void			ControlPanel::addMessage(const std::string& line,
						const GLfloat* color)
{
  ControlPanelMessage item(line, color);

  if (messages.size() < maxLines * maxScrollPages) {
    // not full yet so just append it
    messages.push_back(item);
  }
  else {
    // rotate list and replace oldest (in newest position after rotate)
    messages.erase(messages.begin());
    messages.push_back(item);
  }

  // this stuff has no effect on win32 (there's no console)
  extern bool echoToConsole;
  if (echoToConsole)
    fprintf(stdout, "%s\n", line.c_str());
    fflush(stdout);

  changedMessage = numBuffers;
}

void			ControlPanel::setRadarRenderer(RadarRenderer* rr)
{
  radarRenderer = rr;
}
// ex: shiftwidth=2 tabstop=8
