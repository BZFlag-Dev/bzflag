/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
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

static const char*	panelFile = "panel";

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
extern void		printMissingDataDirectoryError(const char*);

ControlPanel::ControlPanel(MainWindow& _mainWindow, SceneRenderer& renderer) :
				window(_mainWindow),
				resized(False),
				radarRenderer(NULL),
				panelZoomedImageSize(0),
				panelZoomedImage(NULL),
				origPanelZoomedImage(NULL)
{
  // make sure we're notified when MainWindow resizes or is exposed
  window.getWindow()->addResizeCallback(resizeCallback, this);
  window.getWindow()->addExposeCallback(exposeCallback, this);

  // load message font texture
  messageFont = TextureFont::getTextureFont(TextureFont::FixedBold, True);
  statusFont = messageFont;
  countFont = messageFont;

  // load background for control panel.  position and size of radar
  // and message areas encoded in the upper left corner of the image.
  int width, height, depth;
  panelImage = getTextureImage(panelFile, width, height, depth);
  if (!panelImage) {
    printMissingDataDirectoryError("Can't continue without control "
				"panel image.");
    exit(1);
  }
  background = OpenGLTexture(width, height, panelImage,
				OpenGLTexture::Linear, False);
  const int codePos = 4 * width * (height - 1);
  const float iWidth = (float)panelImage[codePos + 0] + 1.0f;
  const float iHeight = (float)panelImage[codePos + 4] + 1.0f;
  panelWidth = (int)iWidth;
  panelHeight = (int)iHeight;
  du = iWidth / (float)width;
  dv = iHeight / (float)height;
  const float dx = 1.0f / iWidth;
  const float dy = 1.0f / iHeight;
  radarAreaUV[0] = dx * (float)panelImage[codePos + 8];
  radarAreaUV[1] = dy * (float)panelImage[codePos + 12];
  radarAreaUV[2] = dx * (float)panelImage[codePos + 16];
  radarAreaUV[3] = dy * (float)panelImage[codePos + 20];
  messageAreaUV[0] = dx * (float)panelImage[codePos + 24];
  messageAreaUV[1] = dy * (float)panelImage[codePos + 28];
  messageAreaUV[2] = dx * (float)panelImage[codePos + 32];
  messageAreaUV[3] = dy * (float)panelImage[codePos + 36];
  statusAreaUV[0] = dx * (float)panelImage[codePos + 40];
  statusAreaUV[1] = dy * (float)panelImage[codePos + 44];
  statusAreaUV[2] = dx * (float)panelImage[codePos + 48];
  statusAreaUV[3] = dy * (float)panelImage[codePos + 52];
  teamCountAreaUV[0][0] = dx * (float)panelImage[codePos + 56];
  teamCountAreaUV[0][1] = dy * (float)panelImage[codePos + 60];
  teamCountAreaUV[1][0] = dx * (float)panelImage[codePos + 64];
  teamCountAreaUV[1][1] = dy * (float)panelImage[codePos + 68];
  teamCountAreaUV[2][0] = dx * (float)panelImage[codePos + 72];
  teamCountAreaUV[2][1] = dy * (float)panelImage[codePos + 76];
  teamCountAreaUV[3][0] = dx * (float)panelImage[codePos + 80];
  teamCountAreaUV[3][1] = dy * (float)panelImage[codePos + 84];
  teamCountAreaUV[4][0] = dx * (float)panelImage[codePos + 88];
  teamCountAreaUV[4][1] = dy * (float)panelImage[codePos + 92];
  teamCountSizeUV[0] = dx * (float)panelImage[codePos + 96];
  teamCountSizeUV[1] = dy * (float)panelImage[codePos + 100];

  // set panel ratio in main window.  ratio is panel width to height.
  // compute ratio based on the fact that the radar should be square.
  ratio = (iHeight / iWidth) *
		((float)panelImage[codePos + 16] /
		 (float)panelImage[codePos + 20]);
  window.setPanelRatio(ratio);

  // reverse channel order of control panel image if ABGR
  panelFormat = GL_RGBA;
  if (renderer.useABGR()) {
#ifdef GL_ABGR_EXT
    unsigned char* scan = panelImage;
    const int count = width * (height - 1);
    for (int i = 0; i < count; i++) {
      unsigned char t = scan[0];
      scan[0] = scan[3];
      scan[3] = t;
      t = scan[1];
      scan[1] = scan[2];
      scan[2] = t;
      scan += 4;
    }
    panelFormat = GL_ABGR_EXT;
#endif GL_ABGR_EXT
  }

  // gstate for background
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(background);
  gstate = builder.getState();

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

  // done with image
  delete[] panelImage;
  delete[] origPanelZoomedImage;
}

