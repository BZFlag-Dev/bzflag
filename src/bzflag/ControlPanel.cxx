/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// BZFlag common header
#include "common.h"

// interface header
#include "ControlPanel.h"

// system headers
#include <iostream>
#include <algorithm>
#include <assert.h>
#include <time.h>

// common headers
#include "bzregex.h"
#include "BZDBCache.h"
#include "FontManager.h"
#include "TextUtils.h"
#include "ErrorHandler.h"
#include "global.h"

// local headers
#include "FontSizer.h"
#include "HubLink.h"
#include "SceneRenderer.h"
#include "RadarRenderer.h"
#include "bzflag.h"
#include "LocalFontFace.h"
#include "bzUnicode.h"


//============================================================================//

struct AreaInts {
  int xpos;
  int ypos;
  int xsize;
  int ysize;
};


//============================================================================//
//
// ControlPanelMessage
//

float ControlPanelMessage::prevXoffset = 0.0f;


ControlPanelMessage::ControlPanelMessage(const std::string& _data)
: data(_data)
, xoffset(0.0f)
, xoffsetFirst(0.0f)
, numlines(0)
{
}


void ControlPanelMessage::breakLines(float maxLength, int fontFace, float fontSize)
{
  lines.clear();
  numlines = 0;

  FontManager &fm = FontManager::instance();

  if (maxLength <= 0.0f) {
    return;
  }

  std::string s = data;

  bool needXoffsetAdj = false;

  const float charWidth = fm.getStringWidth(fontFace, fontSize, "-");

  // handle the vertical tabs
  std::string::size_type vPos = s.find('\v');
  if (vPos == 0) {
    if (prevXoffset < (maxLength - (2.0f * charWidth))) {
      maxLength   -= prevXoffset;
      xoffset      = prevXoffset;
      xoffsetFirst = prevXoffset;
    }
  }
  else if (vPos != std::string::npos) {
    const std::string prefix = stripAnsiCodes(s.substr(0, vPos));
    const float prefixWidth = fm.getStringWidth(fontFace, fontSize, prefix);
    if (prefixWidth < (maxLength - (2.0f * charWidth))) {
      xoffset = prefixWidth;
      prevXoffset = xoffset;
      needXoffsetAdj = true;
    }
  }

  // strip remaining '\v'
  s.erase(std::remove(s.begin(), s.end(), '\v'), s.end());

  // get message and its length
  const char* msg = s.c_str();
  int lineLen = (int)s.length();

  // if there are tabs in the message, find the last one
  int lastTab = (int)s.rfind('\t');

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
    if ((fm.getStringWidth(fontFace, fontSize, msg) <= maxLength) && (lastTab <= 0)) {
      n = lineLen;
    }
    else {

      n = 0;

      while ((n < lineLen) &&
	     (fm.getStringWidth(fontFace, fontSize,
				std::string(msg, ((++UTF8StringItr(msg + n)).getBufferFromHere() - msg)))
	      < maxLength)) {
	if (msg[n] != ESC_CHAR) {
	  n = static_cast<int>((++UTF8StringItr(msg+n)).getBufferFromHere() - msg);
        }
        else {
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
	    while ((n < lineLen) &&
		   ((msg[n] == ';') ||
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
	}

	if (TextUtils::isWhitespace(msg[n])) {
	  lastWhitespace = n;
	  // Tabs break out into their own message.  These get dealt with
	  // in ControlPanel::render, which will increment x instead of y.
	  if (msg[n] == '\t') {
	    break;
          }
	}
      }
    }

    if (lastWhitespace > 0) {
      n = lastWhitespace;
    }

    // message
    lines.push_back(cumulativeANSICodes + std::string(msg, n));

    if (msg[n] != '\t') {
      numlines++;
    }

    // adjust the maxLength for non-first lines
    if (needXoffsetAdj) {
      maxLength -= xoffset;
      needXoffsetAdj = false;
    }

    // account for portion broken
    msg += n;
    lineLen -= n;
    lastTab -= n;

    // eat leading whitespace after breaks
    if (xoffset != 0.0f) {
      while (TextUtils::isWhitespace(msg[0]) && (msg[0] != '\t')) {
        msg++;
        lineLen--;
        lastTab--;
      }
    }
  }
}


const ControlPanel::MessageQueue* ControlPanel::getTabMessages(int tabID)
{
  if (!validTab(tabID)) {
    return NULL;
  }
  return &(tabs[tabID]->messages);
}


int ControlPanel::getTabMessageCount(int tabID)
{
  if (!validTab(tabID)) {
    return -1;
  }
  return tabs[tabID]->msgCount;
}


//============================================================================//
//
// ControlPanel
//

ControlPanel::ControlPanel(MainWindow& _mainWindow, SceneRenderer& _renderer)
: activeTab(MessageAll)
, tabsOnRight(true)
, totalTabWidth(0)
, window(_mainWindow)
, resized(false)
, numBuffers(2)
, changedMessage(0)
, radarRenderer(NULL)
, renderer(&_renderer)
, fontFace(NULL)
, dimming(1.0f)
, du(0)
, dv(0)
, teamColor(0.0f, 0.0f, 0.0f, 1.0f)
{
  setControlColor();

  // make sure we're notified when MainWindow resizes or is exposed
  window.getWindow()->addResizeCallback(resizeCallback, this);
  window.getWindow()->addExposeCallback(exposeCallback, this);
  BZDB.addCallback("debugLevel",   bzdbCallback, this);
  BZDB.addCallback("displayRadar", bzdbCallback, this);
  BZDB.addCallback(BZDBNAMES.RADARLIMIT, bzdbCallback, this);

  // other initialization
  radarRect.xpos  = 0;
  radarRect.ypos  = 0;
  radarRect.xsize = 0;
  radarRect.ysize = 0;
  messageRect.xpos  = 0;
  messageRect.ypos  = 0;
  messageRect.xsize = 0;
  messageRect.ysize = 0;
  teamColor[0] = teamColor[1] = teamColor[2] = (float)0.0f;

  maxLines = 30;

  // - - - - - - - - -   label     locked  allSrc  allDst
  tabs.push_back(new Tab("All",    true,   true,   true));
  tabs.push_back(new Tab("Chat",   true,   true,   true));
  tabs.push_back(new Tab("Server", true,   true,   true));
  tabs.push_back(new Tab("Misc",   true,   true,   true));
  tabs.push_back(new Tab("Debug",  true,   false,  true));

  setupTabMap();

  resize(); // need resize to set up font and window dimensions

  // register after we're fully initialized
  registerLoggingProc(loggingCallback, this);
}

ControlPanel::~ControlPanel()
{
  for (int t = 0; t < (int)tabs.size(); t++) {
    delete tabs[t];
  }
  tabs.clear();

  // don't notify me anymore (cos you can't wake the dead!)
  unregisterLoggingProc(loggingCallback, this);
  window.getWindow()->removeResizeCallback(resizeCallback, this);
  window.getWindow()->removeExposeCallback(exposeCallback, this);
  BZDB.removeCallback("debugLevel",   bzdbCallback, this);
  BZDB.removeCallback("displayRadar", bzdbCallback, this);
  BZDB.removeCallback(BZDBNAMES.RADARLIMIT, bzdbCallback, this);

  // release font face
  if (fontFace) {
    LocalFontFace::release(fontFace);
  }

  if (echoToConsole && echoAnsi) {
    std::cout << ColorStrings[FinalResetColor] << std::flush;
  }
}


void ControlPanel::loggingCallback(int level,
                                   const std::string& rawMsg, void* data)
{
// -- always store the debug messages
//  if (level > debugLevel) {
//    return;
//  }
  std::string msg = rawMsg;
  while (!msg.empty() && (msg[msg.size() - 1] == '\n')) {
    msg.resize(msg.size() - 1);
  }
  std::string color = "";
  if (level >= 0) {
    switch (level) {
      case 1:  { color = ANSI_STR_FG_GREEN;   break; }
      case 2:  { color = ANSI_STR_FG_CYAN;    break; }
      case 3:  { color = ANSI_STR_FG_BLUE;    break; }
      case 4:  { color = ANSI_STR_FG_YELLOW;  break; }
      case 5:  { color = ANSI_STR_FG_ORANGE;  break; }
      case 6:  { color = ANSI_STR_FG_RED;     break; }
      case 7:  { color = ANSI_STR_FG_MAGENTA; break; }
      case 8:  { color = ANSI_STR_FG_WHITE;   break; }
      case 9:  { color = ANSI_STR_FG_BLACK;   break; }
      default: { color = ANSI_STR_UNDERLINE;  break; }
    }
  }
  ((ControlPanel*)data)->addMessage(color + msg, ControlPanel::MessageDebug);
}


void ControlPanel::bzdbCallback(const std::string& /*name*/, void* data)
{
  ((ControlPanel*)data)->resize();
}


void ControlPanel::setupTabMap()
{
  tabMap.clear();

  for (int t = 0; t < (int)tabs.size(); t++) {
    const std::string& label = tabs[t]->label;
    tabMap[label] = t;
    tabMap[stripAnsiCodes(label)] = t;
    if (tabs[t]->locked) {
      tabMap[TextUtils::tolower(label)] = t;
    }
  }
}


void ControlPanel::setControlColor(const fvec4* color)
{
  if (color != NULL) {
    teamColor = *color;
  } else {
    teamColor = fvec4(0.0f, 0.0f, 0.0f, 1.0f);
  }
}


void ControlPanel::render(SceneRenderer& _renderer)
{
  const float opacity = _renderer.getPanelOpacity();

  if (!BZDB.isTrue("displayConsole") && (opacity != 1.0f)) {
    return; // NOTE: always draw the console if it's fully opaque
  }

  if (!resized) {
    resize();
  }

  // optimization for software rendering folks
  if (!changedMessage && (opacity == 1.0f)) {
    return;
  }

  int i, j;
  const int x = window.getOriginX();
  const int y = window.getOriginY();
  const int w = window.getWidth();
  const int tabStyle = BZDB.evalInt("showtabs");
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
  fm.setOpacity(dimming);

  if (changedMessage > 0) {
    changedMessage--;
  }
  const float fx = messageRect.xpos + margin;
  float       fy = messageRect.ypos + margin + 1.0f;
  const int   ay = ((opacity == 1.0f) || !showTabs) ? 0 : int(lineHeight + 4);

  glScissor(messageRect.xpos + x - 1,
	    messageRect.ypos + y,
	    messageRect.xsize + 1,
	    messageRect.ysize + ay);
  OpenGLGState::resetState();

  if (opacity > 0.0f) {
    // nice blended messages background
    const bool blended = BZDBCache::blend && (opacity < 1.0f);
    if (blended) {
      glEnable(GL_BLEND);
    }

    // clear the background
    glColor4f(0.0f, 0.0f, 0.0f, opacity);
    glRecti(messageRect.xpos - 1, // clear an extra pixel column to
	    messageRect.ypos,     //   simplify fuzzy float stuff later 
	    messageRect.xpos + messageRect.xsize,
	    messageRect.ypos + messageRect.ysize);

    // display tabs for chat sections
    if (showTabs) {
      drawTabBoxes(ay);
    }

    if (blended) {
      glDisable(GL_BLEND);
    }
  }

  drawScrollBar();

  if (showTabs) {
    drawTabLabels(ay);
  }

  /* draw messages
   *
   * It works by first breaking the string into a vector of strings
   * (done elsewhere), each of which will fit the control panel, and
   * tallying the number of lines, then moving up the proper number of
   * lines and displaying downward -- that is, it kinda backtracks for
   * each line that will wrap.
   *
   * messageRect.xsize = Width of Message Window in Pixels.
   *
   * maxLines = Max messages lines that can be displayed at once per
   * page.  This COULD be a BZDB parameter (but isn't).
   *
   * maxScrollPages = This number * maxLines is the total maximum
   * lines of messages (and scrollback). It is stored as a BZDB
   * parameter.
   */

  glScissor(messageRect.xpos + x,
	    messageRect.ypos + y,
	    messageRect.xsize,
	    messageRect.ysize - (showTabs ? int(lineHeight + 4) : 0) + ay);

  if (activeTab >= 0) {
    i = (int)tabs[activeTab]->messages.size() - 1;
  } else {
    i = -1;
  }
  if ((i >= 0) && (tabs[activeTab]->offset > 0)) {
    i -= tabs[activeTab]->offset;
    if (i < 0) {
      i = 0;
    }
  }

  const std::string highlightPattern = BZDB.get("highlightPattern");
  bool useHighlight = (highlightPattern.size() > 0);
  regex_t re;
  if (useHighlight) {
    if (regcomp(&re, highlightPattern.c_str(), REG_EXTENDED | REG_ICASE) != 0) {
      useHighlight = false; // bad regex
    }
  }

  const int topicLines = tabs[activeTab]->topic.numlines;
  const int maxNormLines = (maxLines - ((topicLines > 0) ? (topicLines + 1) : 0));

  for (j = 0; (i >= 0) && (j < maxNormLines); i--) {
    // draw each line of text
    const ControlPanelMessage& cpMsg = tabs[activeTab]->messages[i];
    int numLines = cpMsg.numlines;
    int numStrings = (int)cpMsg.lines.size();
    int msgy = numLines - 1;
    int msgx = 0;

    // see if this message need to be highlighted (check each line)
    bool highlight = false;
    if (useHighlight) {
      for (int l = 0; l < numStrings; l++)  {
	const std::string &msg = cpMsg.lines[l];
	std::string raw = stripAnsiCodes(msg);
	if (regexec(&re, raw.c_str(), 0, NULL, 0) == 0) {
	  highlight = true;
	}
      }
    }

    // default to drawing text in white
    float whiteColor[4] = {1.0f, 1.0f, 1.0f, dimming};
    glColor4fv(whiteColor);

    bool isTab = false;

    for (int l = 0; l < numStrings; l++)  {
      const std::string &msg = cpMsg.lines[l];

      // Tab chars move horizontally instead of vertically
      // It doesn't matter where in the string the tab char is
      // Usually it will be like <ansi><ansi><ansi>\ttext
      // We use 1 tabstop spaced 1/3 of the way across the controlpanel
      isTab = (msg.find('\t') != std::string::npos);
      if (isTab) {
	msgx += messageRect.xsize / 3;
	msgy++;
      } else {
	msgx = 0;
      }

      assert(msgy >= 0);

      // only draw message if inside message area
      if ((j + msgy) < maxNormLines) {
        const float xoff = (l == 0) ? cpMsg.xoffsetFirst : cpMsg.xoffset;
	if (!highlight) {
	  fm.drawString(fx + msgx + xoff, fy + msgy * lineHeight, 0,
	                fontFace->getFMFace(), fontSize, msg);
	}
	else {
	  // highlight this line
	  std::string newMsg = ANSI_STR_PULSATING;
	  newMsg += ANSI_STR_UNDERLINE;
	  newMsg += ANSI_STR_FG_CYAN;
	  newMsg += stripAnsiCodes(msg);
	  fm.drawString(fx + msgx + xoff, fy + msgy * lineHeight, 0,
	                fontFace->getFMFace(), fontSize, newMsg);
	}
      }

      // next line
      msgy--;
    }

    j += numLines;

    fy += lineHeight * numLines;
  }

  // draw the topic lines
  fy = messageRect.ypos + messageRect.ysize - margin;
  for (i = 0; i < topicLines; i++) {
    j = (topicLines - (i + 1));
    const std::string& line = tabs[activeTab]->topic.lines[j];
    fm.drawString(fx, fy - ((i + 1) * lineHeight), 0,
                  fontFace->getFMFace(), fontSize, line);
  }

  // free the regex memory
  if (useHighlight) {
    regfree(&re);
  }

  glScissor(messageRect.xpos + x - 2,
	    messageRect.ypos + y - 2,
	    messageRect.xsize + 3,
	    messageRect.ysize + 33);
  OpenGLGState::resetState();

  drawOutline(ay);

  glColor4f(teamColor[0], teamColor[1], teamColor[2],1.0f);

  glPopMatrix();

  fm.setOpacity(1.0f);
}


void ControlPanel::drawScrollBar()
{
  if (activeTab < 0) {
    return;
  }
  const Tab* tab = tabs[activeTab];
  if (tab->offset == 0) {
    return; // only show the scroll indicator if not at the end
  }
  const int lines = int(tab->messages.size());
  if (lines > 0) {
    const float size = std::max(float(maxLines) / lines, 0.02f);
    const float offset = float(tab->offset) / lines;
    const int maxTop = messageRect.ypos + messageRect.ysize;
    int top = messageRect.ypos + int((offset + size) * (float)messageRect.ysize);
    if (top > maxTop) {
      top = maxTop;
    }
    glColor3f(0.7f, 0.7f, 0.7f);
    glRecti(messageRect.xpos,
            messageRect.ypos + int(offset * (float)messageRect.ysize),
            messageRect.xpos + 2,
            top);
  }
}


void ControlPanel::drawTabBoxes(int yOffset)
{
  const IntRect& rect = messageRect;

  const int tabHeight = (int)lineHeight + 4;

  long int drawnTabWidth = 0;
  for (int t = 0; t < (int)tabs.size(); t++) {
    const Tab* tab = tabs[t];
    if (!tab->visible) {
      continue;
    }
    const int tabWidth = (int)tab->width;

    // current mode is given a dark background to match the control panel
    if (activeTab == t) {
      glColor4f(0.0f, 0.0f, 0.0f, RENDERER.getPanelOpacity());
    } else {
      glColor4f(0.10f, 0.10f, 0.10f, RENDERER.getPanelOpacity());
    }

    if (tabsOnRight) {
      // draw the tabs on the right side
      glRecti(rect.xpos + rect.xsize - totalTabWidth + drawnTabWidth,
              rect.ypos + rect.ysize + yOffset - tabHeight,
              rect.xpos + rect.xsize - totalTabWidth + drawnTabWidth + tabWidth,
              rect.ypos + rect.ysize + yOffset);
    } else {
      // draw the tabs on the left side
      glRecti(rect.xpos + drawnTabWidth,
              rect.ypos + rect.ysize + yOffset - tabHeight,
              rect.xpos + drawnTabWidth + tabWidth,
              rect.ypos + rect.ysize + yOffset);
    }

    drawnTabWidth += long(tab->width);
  }
}


void ControlPanel::drawTabLabels(int yOffset)
{
  FontManager &fm = FontManager::instance();
  const IntRect& rect = messageRect;

  long int drawnTabWidth = 0;
  for (int t = 0; t < (int)tabs.size(); t++) {
    const Tab* tab = tabs[t];
    if (!tab->visible) {
      continue;
    }

    // current mode is bright, others are not so bright
    if (activeTab == t) {
      glColor4f(1.0f, 1.0f, 1.0f, dimming);
    } else if (tab->unread) {
      glColor4f(0.5f, 0.0f, 0.0f, dimming);
    } else {
      glColor4f(0.5f, 0.5f, 0.5f, dimming);
    }

    if (tabsOnRight) {
      // draw the tabs on the right side (with one letter padding)
      fm.drawString(rect.xpos + rect.xsize - totalTabWidth + drawnTabWidth + floorf(fontSize),
                    rect.ypos + rect.ysize - floorf(lineHeight * 0.9f) + yOffset,
                    0.0f, fontFace->getFMFace(), (float)fontSize, tab->label);
    } else {
      // draw the tabs on the left side (with one letter padding)
      fm.drawString(rect.xpos + drawnTabWidth + floorf(fontSize),
                    rect.ypos + rect.ysize - floorf(lineHeight * 0.9f) + yOffset,
                    0.0f, fontFace->getFMFace(), (float)fontSize, tab->label);
    }
    drawnTabWidth += long(tab->width);
  }
}


void ControlPanel::drawOutline(int yOffset)
{
  const fvec2 halfPixel(0.5f, 0.5f);

  if (BZDBCache::blend) {
    glEnable(GL_BLEND);
  }

  float opacity = RENDERER.getPanelOpacity();
  const float fudgeFactor = BZDBCache::hudGUIBorderOpacityFactor;
  if (opacity < 1.0f) {
    opacity = (opacity * fudgeFactor) + (1.0f - fudgeFactor);
  }

  glColor4f(teamColor[0], teamColor[1], teamColor[2], opacity);

  const int x = window.getOriginX();
  const int y = window.getOriginY();

  glBegin(GL_LINE_LOOP); {
    long xpos, ypos;

    // bottom left
     xpos = x + messageRect.xpos - 1;
     ypos = y + messageRect.ypos - 1;
    glVertex2fv(fvec2(xpos, ypos) + halfPixel);

    // bottom right
    xpos += messageRect.xsize + 1;
    glVertex2fv(fvec2(xpos, ypos) + halfPixel);

    // top right
    ypos += messageRect.ysize + 1;
    glVertex2fv(fvec2(xpos, ypos) + halfPixel);

    // over to panel on left
    if (!tabsOnRight) {
      xpos = x + messageRect.xpos + totalTabWidth;
      glVertex2fv(fvec2(xpos, ypos) + halfPixel);
    }

    // across the top from right to left
    long int drawnTabWidth = 0;
    for (int t = (int)tabs.size() - 1; t >= 0; t--) {
      const Tab* tab = tabs[t];
      if (!tab->visible) {
        continue;
      }
      if (activeTab == t) {
	ypos += yOffset;
        glVertex2fv(fvec2(xpos, ypos) + halfPixel);

	xpos -= long(tab->width) + 1;
        glVertex2fv(fvec2(xpos, ypos) + halfPixel);

	ypos -= yOffset;
        glVertex2fv(fvec2(xpos, ypos) + halfPixel);
      }
      else {
	xpos -= long(tab->width);
        glVertex2fv(fvec2(xpos, ypos) + halfPixel);
      }
      drawnTabWidth += long(tab->width);
    }

    // over from panel on right
    xpos = x + messageRect.xpos - 1;
    glVertex2fv(fvec2(xpos, ypos) + halfPixel);

  } glEnd();

  if (BZDBCache::blend) {
    glDisable(GL_BLEND);
  }
}


void ControlPanel::resize()
{
  tabs[MessageDebug]->visible = (debugLevel > 0);

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
  radarRect.xpos = radarRect.ypos = (int)radarSpace + 1;
  radarRect.xsize = radarRect.ysize = (int)(radarSize - (radarSpace * 2.0f)) - 2;

  messageRect.xpos  = (int)radarSize + 1; // X coord
  messageRect.ypos  = radarRect.ypos; // Y coord
  messageRect.xsize = (int)(w - radarSize - radarSpace) - 2; // Width
  messageRect.ysize = radarRect.ysize; // Height
  if (!BZDB.isTrue("displayRadar") || (BZDBCache::radarLimit <= 0.0f)) {
    messageRect.xpos  = (int)radarSpace + 1;
    messageRect.xsize = (int)(w - (radarSpace * 2.0f)) - 2;
  }

  // if radar connected then resize it
  if (radarRenderer) {
    radarRenderer->setShape(radarRect.xpos, radarRect.ypos,
                            radarRect.xsize, radarRect.ysize);
  }

  FontManager &fm = FontManager::instance();
  if (!fontFace) {
    fontFace = LocalFontFace::create("consoleFont");
  }

  FontSizer fs = FontSizer(w, h);
  fontSize = fs.getFontSize(fontFace, "consoleFontSize");

  // tab widths may have changed
  totalTabWidth = 0;
  const float charWidth = fm.getStringWidth(fontFace->getFMFace(), fontSize, "-");
  for (int t = 0; t < (int)tabs.size(); t++) {
    Tab* tab = tabs[t];
    if (!tab->visible) {
      tab->width = 0.0f;
      continue;
    }

    // add space for about 2-chars on each side for padding
    tab->width = 
      fm.getStringWidth(fontFace->getFMFace(), fontSize, tab->label)
      + (4.0f * charWidth);

    totalTabWidth += long(tab->width);
  }

  lineHeight = fm.getStringHeight(fontFace->getFMFace(), fontSize);
  lineHeight = ceilf(lineHeight);

  maxLines = int(messageRect.ysize / lineHeight);

  margin = lineHeight / 4.0f;

  // rewrap all the lines
  ControlPanelMessage::prevXoffset = 0.0f;
  for (int i = 0; i < (int)tabs.size(); i++) {
    for (int j = 0; j < (int)tabs[i]->messages.size(); j++) {
      tabs[i]->messages[j].breakLines(messageRect.xsize - 2 * margin,
                                      fontFace->getFMFace(), fontSize);
    }
    tabs[i]->topic.breakLines(messageRect.xsize - 2 * margin,
                              fontFace->getFMFace(), fontSize);
  }

  // note that we've been resized at least once
  resized = true;

  invalidate();
}


void ControlPanel::resizeCallback(void* self)
{
  ((ControlPanel*)self)->resize();
}


void ControlPanel::setNumberOfFrameBuffers(int n)
{
  numBuffers = n;
}


void ControlPanel::invalidate()
{
  if (numBuffers) {
    changedMessage = numBuffers;
  } else {
    changedMessage++;
  }
}


void ControlPanel::exposeCallback(void* self)
{
  ((ControlPanel*)self)->invalidate();
}


void ControlPanel::setMessagesOffset(int offset, int whence, bool paged)
{
  if (activeTab < 0) {
    return;
  }

  if (paged) {
    if (abs(offset) <= 1) {
      offset = offset * (maxLines - 1);
    } else {
      offset = offset * maxLines;
    }
  }

  // offset = offset from whence (offset of 0 is the bottom/most recent)
  // whence = 0, 1, or 2 (akin to SEEK_SET, SEEK_CUR, SEEK_END)

  switch (whence) {
    case 0: {
      if (offset < (int)tabs[activeTab]->messages.size()) {
        tabs[activeTab]->offset = offset;
      } else {
        tabs[activeTab]->offset = (int)tabs[activeTab]->messages.size() - 1;
      }
      break;
    }
    case 1: {
      if (offset > 0) {
        if (tabs[activeTab]->offset + offset < (int)tabs[activeTab]->messages.size()) {
          tabs[activeTab]->offset += offset;
        } else {
          tabs[activeTab]->offset = (int)tabs[activeTab]->messages.size() - 1;
        }
      } else if (offset < 0) {
        if (tabs[activeTab]->offset + offset >= 0) {
          tabs[activeTab]->offset += offset;
        } else {
          tabs[activeTab]->offset = 0;
        }
      }
      break;
    }
    case 2: {
      if (offset < 0) {
        if ((int)tabs[activeTab]->messages.size() >= offset) {
          tabs[activeTab]->offset += offset;
        } else {
          tabs[activeTab]->offset = 0;
        }
      }
      break;
    }
  }

  invalidate();
}


bool ControlPanel::setActiveTab(int tabID)
{
  if (!validTab(tabID)) {
    return false;
  }

  if (!tabs[tabID]->visible) {
    return false;
  }

  activeTab = tabID;

  if (hubLink) {
    hubLink->activeTabChanged();
  }

  if (activeTab == MessageAll) {
    for (int i = 0; i < (int)tabs.size(); i++) {
      if (tabs[i]->allSrc) {
        tabs[i]->unread = false;
      }
    }
  }
  else if (activeTab >= MessageChat) {
    tabs[activeTab]->unread = false;
  }

  invalidate();

  return true;
}


bool ControlPanel::isTabLocked(int tabID) const
{
  if (!validTab(tabID)) {
    return false;
  }
  return tabs[tabID]->locked;
}


bool ControlPanel::isTabVisible(int tabID) const
{
  if (!validTab(tabID)) {
    return false;
  }
  return tabs[tabID]->visible;
}


void ControlPanel::addMessage(const std::string& line, int realmode)
{
  ControlPanelMessage item(line);
  item.breakLines(messageRect.xsize - 2 * margin, fontFace->getFMFace(), fontSize);

  int _maxScrollPages = BZDB.evalInt("scrollPages");
  if (_maxScrollPages <= 0) {
    _maxScrollPages = atoi(BZDB.getDefault("scrollPages").c_str());
    BZDB.setInt("scrollPages", _maxScrollPages);
  }

  // the effective tab
  const int tabmode = (realmode == MessageCurrent) ? activeTab : realmode;

  const bool allSrc = !validTab(tabmode) || tabs[tabmode]->allSrc;

  // add to the appropriate tabs
  for (int t = MessageAll; t < (int)tabs.size(); t++) {
    Tab* tab = tabs[t];
    if ((t == tabmode) // add to its own mode
	// add to the All tab, unless not a source for All, or Current mode
	|| ((t == MessageAll) && allSrc && (realmode != MessageCurrent))
	// add to all tabs unless the tab is not a destination for MessageAllTabs
	|| ((realmode == MessageAllTabs) && tab->allDst)) {

      // insert the message into the tab
      if ((int)tab->messages.size() < (maxLines * _maxScrollPages)) {
	// not full yet so just append it
	tab->messages.push_back(item);
      } else {
	// rotate list and replace oldest (in newest position after rotate)
	tab->messages.pop_front();
	tab->messages.push_back(item);
      }
      tab->msgCount++;

      // visible changes, force a console refresh
      if (activeTab == t) {
	invalidate();
      }

      // mark the tab as unread (if viewing tabs)
      const bool showTabs = (BZDB.evalInt("showtabs") > 0);
      if (showTabs && (activeTab != t) && (activeTab >= 0) &&
          ((activeTab != MessageAll) || !tab->allSrc)) {
	tab->unread = true;
      }
    }
  }

  if (echoToConsole) {
    std::string echoOut = line;
    echoOut.erase(std::remove(echoOut.begin(), echoOut.end(), '\v'), echoOut.end());
    if (echoAnsi) {
      echoOut += ColorStrings[ResetColor];
    } else {
      echoOut = stripAnsiCodes(line);
    }
#ifndef _WIN32
    std::cout << echoOut << std::endl;
    fflush(stdout);
#else
    // this is cheap but it will work on windows
    FILE *fp = fopen("stdout.txt", "a+");
    if (fp) {
      fprintf(fp,"%s\n", echoOut.c_str());
      fclose(fp);
    }
#endif
  }
}


void ControlPanel::addMessage(const std::string& line,
                              const std::string& tabLabel)
{
  const int tabID = getTabID(tabLabel);
  if (tabID < 0) {
    return;
  }
  addMessage(line, tabID);
}


bool ControlPanel::addTab(const std::string& label, bool allSrc, bool allDst)
{
  if (label.empty()) {
    return false;
  }
  if (getTabID(label) >= 0) {
    return false; // already exists
  }

  tabs.push_back(new Tab(label, false, allSrc, allDst));
  setupTabMap();
  resize();

  if (hubLink) {
    hubLink->tabAdded(label);
  }

  return true;
}


bool ControlPanel::removeTab(const std::string& label)
{
  for (int t = 0; t < (int)tabs.size(); t++) {
    Tab* tab = tabs[t];
    if (tab->label == label) { // an exact test, not using getTabID()
      if (tab->locked) {
        return false;
      }
      delete tab;
      tabs.erase(tabs.begin() + t);
      setupTabMap();
      if (activeTab >= (int)tabs.size()) {
        setActiveTab(MessageAll);
      }
      resize();
      if (hubLink) {
        hubLink->tabRemoved(label);
      }
      return true;
    }
  }
  return false;
}


bool ControlPanel::renameTab(const std::string& oldLabel,
                             const std::string& newLabel)
{
  if (newLabel.empty()) {
    return false;
  }
  for (int t = 0; t < (int)tabs.size(); t++) {
    Tab* tab = tabs[t];
    if (tab->label == oldLabel) {
      if (tab->locked) {
        return false;
      }
      const int newTab = getTabID(newLabel);
      if (validTab(newTab) && (newTab != t)) {
        return false;
      }
      tab->label = newLabel;
      setupTabMap();
      resize();
      return true;
    }
  }
  return false;
}


int ControlPanel::getTabID(const std::string& label) const
{
  TabMap::const_iterator it = tabMap.find(label);
  if (it == tabMap.end()) {
    return -1;
  }
  return it->second;
}


bool ControlPanel::shiftTab(int tabID, int distance)
{
  if (!validTab(tabID) || tabs[tabID]->locked || (distance == 0)) {
    return false;
  }

  const int swapID = (distance > 0) ? (tabID + 1) : (tabID - 1);
  if (!validTab(swapID) || tabs[swapID]->locked) {
    return false;
  }

  // swap the tabs
  Tab* swapTab = tabs[swapID];
  tabs[swapID] = tabs[tabID];
  tabs[tabID]  = swapTab;

  if (activeTab == tabID) {
    activeTab = swapID;
    if (hubLink) {
      hubLink->activeTabChanged();
    }
  }

  setupTabMap();
  resize();


  return true;
}


bool ControlPanel::clearTab(int tabID)
{
  if (!validTab(tabID) || tabs[tabID]->locked) {
    return false;
  }
  tabs[tabID]->messages.clear();
  return true;
}


const std::string& ControlPanel::getTabLabel(int tabID) const
{
  static const std::string empty = "";
  if (!validTab(tabID)) {
    return empty;
  }
  return tabs[tabID]->label;
}


const std::string& ControlPanel::getTabTopic(int tabID) const
{
  static const std::string empty = "";
  if (!validTab(tabID)) {
    return empty;
  }
  return tabs[tabID]->topic.data;
}


bool ControlPanel::setTabTopic(int tabID, const std::string& topic)
{
  if (!validTab(tabID)) {
    return false;
  }
  tabs[tabID]->topic.data = topic;
  tabs[tabID]->topic.breakLines(messageRect.xsize - 2 * margin,
                                fontFace->getFMFace(), fontSize);
  return true;
}


bool ControlPanel::saveMessages(const std::string& filename, bool stripAnsi,
                                const std::string& tabLabel) const
{
  // pick the tab to save
  const MessageQueue* msgs = &(tabs[MessageAll]->messages);
  if (!tabLabel.empty()) {
    const int tabID = getTabID(tabLabel);
    if (tabID < 0) {
      return false;
    }
    msgs = &(tabs[tabID]->messages);
  }

  FILE* file = fopen(filename.c_str(), "a+");
  if (!file) {
    return false;
  }

  const time_t nowTime = time(NULL);
  fprintf(file, "\n----------------------------------------\n");
  fprintf(file, "Messages saved: %s", ctime(&nowTime));
  fprintf(file, "----------------------------------------\n\n");

  MessageQueue::const_iterator msg;
  for (msg = msgs->begin(); msg != msgs->end(); ++msg) {
    const std::string& line = msg->data;
    if (stripAnsi) {
      fprintf(file, "%s\n", stripAnsiCodes(line));
    } else {
      fprintf(file, "%s%s\n", line.c_str(), ColorStrings[ResetColor]);
    }
  }

  fclose(file);

  return true;
}


void ControlPanel::setRadarRenderer(RadarRenderer* rr)
{
  radarRenderer = rr;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
