/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* BZFlag common header */
#include "common.h"

/* interface header */
#include "ControlPanel.h"

/* system headers */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* common implementation headers */
#include "bzfgl.h"
#include "global.h"
#include "BzfWindow.h"
#include "ErrorHandler.h"
#include "Team.h"
#include "OpenGLGState.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "AnsiCodes.h"
#include "FontManager.h"
#include "TextUtils.h"

/* local implementation headers */
#include "SceneRenderer.h"
#include "MainWindow.h"
#include "RadarRenderer.h"

void			printFatalError(const char* fmt, ...);

//
// ControlPanelMessage
//

ControlPanelMessage::ControlPanelMessage(const std::string& _string)
{
  this->string = _string;
}

void ControlPanelMessage::breakLines(float maxLength, int fontFace, float fontSize)
{
  FontManager &fm = FontManager::instance();

  // get message and its length
  const char* msg = string.c_str();
  int lineLen     = string.length();

  // if there are tabs in the message, find the last one
  int lastTab = string.find_last_of('\t', lineLen - 1);

  lines.clear();

  // in order for the new font engine to draw successive lines in the right
  // color, it needs to be fed the right ansi codes at the beginning of each
  // line.
  std::string cumulativeANSICodes = "";

  // break lines
  while (lineLen > 0) {
    int lastWhitespace = 0;
    int n;

    // how many characters will fit?
    // the unprinted ANSI codes don't count
    if ((fm.getStrLength(fontFace, fontSize, msg) <= maxLength) && (lastTab <= 0)) {
      n = lineLen;
    } else {
      n = 0;
      while ((n < lineLen) &&
	     (fm.getStrLength(fontFace, fontSize, std::string(msg).substr(0, n)) < maxLength)) {
	if (msg[n] == ESC_CHAR) {
	  // clear the cumulative codes when we hit a reset
	  // the reset itself will start the new cumulative string.
	  if ((strncmp(msg + n, ANSI_STR_RESET, strlen(ANSI_STR_RESET)) == 0)
	      || (strncmp(msg + n, ANSI_STR_RESET_FINAL, strlen(ANSI_STR_RESET_FINAL)) == 0))
	    cumulativeANSICodes = "";
	  // add this code to our cumulative string
	  cumulativeANSICodes += msg[n];
	  n++;
	  if ((n < lineLen) && (msg[n] == '[')) {
	    cumulativeANSICodes += msg[n];
	    n++;
	    while ((n < lineLen) && ((msg[n] == ';') ||
		  ((msg[n] >= '0') && (msg[n] <= '9')))) {
	      cumulativeANSICodes += msg[n];
	      n++;
	    }
	    // ditch the terminating character too
	    if (n < lineLen) {
	      cumulativeANSICodes += msg[n];
	      n++;
	    }
	  }
	} else {
	  n++;
	}
        if (TextUtils::isWhitespace(msg[n])) {
	  lastWhitespace = n;
	  // Tabs break out into their own message.  These get dealt with
	  // in ::render, which will increment x instead of y.
	  if (msg[n] == '\t')
	    break;
	}
      }
    }

    if (lastWhitespace > 0)
      n = lastWhitespace;

    // message
    lines.push_back(cumulativeANSICodes + std::string(msg).substr(0, n));

    // account for portion broken
    msg += n;
    lineLen -= n;
    lastTab -= n;

  }
}

//
// ControlPanel
//
int			ControlPanel::messagesOffset = 0;

ControlPanel::ControlPanel(MainWindow& _mainWindow, SceneRenderer& renderer) :
				tabsOnRight(true),
				tabs(NULL),
				totalTabWidth(0),
				window(_mainWindow),
				resized(false),
				numBuffers(2),
				exposed(0),
				changedMessage(0),
				radarRenderer(NULL),
				renderer(&renderer),
				fontFace(0),
				du(0),
				dv(0),
				messageMode(MessageAll)
{
  setControlColor();
  
  // make sure we're notified when MainWindow resizes or is exposed
  window.getWindow()->addResizeCallback(resizeCallback, this);
  window.getWindow()->addExposeCallback(exposeCallback, this);
  BZDB.addCallback(StateDatabase::BZDB_NORADAR, bzdbCallback, this);
  
  // other initialization
  radarAreaPixels[0] = 0;
  radarAreaPixels[1] = 0;
  radarAreaPixels[2] = 0;
  radarAreaPixels[3] = 0;
  messageAreaPixels[0] = 0;
  messageAreaPixels[1] = 0;
  messageAreaPixels[2] = 0;
  messageAreaPixels[3] = 0;
  for (int i = 0; i < MessageModeCount; i++) {
    messages[i].clear();
  }
  teamColor[0] = teamColor[1] = teamColor[2] = (GLfloat)0.0f;
  expose();

  maxLines = 30;

  /* use pointer initialization to perform precomputation and
   * prestorage. eventually should move this data into constructor.
   */
  if (tabs == NULL) {
    tabs = new std::vector<const char *>;
    tabs->push_back("All");
    tabs->push_back("Chat");
    tabs->push_back("Server");
    tabs->push_back("Misc");
  }

  resize(); // need resize to set up font and window dimensions
}