void			ControlPanel::render(int retouch)
{
  if (!resized) resize();
  if (retouch) {
    if (exposed) exposed += retouch;
    if (changedMessage) changedMessage += retouch;
    if (changedStatus) changedStatus += retouch;
    if (changedCounts) changedCounts += retouch;
  }
  if (!exposed && !changedMessage && !changedStatus && !changedCounts)
    return;

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

    // draw background
    const int w = width;
    if (OpenGLTexture::getFilter() == OpenGLTexture::Off) {
      glRasterPos2f(0.5f + blanking, 0.5f);
      glDrawPixels(w, h, (GLenum)panelFormat, GL_UNSIGNED_BYTE, panelZoomedImage);
    }
    else {
      glColor3f(1.0f, 1.0f, 1.0f);
      gstate.setState();
      glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2i(0 + blanking, 0);
	glTexCoord2f(du, 0.0f);
	glVertex2i(w + blanking, 0);
	glTexCoord2f(du, dv);
	glVertex2i(w + blanking, h);
	glTexCoord2f(0.0f, dv);
	glVertex2i(0 + blanking, h);
      glEnd();
    }
  }

  if (changedCounts) {
    changedCounts--;

    // erase player count areas
    // always erase because the colors could be different from the
    // ones in the panel image.
    /*if (doClear)*/ {
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
    glColor3f(1.0f, 1.0f, 1.0f);
    for (i = 0; i < NumTeams; i++) {
      if (teamCounts[i] != 0) {
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
  }

  if (changedStatus) {
    changedStatus--;

    // erase status area
    if (doClear) {
      OpenGLGState::resetState();
      glColor3f(0.0f, 0.0f, 0.0f);
      glRecti(statusAreaPixels[0] + 1,
	      statusAreaPixels[1] + 1,
	      statusAreaPixels[0] + statusAreaPixels[2] - 1,
	      statusAreaPixels[1] + statusAreaPixels[3] - 1);
    }

    // draw status
    if (status.getLength() != 0) {
      glColor3f(1.0f, 1.0f, 1.0f);
      statusFont.draw(status,
		(float)statusAreaPixels[0], (float)statusAreaPixels[1]);
    }
  }

  if (changedMessage) {
    changedMessage--;

    const float lineHeight = messageFont.getSpacing();
    const float margin = lineHeight / 4.0f;
    float fx = messageAreaPixels[0] + margin;
    float fy = messageAreaPixels[1] + margin + messageFont.getDescent() + 1.0f;
    glScissor(x + messageAreaPixels[0] + 1,
	      y + messageAreaPixels[1] + 1,
	      messageAreaPixels[2] - (int)margin - 1,
	      messageAreaPixels[3] - (int)margin - 1);

    // erase message area
    if (doClear) {
      OpenGLGState::resetState();
      glColor3f(0.0f, 0.0f, 0.0f);
      glRecti(messageAreaPixels[0] + 1,
	      messageAreaPixels[1] + 1,
	      messageAreaPixels[0] + messageAreaPixels[2] - 1,
	      messageAreaPixels[1] + messageAreaPixels[3] - 1);
    }

    // draw messages
    for (i = messages.getLength() - 1, j = 0;
				i >= 0 && j < maxLines; i--, j++) {
      glColor3fv(messages[i].color);
      messageFont.draw(messages[i].string, fx, fy);
      fy += lineHeight;
    }
  }

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

  // zoom control panel
  zoomPanel((int)w, (int)h);

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

void			ControlPanel::expose()
{
  exposed = 2;
  changedMessage = 2;
  changedStatus = 2;
  changedCounts = 2;
}

void			ControlPanel::exposeCallback(void* self)
{
  ((ControlPanel*)self)->expose();
}

void			ControlPanel::zoomPanel(int width, int height)
{
  // allocate more memory if there isn't enough already
  const int size = 4 * width * height;
  if (panelZoomedImageSize < size) {
    delete[] origPanelZoomedImage;
    panelZoomedImageSize = size;
    origPanelZoomedImage = new unsigned char[panelZoomedImageSize + 4];
    panelZoomedImage = (unsigned char*)(((unsigned long)
					origPanelZoomedImage & ~3) + 4);
  }

  // zoom pixels.  scan over destination and find pixel in source.
  unsigned char* dst = panelZoomedImage;
  const float dy = (float)panelHeight / (float)height;
  float y = 0.5f * dy;
  for (int j = 0; j < height; y += dy, j++) {
    const unsigned char* row = panelImage + 4 * (int)(y+0.25f) * panelWidth;
    const float dx = (float)panelWidth / (float)width;
    float x = 0.5f * dx;
    for (int i = 0; i < width; x += dx, dst += 4, i++) {
      const unsigned char* src = row + 4 * (int)(x+0.25f);
      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];
      dst[3] = src[3];
    }
  }
}

void			ControlPanel::addMessage(const BzfString& line,
						const GLfloat* color)
{
  ControlPanelMessage item(line, color);
  if (messages.getLength() < maxLines) {
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

  changedMessage = 2;
}

void			ControlPanel::setStatus(const char* _status)
{
  status = _status;
  changedStatus = 2;
}

void			ControlPanel::resetTeamCounts()
{
  for (int i = 0; i < NumTeams; i++)
    teamCounts[i] = 0;
  changedCounts = 2;
}

void			ControlPanel::setTeamCounts(const int* counts)
{
  for (int i = 0; i < NumTeams; i++)
    teamCounts[i] = counts[i];
  changedCounts = 2;
}

void			ControlPanel::setRadarRenderer(RadarRenderer* rr)
{
  radarRenderer = rr;
}