ControlPanel::~ControlPanel()
{
  // don't notify me anymore (cos you can't wake the dead!)
  window.getWindow()->removeResizeCallback(resizeCallback, this);
  window.getWindow()->removeExposeCallback(exposeCallback, this);
  BZDB.removeCallback(StateDatabase::BZDB_NORADAR, bzdbCallback, this);
  
  extern bool echoToConsole;
  extern bool echoAnsi;
  if (echoToConsole && echoAnsi) {
    std::cout << ColorStrings[FinalResetColor] << std::flush;
  }
  if (tabs != NULL) {
    tabs->clear();
    delete tabs;
    tabTextWidth.clear();
    totalTabWidth=0;
  }
}

void ControlPanel::bzdbCallback(const std::string& /*name*/, void* data)
{
  ((ControlPanel*)data)->resize();
  return;
}

void			ControlPanel::setControlColor(const GLfloat *color)
{
  if (color)
    memcpy(teamColor, color, 3 * sizeof(float));
  else
    memset(teamColor, 0, 3 * sizeof(float));
}

void			ControlPanel::render(SceneRenderer& renderer)
{
  if (!BZDB.isTrue("displayConsoleAndRadar")) {
    return;
  }

  if (!resized) resize();
  if (!changedMessage && renderer.getPanelOpacity() == 1.0f)
    return;

  int i, j;
  const int x = window.getOriginX();
  const int y = window.getOriginY();
  const int w = window.getWidth();
  const int tabStyle = (int)BZDB.eval("showtabs");
  const bool showTabs = (tabStyle > 0);
  tabsOnRight = (tabStyle == 2);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, (double)w, 0.0, window.getHeight(), -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  OpenGLGState::resetState();

  FontManager &fm = FontManager::instance();

  if (changedMessage > 0) {
    changedMessage--;
  }
  float fx = messageAreaPixels[0] + margin;
  float fy = messageAreaPixels[1] + margin + 1.0f;
  int   ay = (renderer.getPanelOpacity() == 1.0f || !showTabs) ? 0 : int(lineHeight + 4);
  glScissor(x + messageAreaPixels[0],
      y + messageAreaPixels[1],
      messageAreaPixels[2],
      messageAreaPixels[3] + ay);

  OpenGLGState::resetState();
  if (renderer.getPanelOpacity() > 0.0f) {
    // nice blended messages background
    if (BZDBCache::blend && renderer.getPanelOpacity() < 1.0f)
      glEnable(GL_BLEND);

    // clear the background
    glColor4f(0.0f, 0.0f, 0.0f, renderer.getPanelOpacity());
    glRecti(messageAreaPixels[0],
	    messageAreaPixels[1],
	    messageAreaPixels[0] + messageAreaPixels[2],
	    messageAreaPixels[1] + messageAreaPixels[3]);

    // display tabs for chat sections
    if (showTabs) {
      long int drawnTabWidth = 0;
      for (unsigned int tab = 0; tab < tabs->size(); tab++) {

	// current mode is given a dark background to match the control panel
	if (messageMode == MessageModes(tab)) {
	  glColor4f(0.0f, 0.0f, 0.0f, renderer.getPanelOpacity());
	} else {
	  glColor4f(0.10f, 0.10f, 0.10f, renderer.getPanelOpacity());
	}

	if (tabsOnRight) {
	  // draw the tabs on the right side
	  glRecti(messageAreaPixels[0] + messageAreaPixels[2] - totalTabWidth + drawnTabWidth,
		  messageAreaPixels[1] + messageAreaPixels[3] - int(lineHeight + 4) + ay,
		  messageAreaPixels[0] + messageAreaPixels[2] - totalTabWidth + drawnTabWidth + int(tabTextWidth[tab]), //+ drawnTabWidth + int(tabTextWidth[tab]),
		  messageAreaPixels[1] + messageAreaPixels[3] + ay);
	} else {
	  // draw the tabs on the left side
	  glRecti(messageAreaPixels[0] + drawnTabWidth,
		  messageAreaPixels[1] + messageAreaPixels[3] - int(lineHeight + 4) + ay,
		  messageAreaPixels[0] + drawnTabWidth + int(tabTextWidth[tab]),
		  messageAreaPixels[1] + messageAreaPixels[3] + ay);
	}
	drawnTabWidth += long(tabTextWidth[tab]);
      } // end iteration over tabs
    }
    if (BZDBCache::blend && renderer.getPanelOpacity() < 1.0f)
      glDisable(GL_BLEND);
  }

  // Draw tab labels
  if (showTabs) {
    long int drawnTabWidth = 0;
    for (unsigned int tab = 0; tab < tabs->size(); tab++) {

      // current mode is bright, others are not so bright
      if (messageMode == MessageModes(tab)) {
	glColor3f(1.0f, 1.0f, 1.0f);
      } else {
	glColor3f(0.5f, 0.5f, 0.5f);
      }

      if (tabsOnRight) {
	// draw the tabs on the right side
	fm.drawString(messageAreaPixels[0] + messageAreaPixels[2] - totalTabWidth + drawnTabWidth + floorf(fontSize * 2.0f),
		      messageAreaPixels[1] + messageAreaPixels[3] - floorf(lineHeight + 2.0f) + ay,
		      0.0f, fontFace, (float)fontSize, (*tabs)[tab]);
      } else {
	// draw the tabs on the left side
	fm.drawString(messageAreaPixels[0] + drawnTabWidth + floorf(fontSize * 2.0f),
		      messageAreaPixels[1] + messageAreaPixels[3] - floorf(lineHeight + 2.0f) + ay,
		      0.0f, fontFace, (float)fontSize, (*tabs)[tab]);
      }
      drawnTabWidth += long(tabTextWidth[tab]);
    }
  }

  // draw messages
  //
  // It works by first breaking the string into a vector of strings (done 
  //  elsewhere), each of which will fit the control panel, and tallying 
  //  the number of lines, then moving up the proper number of lines and
  //  displaying downward -- that is, it kinda backtracks for each line 
  //  that will wrap.
  //
  //  messageAreaPixels[2] = Width of Message Window in Pixels
  //  maxLines             = Max messages lines that can be displayed
  //  maxScrollPages       = This number * maxLines is the total maximum
  //                         lines of messages (and scrollback). It is
  //                         stored as a BZDB parameter.

  glScissor(x + messageAreaPixels[0],
	    y + messageAreaPixels[1],
	    messageAreaPixels[2],
	    messageAreaPixels[3] - (showTabs ? int(lineHeight + 4) : 0) + ay);

  i = messages[messageMode].size() - 1;
  if (messagesOffset > 0) {
    if (i - messagesOffset > 0)
      i -= messagesOffset;
    else
      i = 0;
  }
  for (j = 0; i >= 0 && j < maxLines; i--) {
    // draw each line of text
    int numLines = messages[messageMode][i].lines.size();
    int msgy = numLines - 1;
    int msgx = 0;

    // default to drawing text in white
    GLfloat whiteColor[3] = {1.0f, 1.0f, 1.0f};
    glColor3fv(whiteColor);

    bool isTab = false;

    for (int l = 0; l < numLines; l++)  {
      assert(msgy >= 0);

      std::string msg = messages[messageMode][i].lines[l];

      // Tab chars move horizontally instead of vertically
      // It doesn't matter where in the string the tab char is
      // Usually it will be like <ansi><ansi><ansi>\ttext
      // We use 1 tabstop spaced 1/3 of the way across the controlpanel
      isTab = (msg.find('\t', 0) != std::string::npos);
      if (isTab) {
	msgx += messageAreaPixels[2] / 3;
	msgy++;
      } else {
	msgx = 0;
      }

      // only draw message if inside message area
      if (j + msgy < maxLines)
	fm.drawString(fx + msgx, fy + msgy * lineHeight, 0, fontFace, fontSize, msg);

      // next line
      if (!isTab) {
	msgy--;
      }
    }
    j += numLines;
    fy += int(lineHeight * numLines);
  }
  glScissor(x + messageAreaPixels[0] - 1,
	    y + messageAreaPixels[1] - 1,
	    messageAreaPixels[2] + 2,
	    messageAreaPixels[3] + 32);
  OpenGLGState::resetState();

  // draw the lines around the console panel
  long xpos;
  long ypos;

  // nice border
  glColor3f(teamColor[0], teamColor[1], teamColor[2] );
  glBegin(GL_LINE_LOOP); {
    // bottom left
    xpos = x + messageAreaPixels[0] - 1;
    ypos = y + messageAreaPixels[1] - 1;
    glVertex2f((float) xpos, (float) ypos);

    // bottom right
    xpos += messageAreaPixels[2] + 1;
    glVertex2f((float) xpos, (float) ypos);

    // top right
    ypos += messageAreaPixels[3] + 1;
    glVertex2f((float) xpos, (float) ypos);

    // over to panel on left
    if (!tabsOnRight) {
      xpos = x + messageAreaPixels[0] + totalTabWidth + 1;
      glVertex2f((float) xpos, (float) ypos);
    }

    // across the top from right to left
    long int drawnTabWidth = 0;
    for (int tab = (int)tabs->size() - 1; tab >= 0; tab--) {

      if (messageMode == MessageModes(tab)) {
	ypos += ay;
	glVertex2f((float) xpos, (float) ypos);

	xpos -= long(tabTextWidth[tab]);
	glVertex2f((float) xpos, (float) ypos);

	ypos -= ay;
	glVertex2f((float) xpos, (float) ypos);
      } else {
	xpos -= long(tabTextWidth[tab]);
	glVertex2f((float) xpos, (float) ypos);
      }
      drawnTabWidth += long(tabTextWidth[tab]);
    }

    // over from panel on right
    if (tabsOnRight) {
      xpos = x + messageAreaPixels[0];
      glVertex2f((float) xpos, (float) ypos);
    }

  } glEnd();

  // some engines miss the corners
  /*
    glBegin(GL_POINTS); {
    glVertex2f((float) (x + messageAreaPixels[0] - 0.9f),
    (float) (y + messageAreaPixels[1] - 1));
    glVertex2f((float) (x + messageAreaPixels[0] - 1 + messageAreaPixels[2] + 1.1f),
    (float) (y + messageAreaPixels[1] - 1));
    glVertex2f((float) (x + messageAreaPixels[0] - 1 + messageAreaPixels[2] + 1.1f),
    (float) (y + messageAreaPixels[1] - 1 + messageAreaPixels[3] + 1.1f));
    if (ay != 0) {
    glVertex2f((float) (x + messageAreaPixels[0] - 1 + 200 + 0.9f),
    (float) (y + messageAreaPixels[1] - 1 + int(lineHeight + 4)
    + messageAreaPixels[3] + 1.1f));
    glVertex2f((float) (x + messageAreaPixels[0] - 0.9f),
    (float) (y + messageAreaPixels[1] - 1 + int(lineHeight + 4)
    + messageAreaPixels[3] + 1.1f));
    }
    } glEnd();
  */

  // border for radar
  if (!BZDB.isTrue(StateDatabase::BZDB_NORADAR)) {
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
  }

  glPopMatrix();
}

void			ControlPanel::resize()
{
  float radarSpace, radarSize;
  // get important metrics
  const float w = (float)window.getWidth();
  const float h = (float)window.getHeight();
  const float opacity = RENDERER.getPanelOpacity();
  radarSize = float(window.getHeight() - window.getViewHeight());
  if (opacity == 1.0f) {
    radarSize = float(window.getHeight() - window.getViewHeight());
    radarSpace = 0.0f;
  } else {
    radarSize = h * (14 + RENDERER.getRadarSize()) / 60.0f;
    radarSpace = 3.0f * w / MinY;
  }

  // compute areas in pixels x,y,w,h
  // leave off 1 pixel for the border
  radarAreaPixels[0] = radarAreaPixels[1] = (int)radarSpace + 1;
  radarAreaPixels[2] = radarAreaPixels[3] = (int)(radarSize - (radarSpace * 2.0f)) - 2;

  messageAreaPixels[0] = (int)radarSize + 1;                    // X coord
  messageAreaPixels[1] = radarAreaPixels[1];                    // Y coord
  messageAreaPixels[2] = (int)(w - radarSize - radarSpace) - 2; // Width
  messageAreaPixels[3] = radarAreaPixels[3];                    // Height
  if (BZDB.isTrue(StateDatabase::BZDB_NORADAR)) {
    messageAreaPixels[0] = (int)radarSpace + 1;
    messageAreaPixels[2] = (int)(w - (radarSpace * 2.0f)) - 2;
  }

  // if radar connected then resize it
  if (radarRenderer)
    radarRenderer->setShape(radarAreaPixels[0], radarAreaPixels[1],
			    radarAreaPixels[2], radarAreaPixels[3]);

  switch (static_cast<int>(BZDB.eval("cpanelfontsize"))) {
  case 0: { // auto
    const bool useBigFont = (messageAreaPixels[2] / 100.0f) > 10.0f;
    fontSize = useBigFont ? 12.0f : 8.0f;
    break;
    }
  case 1: // tiny
    fontSize = 6;
    break;
  case 2: // small
    fontSize = 8;
    break;
  case 3: // medium
    fontSize = 12;
    break;
  case 4: // big
    fontSize = 16;
    break;
  }

  FontManager &fm = FontManager::instance();
  fontFace = fm.getFaceID(BZDB.get("consoleFont"));

  // tab widths may have changed
  if (tabs) {
    tabTextWidth.clear();
    totalTabWidth = 0;
    const float charWidth = fm.getStrLength(fontFace, fontSize, "-");
    for (unsigned int tab = 0; tab < tabs->size(); tab++) {
      // add space for about 2-chars on each side for padding
      tabTextWidth.push_back(fm.getStrLength(fontFace, fontSize, (*tabs)[tab]) + (4.0f * charWidth));
      totalTabWidth += long(tabTextWidth[tab]);
    }
  }

  lineHeight = fm.getStrHeight(fontFace, fontSize, " ");

  maxLines = int(messageAreaPixels[3] / lineHeight);
  
  margin = lineHeight / 4.0f;

  // rewrap all the lines
  for (int i = 0; i < MessageModeCount; i++) {
    for (int j = 0; j < (int)messages[i].size(); j++) {
      messages[i][j].breakLines(messageAreaPixels[2] - 2 * margin, fontFace, fontSize);
    }
  }

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
      if (offset < (int)messages[messageMode].size())
	messagesOffset = offset;
      else
	messagesOffset = messages[messageMode].size() - 1;
      break;
    case 1:
      if (offset > 0) {
	if (messagesOffset + offset < (int)messages[messageMode].size())
	  messagesOffset += offset;
	else
	  messagesOffset = messages[messageMode].size() - 1;
      } else if (offset < 0) {
	if (messagesOffset + offset >= 0)
	  messagesOffset += offset;
	else
	  messagesOffset = 0;
      }
      break;
    case 2:
      if (offset < 0) {
	if ((int)messages[messageMode].size() >= offset)
	  messagesOffset += offset;
	else
	  messagesOffset = 0;
      }
      break;
  }
  changedMessage = numBuffers;
}

void			ControlPanel::setMessagesMode(int _messageMode)
{
  messageMode = _messageMode;
  changedMessage = numBuffers;
}

void			ControlPanel::addMessage(const std::string& line,
						 const int mode)
{
  ControlPanelMessage item(line);
  item.breakLines(messageAreaPixels[2] - 2 * margin, fontFace, fontSize);
  
  int maxScrollPages = BZDB.evalInt("scrollPages");
  if (maxScrollPages <= 0) {
    maxScrollPages = atoi(BZDB.getDefault("scrollPages").c_str());
    BZDB.setInt("scrollPages", maxScrollPages);
  }

  // Add to "All" tab
  if ((int)messages[MessageAll].size() < maxLines * maxScrollPages) {
    // not full yet so just append it
    messages[MessageAll].push_back(item);
  } else {
    // rotate list and replace oldest (in newest position after rotate)
    messages[MessageAll].erase(messages[MessageAll].begin());
    messages[MessageAll].push_back(item);
  }

  // Add to other tab
  if (mode >= MessageChat && mode <= MessageMisc) {
    if ((int)messages[mode].size() < maxLines * maxScrollPages) {
      // not full yet so just append it
      messages[mode].push_back(item);
    } else {
      // rotate list and replace oldest (in newest position after rotate)
      messages[mode].erase(messages[mode].begin());
      messages[mode].push_back(item);
    }
  }

  // this stuff has no effect on win32 (there's no console)
  extern bool echoToConsole;
  extern bool echoAnsi;
  if (echoToConsole) {
    if (echoAnsi) {
      std::cout << line << ColorStrings[ResetColor] << std::endl;
    } else {
      std::cout << stripAnsiCodes(line) << std::endl;
    }
    fflush(stdout);
  }

  changedMessage = numBuffers;
}

void			ControlPanel::setRadarRenderer(RadarRenderer* rr)
{
  radarRenderer = rr;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

