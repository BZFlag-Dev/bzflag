/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "HUDRenderer.h"

// system headers
#include <time.h>

/* common implementation headers */
#include "BundleMgr.h"
#include "Bundle.h"
#include "FontManager.h"
#include "BZDBCache.h"

/* local implementation headers */
#include "LocalPlayer.h"
#include "World.h"
#include "HUDui.h"
#include "Roaming.h"
#include "playing.h"


//
// HUDRenderer
//

// headingOffset:  the number of degrees from the center of the heading
// strip display to either side.  altitudeOffset is similar.
const float		HUDRenderer::altitudeOffset = 20.0f;
const GLfloat		HUDRenderer::black[3] = { 0.0f, 0.0f, 0.0f };
std::string		HUDRenderer::headingLabel[36];
std::string		HUDRenderer::restartLabelFormat("Press %s to start");
std::string		HUDRenderer::resumeLabel("Press Pause to resume");
std::string		HUDRenderer::autoPilotLabel("AutoPilot on");
std::string		HUDRenderer::cancelDestructLabel("Press Destruct to cancel");
std::string		HUDRenderer::gameOverLabel("GAME OVER");

HUDRenderer::HUDRenderer(const BzfDisplay* _display,
				const SceneRenderer& renderer) :
				display(_display),
				window(renderer.getWindow()),
				firstRender(true),
				playing(false),
				roaming(false),
				dim(false),
				numPlayers(0),
				timeLeft(~0u),
				playerHasHighScore(false),
				teamHasHighScore(false),
				heading(0.0),
				altitude(0.0),
				altitudeTape(false),
				fps(-1.0),
				drawTime(-1.0),
				restartLabel(restartLabelFormat),
				showCompose(false),
				showCracks(true),
				dater(false),
				lastTimeChange(time(NULL)),
				triangleCount(0),
				radarTriangleCount(0)
{
  if (BZDB.eval("timedate") == 0) //we just want the time
    dater = false;
  else if (BZDB.eval("timedate") == 1) //just the date
    dater = true;
  int i;

  // initialize colors
  hudColor[0] = 1.0f;
  hudColor[1] = 0.625f;
  hudColor[2] = 0.125f;
  messageColor[0] = 1.0f;
  messageColor[1] = 1.0f;
  messageColor[2] = 1.0f;
  warningColor[0] = 1.0f;
  warningColor[1] = 0.0f;
  warningColor[2] = 0.0f;

  // make sure we're notified when MainWindow resizes
  window.getWindow()->addResizeCallback(resizeCallback, this);

  // initialize heading and altitude labels
  if (headingLabel[0].length() == 0) {
    char buf[10];
    for (i = 0; i < 36; i++) {
      sprintf(buf, "%d", i * 10);
      headingLabel[i] = std::string(buf);
    }
  }

  // initialize clocks
  globalClock.setClock(-1.0f, 0.8f, 0.4f);
  scoreClock.setClock(-1.0f, 0.5f, 0.2f);

  // create compose dialog
  composeTypeIn = new HUDuiTypeIn();
  composeTypeIn->setLabel("Send:");
  composeTypeIn->setMaxLength(MessageLen - 1);
  composeTypeIn->showFocus(false);

  // create scoreboard renderer
  scoreboard = new ScoreboardRenderer();

  // initialize fonts
  resize(true);
}

HUDRenderer::~HUDRenderer()
{
  // don't notify me anymore (cos you can't wake the dead!)
  window.getWindow()->removeResizeCallback(resizeCallback, this);

  // release ui controls
  delete composeTypeIn;
}


ScoreboardRenderer *HUDRenderer::getScoreboard ()
{
  return scoreboard;
}



void			HUDRenderer::resizeCallback(void* self)
{
  ((HUDRenderer*)self)->resize(false);
}

void			HUDRenderer::resize(bool firstTime)
{
  // get important metrics
  const int w = firstTime ? MinX : window.getWidth();
  const int vh = firstTime ? MinY : window.getViewHeight();

  // compute good targeting box sizes
  {
    const float xScale = (float)w / (float) MinX;
    const float yScale = (float)vh / (float) MinY;
    const float scale = (xScale < yScale) ? xScale : yScale;
    const float effScale =  scale * ( 0.7f + RENDERER.getMaxMotionFactor() / 16.667f);
    maxMotionSize = (int)((float)MaxMotionSize * effScale);
    noMotionSize = (int)((float)NoMotionSize * effScale / 2.0f);
    headingOffset = 22.0f * (effScale > 1.0f ? 1.0f : effScale);
  }

  // initialize readout spacings
  headingMarkSpacing = (int)(5.0f * float(maxMotionSize) / headingOffset);
  altitudeMarkSpacing = floorf(5.0f * float(maxMotionSize) / altitudeOffset);

  setBigFontSize(w, vh);
  setAlertFontSize(w, vh);
  setMajorFontSize(w, vh);
  setMinorFontSize(w, vh);
  setHeadingFontSize(w, vh);
  setComposeFontSize(w, vh);
  setLabelsFontSize(w, vh);

  // set scoreboard window size & location
  const float sby = window.getViewHeight() - majorFontHeight - alertFontHeight * 2.0f;
  scoreboard->setWindowSize (0.01f * window.getWidth(), sby,
	0.98f * window.getWidth(), sby);
}

int			HUDRenderer::getNoMotionSize() const
{
  return noMotionSize;
}

int			HUDRenderer::getMaxMotionSize() const
{
  return maxMotionSize;
}

void			HUDRenderer::setBigFontSize(int, int height)
{
  const float s = (float)height / 22.0f;
  FontManager &fm = FontManager::instance();
  bigFontFace = fm.getFaceID(BZDB.get("sansSerifFont"));
  bigFontSize = floorf(s);

  restartLabelWidth = fm.getStrLength(bigFontFace, bigFontSize, restartLabel);
  resumeLabelWidth = fm.getStrLength(bigFontFace, bigFontSize, resumeLabel);
  cancelDestructLabelWidth = fm.getStrLength(bigFontFace, bigFontSize, cancelDestructLabel);
  gameOverLabelWidth = fm.getStrLength(bigFontFace, bigFontSize, gameOverLabel);
  autoPilotWidth = fm.getStrLength(bigFontFace, bigFontSize, autoPilotLabel);
}

void			HUDRenderer::setAlertFontSize(int, int height)
{
  const float s = (float)height / 36.0f;
  FontManager &fm = FontManager::instance();
  alertFontFace = fm.getFaceID(BZDB.get("sansSerifFont"));
  alertFontSize = floorf(s);
  alertFontHeight = fm.getStrHeight(alertFontFace, alertFontSize, " ");

  for (int i = 0; i < MaxAlerts; i++)
    if (alertClock[i].isOn())
      alertLabelWidth[i] = fm.getStrLength(alertFontFace, alertFontSize, alertLabel[i]);
}

void			HUDRenderer::setMajorFontSize(int, int height)
{
  const float s = (float)height / 36.0f;
  FontManager &fm = FontManager::instance();
  majorFontFace = fm.getFaceID(BZDB.get("serifFont"));
  majorFontSize = floorf(s);
  majorFontHeight = fm.getStrHeight(majorFontFace, majorFontSize, " ");
}

void			HUDRenderer::setMinorFontSize(int, int height)
{
  FontManager &fm = FontManager::instance();
  minorFontFace = fm.getFaceID(BZDB.get("consoleFont"));

  switch (static_cast<int>(BZDB.eval("scorefontsize"))) {
  case 0: { // auto
    const float s = (float)height / 72.0f;
    minorFontSize = floorf(s);
    break;
  }
  case 1: // tiny
    minorFontSize = 6;
    break;
  case 2: // small
    minorFontSize = 8;
    break;
  case 3: // medium
    minorFontSize = 12;
    break;
  case 4: // big
    minorFontSize = 16;
    break;
  }
}

void			HUDRenderer::setHeadingFontSize(int, int height)
{
  const float s = (float)height / 144.0f;
  FontManager &fm = FontManager::instance();
  headingFontFace = fm.getFaceID(BZDB.get("sansSerifFont"));
  headingFontSize = floorf(s);

  // compute heading labels and (half) widths
  int i;
  for (i = 0; i < 36; i++)
    headingLabelWidth[i] = 0.5f * fm.getStrLength(headingFontFace, headingFontSize, headingLabel[i]);

  // compute maximum width over all altitude labels
  altitudeLabelMaxWidth = fm.getStrLength(headingFontFace, headingFontSize, "9999");
}

void			HUDRenderer::setComposeFontSize(int, int height)
{
  const float s = (float)height / 72.0f;
  FontManager &fm = FontManager::instance();
  composeFontFace = fm.getFaceID(BZDB.get("consoleFont"));
  composeTypeIn->setFontFace(composeFontFace);
  composeTypeIn->setFontSize(floorf(s));
}

void			HUDRenderer::setLabelsFontSize(int, int height)
{
  const float s = (float)height / 96.0f;
  FontManager &fm = FontManager::instance();
  labelsFontFace = fm.getFaceID(BZDB.get("consoleFont"));
  labelsFontSize = floorf(s);
}

void			HUDRenderer::setColor(float r, float g, float b)
{
  hudColor[0] = r;
  hudColor[1] = g;
  hudColor[2] = b;
}

void			HUDRenderer::setPlaying(bool _playing)
{
  playing = _playing;
}

void			HUDRenderer::setRoaming(bool _roaming)
{
  roaming = _roaming;
}

void			HUDRenderer::setDim(bool _dim)
{
  dim = _dim;
  scoreboard->setDim (_dim);
}

void			HUDRenderer::setPlayerHasHighScore(bool hasHigh)
{
  playerHasHighScore = hasHigh;
}

void			HUDRenderer::setTeamHasHighScore(bool hasHigh)
{
  teamHasHighScore = hasHigh;
}

void			HUDRenderer::setHeading(float angle)
{
  heading = (float)(90.0 - 180.0 * angle / M_PI);
  while (heading < 0.0f) heading += 360.0f;
  while (heading >= 360.0f) heading -= 360.0f;
}

void			HUDRenderer::setAltitude(float _altitude)
{
  altitude = _altitude;
}

void			HUDRenderer::setAltitudeTape(bool on)
{
  altitudeTape = on;
}

void			HUDRenderer::setFPS(float _fps)
{
  fps = _fps;
}

void			HUDRenderer::setDrawTime(float drawTimeInseconds)
{
  drawTime = drawTimeInseconds;
}

void			HUDRenderer::setFrameTriangleCount(int tpf)
{
  triangleCount = tpf;
}

void			HUDRenderer::setFrameRadarTriangleCount(int rtpf)
{
  radarTriangleCount = rtpf;
}

void			HUDRenderer::setAlert(int index, const char* string,
						float duration, bool warning)
{
  if (index < 0) index = 0;
  else if (index >= MaxAlerts) index = MaxAlerts - 1;
  if (!string) {
    alertClock[index].setClock(0.0f);
  } else {
    FontManager &fm = FontManager::instance();
    alertLabel[index] = BundleMgr::getCurrentBundle()->getLocalString(string);
    alertLabelWidth[index] = fm.getStrLength(alertFontFace, alertFontSize, alertLabel[index]);
    alertColor[index] = warning ? warningColor : messageColor;
    alertClock[index].setClock(duration);
  }
}

bool			HUDRenderer::getComposing() const
{
  return showCompose;
}

std::string		HUDRenderer::getComposeString() const
{
  return composeTypeIn->getString();
}

// Sets the string and allows editing by default
void			HUDRenderer::setComposeString(const std::string &message) const
{
  composeTypeIn->setEditing(true);
  composeTypeIn->setString(message);
}

// Sets the string and allows you to edit if _allowEdit is true
void			HUDRenderer::setComposeString(const std::string &message,
						bool _allowEdit) const
{
  composeTypeIn->setEditing(_allowEdit);
  composeTypeIn->setString(message);
}


// Set the prompt and allow editing by default
void			HUDRenderer::setComposing(const std::string &prompt) {
  this->setComposing(prompt, true);
}


// Set the prompt and allow editing or not depending on _allowEdit
void			HUDRenderer::setComposing(const std::string &prompt,
						bool _allowEdit)
{
  showCompose = (prompt.length() != 0);
  if (showCompose) {
    composeTypeIn->setEditing(_allowEdit);
    composeTypeIn->setLabel(prompt);
    composeTypeIn->setString("");
    composeTypeIn->setFocus();

    int cFontFace = composeTypeIn->getFontFace();
    float cFontSize = composeTypeIn->getFontSize();
    if (cFontFace >= 0) {
      FontManager &fm = FontManager::instance();
      const float x = fm.getStrLength(cFontFace, cFontSize, composeTypeIn->getLabel()) +
		      fm.getStrLength(cFontFace, cFontSize, "99");
      const float y = 1.0f;
      composeTypeIn->setLabelWidth(x);
      composeTypeIn->setPosition(x, y);
      // FIXME what is this supposed to do?
      composeTypeIn->setSize(window.getWidth() - x, fm.getStrHeight(cFontFace, cFontSize, " ") * 0);
    }
  } else {
    HUDui::setFocus(NULL);
  }
}

void			HUDRenderer::setFlagHelp(FlagType* desc, float duration)
{
  flagHelpClock.setClock(duration);

  // Generate the formatted help for this flag
  flagHelpText = makeHelpString(desc->flagHelp);

  // count the number of lines in the help message
  flagHelpLines = 0;
  const int helpLength = flagHelpText.size();
  const char* helpMsg = flagHelpText.c_str();
  for (int i = 0; i < helpLength; i++)
    if (helpMsg[i] == '\0')
      flagHelpLines++;
}

void			HUDRenderer::initCracks()
{
  for (int i = 0; i < HUDNumCracks; i++) {
    const float d = 0.90f * float(maxMotionSize) * ((float)bzfrand() + 0.90f);
    const float a = (float)(2.0 * M_PI * (double(i) + bzfrand()) / double(HUDNumCracks));
    cracks[i][0][0] = 0.0f;
    cracks[i][0][1] = 0.0f;
    cracks[i][1][0] = d * cosf(a);
    cracks[i][1][1] = d * sinf(a);
    makeCrack(cracks, i, 1, a);
  }
}

void			HUDRenderer::setCracks(bool _showCracks)
{
  if ((showCracks != _showCracks) && _showCracks) {
    initCracks();
    crackStartTime = TimeKeeper::getCurrent();
  }
  showCracks = _showCracks;
}

void			HUDRenderer::addMarker(float _heading, const float *_color )
{
  markers.resize(markers.size() + 1);
  HUDMarker &m = markers[markers.size() - 1];

  _heading = (float)(90.0 - 180.0 * _heading / M_PI);
  while (_heading < 0.0f) _heading += 360.0f;
  while (_heading >= 360.0f) _heading -= 360.0f;
  m.heading = _heading;
  memcpy(m.color, _color, sizeof(m.color));
}

void			HUDRenderer::setRestartKeyLabel(const std::string& label)
{
  char buffer[250];
  sprintf(buffer, BundleMgr::getCurrentBundle()->getLocalString(restartLabelFormat).c_str(), label.c_str());
  restartLabel = buffer;
  FontManager &fm = FontManager::instance();
  restartLabelWidth = fm.getStrLength(bigFontFace, bigFontSize, restartLabel);
}

void			HUDRenderer::setTimeLeft(uint32_t _timeLeft)
{
  timeLeft = _timeLeft;
  timeSet = TimeKeeper::getTick();
}

/* FIXME - makeHelpString should return an array of strings instead of
 * using implicit null chars.
 */
std::string		HUDRenderer::makeHelpString(const char* help) const
{
  if (!help) return std::string();

  FontManager &fm = FontManager::instance();
  static const float spaceWidth = fm.getStrLength(minorFontFace, minorFontSize, " ");

  // find sections of string not more than maxWidth pixels wide
  // and put them into a std::string separated by \0's.
  const float maxWidth = (float)window.getWidth() * 0.75f;
  std::string msg;
  std::string text = BundleMgr::getCurrentBundle()->getLocalString(help);

  char c;
  float wordWidth;
  std::string word = "";
  float currentLineWidth = 0.0f;
  unsigned int position = 0;
  while (position < text.size()) {
    c = text[position];
    // when we hit a space, append the previous word
    if (c == ' ') {
      if (word.size() == 0) {
	position++;
	continue;
      }

      wordWidth = fm.getStrLength(minorFontFace, minorFontSize, word);
      msg += c;
      if (wordWidth + currentLineWidth + spaceWidth < maxWidth) {
	currentLineWidth += wordWidth;
      } else {
	msg += '\0';
	currentLineWidth = 0.0f;
      }
      msg.append(word);
      word.resize(0);

    } else {
      word += c;
    }
    position++;
  }

  if (word.size() > 0) {
    wordWidth = fm.getStrLength(minorFontFace, minorFontSize, word);
    if (wordWidth + currentLineWidth + spaceWidth >= maxWidth) {
      msg += '\0';
    }
    msg += ' ';
    msg.append(word);
  }

  // append terminating null so line counts are correct
  msg += '\0';

  return msg;
}

void			HUDRenderer::makeCrack(float crackpattern[HUDNumCracks][(1 << HUDCrackLevels) + 1][2], int n, int l, float a)
{
  if (l >= (1 << (HUDCrackLevels - 1))) return;
  float d = 0.5f * float(maxMotionSize) *
		((float)bzfrand() + 0.5f) * powf(0.5f, 0.69f * logf(float(l)));
  float newAngle = (float)(a + M_PI * bzfrand() / double(HUDNumCracks));
  crackpattern[n][2*l][0] = crackpattern[n][l][0] + d * cosf(newAngle);
  crackpattern[n][2*l][1] = crackpattern[n][l][1] + d * sinf(newAngle);
  makeCrack(crackpattern, n, 2*l, newAngle);
  d = 0.5f * float(maxMotionSize) *
		((float)bzfrand() + 0.5f) * powf(0.5f, 0.69f * logf(float(l)));
  newAngle = (float)(a - M_PI * bzfrand() / double(HUDNumCracks));
  crackpattern[n][2*l+1][0] = crackpattern[n][l][0] + d * cosf(newAngle);
  crackpattern[n][2*l+1][1] = crackpattern[n][l][1] + d * sinf(newAngle);
  makeCrack(crackpattern, n, 2*l+1, newAngle);
}

static const float dimFactor = 0.2f;

void			HUDRenderer::hudColor3f(GLfloat r, GLfloat g, GLfloat b)
{
  if (dim)
    glColor3f(dimFactor * r, dimFactor * g, dimFactor * b);
  else
    glColor3f(r, g, b);
}

void			HUDRenderer::hudColor4f(
				GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  if (dim)
    glColor4f(dimFactor * r, dimFactor * g, dimFactor * b, a);
  else
    glColor4f(r, g, b, a);
}

void			HUDRenderer::hudColor3fv(const GLfloat* c)
{
  if (dim)
    glColor3f(dimFactor * c[0], dimFactor * c[1], dimFactor * c[2]);
  else
    glColor3fv(c);
}

void			HUDRenderer::hudSColor3fv(const GLfloat* c)
{
  if (dim)
    glColor3f(dimFactor * c[0], dimFactor * c[1], dimFactor * c[2]);
  else
    glColor3fv(c);
}

void			HUDRenderer::hudColor4fv(const GLfloat* c)
{
  if (dim)
    glColor4f(dimFactor * c[0], dimFactor * c[1], dimFactor * c[2], c[3]);
  else
    glColor4fv(c);
}

void			HUDRenderer::render(SceneRenderer& renderer)
{
  if (firstRender) {
    firstRender = false;
    resize(false);
  }

  OpenGLGState::resetState();
  if (!BZDB.isTrue("noGUI")) {
    if (playing) {
      renderPlaying(renderer);
    }
    else if (roaming) {
      renderRoaming(renderer);
    }
    else {
      renderNotPlaying(renderer);
    }
  }
  else {
    const bool showTimes = (fps > 0.0f) || (drawTime > 0.0f) ||
			   (triangleCount > 0) || (radarTriangleCount > 0);
    const bool showTankLabels = BZDB.isTrue("displayLabels");

    if (showCompose || showTimes || showTankLabels) {
      // get view metrics
      const int width = window.getWidth();
      const int height = window.getHeight();
      const int viewHeight = window.getViewHeight();
      const int ox = window.getOriginX();
      const int oy = window.getOriginY();
      // use one-to-one pixel projection
      glScissor(ox, oy + height - viewHeight, width, viewHeight);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0.0, width, viewHeight - height, viewHeight, -1.0, 1.0);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();

      if (showCompose) {
	renderCompose(renderer);
      }
      if (showTimes) {
	renderTimes();
      }
      if (showTankLabels) {
	renderTankLabels(renderer);
      }

      glPopMatrix();
    }
  }
}

void			HUDRenderer::renderAlerts(void)
{
  const float centerx = 0.5f * (float)window.getWidth();

  FontManager &fm = FontManager::instance();

  float y = (float)window.getViewHeight() +
	    -fm.getStrHeight(majorFontFace, majorFontSize, " ") +
	    -fm.getStrHeight(alertFontFace, alertFontSize, " ");

  for (int i = 0; i < MaxAlerts; i++) {
    if (alertClock[i].isOn()) {
      hudColor3fv(alertColor[i]);
      std::string newAlertLabel = (dim ? ColorStrings[DimColor] : "") + alertLabel[i];
      // FIXME: this assumes that there's not more than one reset in the string.
      if (dim) {
	newAlertLabel.insert(newAlertLabel.find(ColorStrings[ResetColor], 0) - 1
			     + ColorStrings[ResetColor].size(), ColorStrings[DimColor]);
      }
      fm.drawString(centerx - 0.5f * alertLabelWidth[i], y, 0,
		    alertFontFace, alertFontSize, newAlertLabel);
      y -= fm.getStrHeight(alertFontFace, alertFontSize, " ");
    }
  }
}

void			HUDRenderer::renderStatus(void)
{
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (!myTank || !World::getWorld()) return;

  Bundle *bdl = BundleMgr::getCurrentBundle();

  FontManager &fm = FontManager::instance();

  char buffer[80];
  const float h = fm.getStrHeight(majorFontFace, majorFontSize, " ");
  float x = 0.25f * h;
  float y = (float)window.getViewHeight() - h;
  TeamColor teamIndex = myTank->getTeam();
  FlagType* flag = myTank->getFlag();

  // print player name and score in upper left corner in team (radar) color
  if (!roaming && (!playerHasHighScore || scoreClock.isOn())) {
    sprintf(buffer, "%s: %d", myTank->getCallSign(), myTank->getScore());
    hudColor3fv(Team::getRadarColor(teamIndex, World::getWorld()->allowRabbit()));
    fm.drawString(x, y, 0, majorFontFace, majorFontSize, buffer);
  }

  // print flag if player has one in upper right
  if (flag != Flags::Null) {
    sprintf(buffer, "%s", BundleMgr::getCurrentBundle()->getLocalString(flag->flagName).c_str());
    x = (float)window.getWidth() - 0.25f * h - fm.getStrLength(majorFontFace, majorFontSize, buffer);
    if (flag->endurance == FlagSticky)
      hudColor3fv(warningColor);
    else
      hudColor3fv(messageColor);
    fm.drawString(x, y, 0, majorFontFace, majorFontSize, buffer);
  } else {
    time_t timeNow;
    struct tm userTime;
    time(&timeNow);
    userTime = *localtime(&timeNow);

    // switch date and time if necessary
    if (BZDB.eval("timedate") == 2) {
      if (time(NULL) - lastTimeChange >= 2) {
	dater = !dater;
	lastTimeChange = time(NULL);
      }
    } else {
      dater = (BZDB.eval("timedate") == 1);
    }

    // print time or date
    if (dater)
      sprintf(buffer, "%4d.%02d.%02d", 1900 + userTime.tm_year, userTime.tm_mon + 1, userTime.tm_mday);
    else
      sprintf(buffer, "%2d:%2.2d", userTime.tm_hour, userTime.tm_min);
    x = (float)window.getWidth() - 0.25f * h - fm.getStrLength(majorFontFace, majorFontSize, buffer);
    hudColor3fv(messageColor);
    fm.drawString(x, y, 0, majorFontFace, majorFontSize, buffer);
  }

  // print current position of tank
  if (BZDB.isTrue("showCoordinates")) {
    y -= float(1.5*h);
    sprintf(buffer, "[%d %d %d]", (int)myTank->getPosition()[0],
	    (int)myTank->getPosition()[1], (int)myTank->getPosition()[2]);
    x = (float)window.getWidth() - 0.25f * h - fm.getStrLength(majorFontFace, majorFontSize, buffer);
    fm.drawString(x, y, 0, majorFontFace, majorFontSize, buffer);
    y += float(1.5*h);
  }

  if (roaming && BZDB.isTrue("showVelocities"))
  {
	  Player *target = ROAM.getTargetTank();
	  if (target)
	  {
		  float vel[3] = {0};
		  memcpy(vel,target->getVelocity(),sizeof(float)*3);

		  float linSpeed = sqrt(vel[0]*vel[0]+vel[1]*vel[1]);
		  float vertSpeed = vel[2];
		  float rotSpeed = fabs(target->getAngularVelocity());

		  float drawY = y-1.5f*h;
		  // draw header
		  x = (float)window.getWidth() - 0.25f * h - fm.getStrLength(majorFontFace, majorFontSize, "Target Info");
		  fm.drawString(x, drawY, 0, majorFontFace, majorFontSize, "Target Info");

		  float smallZHeight = fm.getStrHeight(minorFontFace,minorFontSize,std::string("X"))*1.5f;
		  sprintf(buffer,"Linear Speed:%5.2f",linSpeed);
		  x = (float)window.getWidth() - 0.25f * h - fm.getStrLength(minorFontFace, minorFontSize,buffer);
		  fm.drawString(x,drawY-smallZHeight, 0, minorFontFace, minorFontSize, buffer);

		  sprintf(buffer,"Vertical Speed:%5.2f",vertSpeed);
		  x = (float)window.getWidth() - 0.25f * h - fm.getStrLength(minorFontFace, minorFontSize,buffer);
		  fm.drawString(x, drawY-smallZHeight*2.0f, 0, minorFontFace, minorFontSize, buffer);

		  sprintf(buffer,"Angular Speed:%5.2f",rotSpeed);
		  x = (float)window.getWidth() - 0.25f * h - fm.getStrLength(minorFontFace, minorFontSize,buffer);
		  fm.drawString(x,drawY-smallZHeight*3.0f, 0, minorFontFace, minorFontSize, buffer);
	  }
  }

  // print status top-center
  static const GLfloat redColor[3] = { 1.0f, 0.0f, 0.0f };
  static const GLfloat yellowColor[3] = { 1.0f, 1.0f, 0.0f };
  static const GLfloat greenColor[3] = { 0.0f, 1.0f, 0.0f };
  const GLfloat* statusColor = warningColor;
  // TODO: the upper 4 values of timeLeft (~0u-3 to ~0u)
  // are reserved for future use as timer flags (e.g. paused)
  if ((timeLeft == 0) || (timeLeft >= (~0u - 3))) {
    strcpy(buffer, "");
  } else {
    int t = timeLeft - (int)(TimeKeeper::getTick() - timeSet);
    if (t < 0) t = 0;
    if (t >= 3600)
      sprintf(buffer, "%d:%02d:%02d   ", t / 3600, (t / 60) % 60, t % 60);
    else if (t >= 60)
      sprintf(buffer, "%d:%02d   ", t / 60, t % 60);
    else
      sprintf(buffer, "0:%02d   ", t);
  }
  if (!roaming) {
    switch (myTank->getFiringStatus()) {
      case LocalPlayer::Deceased:
	strcat(buffer, bdl->getLocalString("Dead").c_str());
	break;

      case LocalPlayer::Ready:
	if (flag != Flags::Null && flag->endurance == FlagSticky &&
	    World::getWorld()->allowShakeTimeout()) {
	  /* have a bad flag -- show time left 'til we shake it */
	  statusColor = yellowColor;
	  sprintf(buffer, bdl->getLocalString("%.1f").c_str(), myTank->getFlagShakingTime());
	} else {
	  statusColor = greenColor;
	  strcat(buffer, bdl->getLocalString("Ready").c_str());
	}
	break;

      case LocalPlayer::Loading:

    if (World::getWorld()->getMaxShots() != 0) {
	  statusColor = redColor;
	  sprintf(buffer, bdl->getLocalString("Reloaded in %.1f").c_str(), myTank->getReloadTime());
	}
	break;

      case LocalPlayer::Sealed:
	strcat(buffer, bdl->getLocalString("Sealed").c_str());
	break;

      case LocalPlayer::Zoned:
	strcat(buffer, bdl->getLocalString("Zoned").c_str());
	break;
    }
  }

  if (roaming) {
    statusColor = messageColor;
    if (dim) strcat(buffer, ColorStrings[DimColor].c_str());
    strcat(buffer, ROAM.getRoamingLabel().c_str());
  }

  x = 0.5f * ((float)window.getWidth() - fm.getStrLength(majorFontFace, majorFontSize, buffer));
  hudColor3fv(statusColor);
  fm.drawString(x, y, 0, majorFontFace, majorFontSize, buffer);
}

int HUDRenderer::tankScoreCompare(const void* _a, const void* _b)
{
  RemotePlayer* a = World::getWorld()->getPlayer(*(int*)_a);
  RemotePlayer* b = World::getWorld()->getPlayer(*(int*)_b);
  if (World::getWorld()->allowRabbit())
    return b->getRabbitScore() - a->getRabbitScore();
  else
    return b->getScore() - a->getScore();
}

int HUDRenderer::teamScoreCompare(const void* _c, const void* _d)
{
  Team* c = World::getWorld()->getTeams()+*(int*)_c;
  Team* d = World::getWorld()->getTeams()+*(int*)_d;

  return (d->won-d->lost) - (c->won-c->lost);
}


void			HUDRenderer::renderTankLabels(SceneRenderer& renderer)
{
  if (!World::getWorld()) return;

  int offset = window.getViewHeight() - window.getHeight();

  GLint view[] = {window.getOriginX(), window.getOriginY(),
		 window.getWidth(), window.getHeight()};
  const GLfloat *projf = renderer.getViewFrustum().getProjectionMatrix();
  const GLfloat *modelf = renderer.getViewFrustum().getViewMatrix();

  // convert to doubles
  GLdouble proj[16], model[16];
  for (int j = 0; j < 16; j++) {
    proj[j] = projf[j];
    model[j] = modelf[j];
  }

  for (int i = 0; i < curMaxPlayers; i++) {
    RemotePlayer *pl = World::getWorld()->getPlayer(i);
    if (pl && pl->isAlive()) {
      const char *name = pl->getCallSign();
      double x, y, z;
      hudSColor3fv(Team::getRadarColor(pl->getTeam(), World::getWorld()->allowRabbit()));
      gluProject(pl->getPosition()[0], pl->getPosition()[1],
		 pl->getPosition()[2], model, proj, view, &x, &y, &z);
      if (z >= 0.0 && z <= 1.0) {
	FontManager &fm = FontManager::instance();
	fm.drawString(float(x) - fm.getStrLength(labelsFontFace, labelsFontSize, name) / 2.0f,
		      float(y) + offset - fm.getStrHeight(labelsFontFace, labelsFontSize, name),
		      0, labelsFontFace, labelsFontSize, name);
	FlagType* flag = pl->getFlag();
	if (flag != Flags::Null) {
	  std::string flagStr = "(";
	  flagStr += flag->endurance == FlagNormal ? flag->flagName : flag->flagAbbv;
	  flagStr += ")";
	  const char *fname = flagStr.c_str();
	  fm.drawString(float(x) - fm.getStrLength(labelsFontFace, labelsFontSize, fname) / 2.0f,
			float(y) + offset -
			(2.0f * fm.getStrHeight(labelsFontFace, labelsFontSize, fname)),
			0, labelsFontFace, labelsFontSize, fname);
	}
      }
    }
  }
}

void			HUDRenderer::renderCracks()
{
  double delta = (TimeKeeper::getCurrent() - crackStartTime) * 5.0;
  if (delta > 1.0)
    delta = 1.0;
  int maxLevels = (int) (HUDCrackLevels * delta);

  glPushMatrix();
  glTranslatef(GLfloat(window.getWidth() >> 1),
	       GLfloat(window.getViewHeight() >> 1), 0.0f);
  glLineWidth(3.0);
  hudColor3f(1.0f, 1.0f, 1.0f);
  glBegin(GL_LINES);
    for (int i = 0; i < HUDNumCracks; i++) {
      glVertex2fv(cracks[i][0]);
      glVertex2fv(cracks[i][1]);
      for (int j = 0; j < maxLevels-1; j++) {
	const int num = 1 << j;
	for (int k = 0; k < num; k++) {
	  glVertex2fv(cracks[i][num + k]);
	  glVertex2fv(cracks[i][2 * (num + k)]);
	  glVertex2fv(cracks[i][num + k]);
	  glVertex2fv(cracks[i][2 * (num + k) + 1]);
	}
      }
    }
  glEnd();
  glLineWidth(1.0);
  glPopMatrix();
}

void			HUDRenderer::renderCompose(SceneRenderer&)
{
  composeTypeIn->render();
  OpenGLGState::resetState();
}

void			HUDRenderer::renderTimes(void)
{
  const int centerx = window.getWidth() >> 1;
  const int centery = window.getViewHeight() >> 1;
  FontManager &fm = FontManager::instance();

  // draw frames per second
  if (fps > 0.0f) {
    char buf[20];
    sprintf(buf, "FPS: %d", int(fps));
    hudColor3f(1.0f, 1.0f, 1.0f);
    fm.drawString((float)(centerx - maxMotionSize), (float)centery + (float)maxMotionSize +
		3.0f * fm.getStrHeight(headingFontFace, headingFontSize, "0"), 0,
		headingFontFace, headingFontSize, buf);
  }
  float triCountYOffset = 4.5f;
  if (radarTriangleCount > 0) {
    char buf[20];
    sprintf(buf, "rtris: %i", radarTriangleCount);
    hudColor3f(1.0f, 1.0f, 1.0f);
    fm.drawString((float)(centerx - maxMotionSize), (float)centery + (float)maxMotionSize +
		triCountYOffset * fm.getStrHeight(headingFontFace, headingFontSize, "0"), 0,
		headingFontFace, headingFontSize, buf);
    triCountYOffset += 1.5f;
  }
  if (triangleCount > 0) {
    char buf[20];
    sprintf(buf, "tris: %i", triangleCount);
    hudColor3f(1.0f, 1.0f, 1.0f);
    fm.drawString((float)(centerx - maxMotionSize), (float)centery + (float)maxMotionSize +
		triCountYOffset * fm.getStrHeight(headingFontFace, headingFontSize, "0"), 0,
		headingFontFace, headingFontSize, buf);
  }
  if (drawTime > 0.0f) {
    char buf[20];
    sprintf(buf, "time: %dms", (int)(drawTime * 1000.0f));
    hudColor3f(1.0f, 1.0f, 1.0f);
    fm.drawString((float)(centerx + maxMotionSize) - fm.getStrLength(headingFontFace, headingFontSize, buf),
		  (float)centery + (float)maxMotionSize +
		  3.0f * fm.getStrHeight(headingFontFace, headingFontSize, "0"), 0, headingFontFace, headingFontSize, buf);
  }
}

void			HUDRenderer::renderBox(SceneRenderer&)
{
  // get view metrics
  const int width = window.getWidth();
  const int height = window.getHeight();
  const int viewHeight = window.getViewHeight();
  const int ox = window.getOriginX();
  const int oy = window.getOriginY();
  const int centerx = width >> 1;
  const int centery = viewHeight >> 1;
  int i;
  float x, y;

  FontManager &fm = FontManager::instance();

  OpenGLGState::resetState();
  const bool smooth = BZDBCache::smooth;

  // draw targeting box
  hudColor3fv(hudColor);
  glBegin(GL_LINE_LOOP); {
    glVertex2i(centerx - noMotionSize, centery - noMotionSize);
    glVertex2i(centerx + noMotionSize, centery - noMotionSize);
    glVertex2i(centerx + noMotionSize, centery + noMotionSize);
    glVertex2i(centerx - noMotionSize, centery + noMotionSize);
  } glEnd();
  glBegin(GL_POINTS); {
    glVertex2i(centerx - noMotionSize, centery - noMotionSize);
    glVertex2i(centerx + noMotionSize, centery - noMotionSize);
    glVertex2i(centerx + noMotionSize, centery + noMotionSize);
    glVertex2i(centerx - noMotionSize, centery + noMotionSize);
  } glEnd();
  glBegin(GL_LINE_LOOP); {
    glVertex2i(centerx - maxMotionSize, centery - maxMotionSize);
    glVertex2i(centerx + maxMotionSize, centery - maxMotionSize);
    glVertex2i(centerx + maxMotionSize, centery + maxMotionSize);
    glVertex2i(centerx - maxMotionSize, centery + maxMotionSize);
  } glEnd();
  glBegin(GL_POINTS); {
    glVertex2i(centerx - maxMotionSize, centery - maxMotionSize);
    glVertex2i(centerx + maxMotionSize, centery - maxMotionSize);
    glVertex2i(centerx + maxMotionSize, centery + maxMotionSize);
    glVertex2i(centerx - maxMotionSize, centery + maxMotionSize);
  } glEnd();

  // draw heading strip
  if (true /* always draw heading strip */) {
    // first clip to area
    glScissor(ox + centerx - maxMotionSize, oy + height - viewHeight + centery + maxMotionSize - 5,
		2 * maxMotionSize, 25 + (int)(headingFontSize + 0.5f));

    // draw heading mark
    glBegin(GL_LINES);
      glVertex2i(centerx, centery + maxMotionSize);
      glVertex2i(centerx, centery + maxMotionSize - 5);
    glEnd();

    // figure out which marker is closest to center
    int baseMark = int(heading) / 10;
    // get minimum and maximum visible marks (leave some leeway)
    int minMark = baseMark - int(headingOffset / 10.0f) - 1;
    int maxMark = baseMark + int(headingOffset / 10.0f) + 1;

    // draw tick marks
    glPushMatrix();
    if (smooth) {
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_LINE_SMOOTH);
      glEnable(GL_BLEND);
    }
    GLfloat basex = maxMotionSize * (heading - 10.0f * float(minMark)) /
								headingOffset;
    if (!smooth) basex = floorf(basex);
    glTranslatef((float)centerx - basex, (float)(centery + maxMotionSize), 0.0f);
    x = smooth ? 0.0f : -0.5f;
    glBegin(GL_LINES);
    for (i = minMark; i <= maxMark; i++) {
      glVertex2i((int)x, 0);
      glVertex2i((int)x, 8);
      x += headingMarkSpacing;
      glVertex2i((int)x, 0);
      glVertex2i((int)x, 4);
      x += headingMarkSpacing;
    }
    glEnd();

    // back to our regular rendering mode
    if (smooth) {
      glDisable(GL_LINE_SMOOTH);
      glDisable(GL_BLEND);
    }
    glPopMatrix();

    bool smoothLabel = smooth;
    x = (float)centerx - basex;
    y = 7.0f + (float)(centery + maxMotionSize);
    if (smoothLabel) {
      x -= 0.5f;
      hudColor4f(hudColor[0], hudColor[1], hudColor[2], basex - floorf(basex));
    }
    for (i = minMark; i <= maxMark; i++) {
      fm.drawString(x - headingLabelWidth[(i + 36) % 36], y, 0, headingFontFace,
		    headingFontSize, headingLabel[(i + 36) % 36]);
      x += 2.0f * headingMarkSpacing;
    }
    if (smoothLabel) {
      x = (float)centerx - basex + 0.5f;
      basex -= floorf(basex);
      hudColor4f(hudColor[0], hudColor[1], hudColor[2], 1.0f - basex);
      for (i = minMark; i <= maxMark; i++) {
      fm.drawString(x - headingLabelWidth[(i + 36) % 36], y, 0, headingFontFace,
		    headingFontSize, headingLabel[(i + 36) % 36]);
	x += 2.0f * headingMarkSpacing;
      }
    }
    OpenGLGState::resetState();

    // draw markers (give 'em a little more space on the sides)
    glScissor(ox + centerx - maxMotionSize - 8, oy + height - viewHeight + centery + maxMotionSize,
		2 * maxMotionSize + 16, 10);
    glPushMatrix();
    glTranslatef((float)centerx, (float)(centery + maxMotionSize), 0.0f);
    for (MarkerList::const_iterator it = markers.begin(); it != markers.end(); ++it) {
      const HUDMarker &m = *it;
      const float relAngle = fmodf(360.0f + m.heading - heading, 360.0f);
      hudColor3fv(m.color);
      if (relAngle <= headingOffset || relAngle >= 360.0f - headingOffset) {
	// on the visible part of tape
	GLfloat mx = maxMotionSize / headingOffset *
			((relAngle < 180.0f) ? relAngle : relAngle - 360.0f);
	glBegin(GL_QUADS);
	  glVertex2f(mx, 0.0f);
	  glVertex2f(mx + 4.0f, 4.0f);
	  glVertex2f(mx, 8.0f);
	  glVertex2f(mx - 4.0f, 4.0f);
	glEnd();
      } else if (relAngle <= 180.0) {
	// off to the right
	glBegin(GL_TRIANGLES);
	  glVertex2f((float)maxMotionSize, 0.0f);
	  glVertex2f((float)maxMotionSize + 4.0f, 4.0f);
	  glVertex2f((float)maxMotionSize, 8.0f);
	glEnd();
      } else {
	// off to the left
	glBegin(GL_TRIANGLES);
	  glVertex2f(-(float)maxMotionSize, 0.0f);
	  glVertex2f(-(float)maxMotionSize, 8.0f);
	  glVertex2f(-(float)maxMotionSize - 4.0f, 4.0f);
	glEnd();
      }
    }
    markers.clear();
    glPopMatrix();
  }

  // draw altitude strip
  if (altitudeTape) {
    // clip to area
    glScissor(ox + centerx + maxMotionSize - 5, oy + height - viewHeight + centery - maxMotionSize,
		(int)altitudeLabelMaxWidth + 15, 2 * maxMotionSize);

    // draw altitude mark
    hudColor3fv(hudColor);
    glBegin(GL_LINES);
      glVertex2i(centerx + maxMotionSize, centery);
      glVertex2i(centerx + maxMotionSize - 5, centery);
    glEnd();

    // figure out which marker is closest to center
    int baseMark = int(altitude) / 5;

    // get minimum and maximum visible marks (leave some leeway)
    int minMark = baseMark - int(altitudeOffset / 5.0f) - 1;
    if (minMark < 0) minMark = 0;

    int maxMark = baseMark + int(altitudeOffset / 5.0f) + 1;

    // draw tick marks
    glPushMatrix();
    if (smooth) {
      glEnable(GL_LINE_SMOOTH);
      glEnable(GL_BLEND);
    }
    // NOTE: before I (Steve Krenzel) made changes, minMark was always 0, which appears
    // to have made basey always equal 0, maybe I overlooked something
    GLfloat basey = maxMotionSize * (altitude - 5.0f * float(minMark)) /
								altitudeOffset;
    if (!smooth) basey = floorf(basey);
    glTranslatef((float)(centerx + maxMotionSize),
				(float)centery - basey, 0.0f);
    y = smooth ? 0.0f : -0.5f;
    glBegin(GL_LINES);
    for (i = minMark; i <= maxMark; i++) {
      glVertex2i(0, (int)y);
      glVertex2i(8, (int)y);
      y += altitudeMarkSpacing;
    }
    glEnd();

    // back to our regular rendering mode
    if (smooth) {
      glDisable(GL_LINE_SMOOTH);
      glDisable(GL_BLEND);
    }
    glPopMatrix();

    bool smoothLabel = smooth;
    x = (float)(10 + centerx + maxMotionSize);
    y = (float)centery - basey + floorf(fm.getStrHeight(headingFontFace, headingFontSize, "0") / 2);
    if (smoothLabel) {
      y -= 0.5f;
      hudColor4f(hudColor[0], hudColor[1], hudColor[2], basey - floorf(basey));
    }
    char buf[10];
    for (i = minMark; i <= maxMark; i++) {
      sprintf(buf, "%d", i * 5);
      fm.drawString(x, y, 0, headingFontFace, headingFontSize, std::string(buf));
      y += altitudeMarkSpacing;
    }
    if (smoothLabel) {
      y = (float)centery - basey + floorf(fm.getStrHeight(headingFontFace, headingFontSize, "0") / 2);
      y += 0.5f;
      basey -= floorf(basey);
      hudColor4f(hudColor[0], hudColor[1], hudColor[2], 1.0f - basey);
      for (i = minMark; i <= maxMark; i++) {
	sprintf(buf, "%d", i * 5);
	fm.drawString(x, y, 0, headingFontFace, headingFontSize, std::string(buf));
	y += altitudeMarkSpacing;
      }
    }
  }
}

void			HUDRenderer::renderPlaying(SceneRenderer& renderer)
{
  // get view metrics
  const int width = window.getWidth();
  const int height = window.getHeight();
  const int viewHeight = window.getViewHeight();
  const int ox = window.getOriginX();
  const int oy = window.getOriginY();
  const int centerx = width >> 1;
  int i;
  float y;

  FontManager &fm = FontManager::instance();

  // use one-to-one pixel projection
  glScissor(ox, oy + height - viewHeight, width, viewHeight);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, width, viewHeight - height, viewHeight, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // cover the lower portion of the screen when burrowed
  const LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank && myTank->getPosition()[2] < 0.0f) {
    glColor4f(0.02f, 0.01f, 0.01f, 1.0);
    glRectf(0, 0, (float)width, (myTank->getPosition()[2]/(BZDB.eval(StateDatabase::BZDB_BURROWDEPTH)-0.1f)) * ((float)viewHeight/2.0f));
  }

  // draw shot reload status
  if (BZDB.isTrue("displayReloadTimer")) {
    renderShots(myTank);
  }

  // draw cracks
  if (showCracks)
    renderCracks();

  // draw status line
  renderStatus();

  // draw alert messages
  renderAlerts();

  // show player scoreboard
  scoreboard->render(false);

  // draw flag help
  if (flagHelpClock.isOn()) {
    hudColor3fv(messageColor);
    flagHelpY = (float) ((window.getViewHeight() >> 1) - maxMotionSize);
    y = flagHelpY;
    const char* flagHelpBase = flagHelpText.c_str();
    for (i = 0; i < flagHelpLines; i++) {
      y -= fm.getStrHeight(minorFontFace, minorFontSize, "0");
      fm.drawString((float)(centerx - fm.getStrLength(minorFontFace, minorFontSize, flagHelpBase)/2.0),
		    y, 0, minorFontFace, minorFontSize, flagHelpBase);
      while (*flagHelpBase) flagHelpBase++;
      flagHelpBase++;
    }

  }

  if (myTank && globalClock.isOn()) {
    float yy = 0.5f * (float)height
      + fm.getStrHeight(bigFontFace, bigFontSize, "0");
    if (myTank->isAutoPilot()) {
      hudColor3fv(messageColor);
      fm.drawString(0.5f * ((float)width - autoPilotWidth), yy, 0, bigFontFace,
		    bigFontSize, autoPilotLabel);
    }
  }

  // draw times
  renderTimes();

  // draw message composition
  if (showCompose)
    renderCompose(renderer);

  // draw targeting box
  renderBox(renderer);

  // restore graphics state
  glPopMatrix();
}

void			HUDRenderer::renderNotPlaying(SceneRenderer& renderer)
{
  // get view metrics
  const int width = window.getWidth();
  const int height = window.getHeight();
  const int viewHeight = window.getViewHeight();
  const int ox = window.getOriginX();
  const int oy = window.getOriginY();

  FontManager &fm = FontManager::instance();

  // use one-to-one pixel projection
  glScissor(ox, oy + height - viewHeight, width, viewHeight);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, width, viewHeight - height, viewHeight, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // draw cracks
  if (showCracks)
    renderCracks();

  // draw status line
  renderStatus();

  // draw alert messages
  renderAlerts();

  // show player scoreboard
  scoreboard->render(true);

  // draw times
  renderTimes();

  // draw message composition
  if (showCompose)
    renderCompose(renderer);

  // tell player what to do to start/resume playing
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank && globalClock.isOn()) {
    float y = 0.5f * (float)viewHeight + fm.getStrHeight(bigFontFace, bigFontSize, "0");
    if (gameOver) {
      hudColor3fv(messageColor);
      fm.drawString(0.5f * ((float)width - gameOverLabelWidth), y, 0,
		    bigFontFace, bigFontSize, gameOverLabel);
    } else if (!myTank->isAlive() && !myTank->isExploding()) {
      hudColor3fv(messageColor);
      fm.drawString(0.5f * ((float)width - restartLabelWidth), y, 0,
		    bigFontFace, bigFontSize, restartLabel);
    } else if (myTank->isPaused()) {
      hudColor3fv(messageColor);
      fm.drawString(0.5f * ((float)width - resumeLabelWidth), y, 0,
		    bigFontFace, bigFontSize, resumeLabel);
    } else if (myTank->isAutoPilot()) {
      hudColor3fv(messageColor);
      fm.drawString(0.5f * ((float)width - autoPilotWidth), y, 0,
		    bigFontFace, bigFontSize, autoPilotLabel);
    }
  }

  // restore graphics state
  glPopMatrix();
}

void			HUDRenderer::renderRoaming(SceneRenderer& renderer)
{
  // get view metrics
  const int width = window.getWidth();
  const int height = window.getHeight();
  const int viewHeight = window.getViewHeight();
  const int ox = window.getOriginX();
  const int oy = window.getOriginY();

  FontManager &fm = FontManager::instance();

  // use one-to-one pixel projection
  glScissor(ox, oy + height - viewHeight, width, viewHeight);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, width, viewHeight - height, viewHeight, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // black out the underground if we're driving with a tank with BU
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank && myTank->getPosition()[2] < 0.0f) {
    glColor4f(0.02f, 0.01f, 0.01f, 1.0);
    glRectf(0, 0, (float)width, (myTank->getPosition()[2]/(BZDB.eval(StateDatabase::BZDB_BURROWDEPTH)-0.1f)) * ((float)viewHeight/2.0f));
  }

  // draw shot reload status
  if ((ROAM.getMode() == Roaming::roamViewFP) && BZDB.isTrue("displayReloadTimer")) {
    renderShots(ROAM.getTargetTank());
  }

  // draw alert messages
  renderAlerts();

  // show player scoreboard
  scoreboard->render(false);

  // show tank labels
  if (BZDB.isTrue("displayLabels")) renderTankLabels(renderer);

  // draw times
  renderTimes();

  // draw message composition
  if (showCompose)
    renderCompose(renderer);

  // display game over
  if (myTank && globalClock.isOn()) {
    float y = 0.5f * (float)height + fm.getStrHeight(bigFontFace, bigFontSize, "0");
    if (gameOver) {
      hudColor3fv(messageColor);
      fm.drawString(0.5f * ((float)width - gameOverLabelWidth), y, 0,
		    bigFontFace, bigFontSize, gameOverLabel);
    }
  }

  renderStatus();

  // draw targeting box
  if (altitude != -1.0f)
    renderBox(renderer);

  // restore graphics state
  glPopMatrix();
}



static int compare_float (const void* a, const void* b)
{
  const float fa = *((const float*)a);
  const float fb = *((const float*)b);
  if (fa > fb) {
    return 1;
  } else {
    return -1;
  }
}

void			HUDRenderer::renderShots(const Player* target)
{
  // get the target tank
  if (!target) return;

  // get view metrics
  const int width = window.getWidth();
  const int height = window.getHeight();
  const int viewHeight = window.getViewHeight();
  const int centerx = width >> 1;
  const int centery = viewHeight >> 1;

  const int indicatorWidth = width / 50;
  const int indicatorHeight = height / 80;
  const int indicatorSpace = indicatorHeight / 10 + 2;
  const int indicatorLeft = centerx + maxMotionSize + indicatorWidth + 16;
  const int indicatorTop = centery - (int)(0.5f * (indicatorHeight + indicatorSpace) * target->getMaxShots());

  const int maxShots = target->getMaxShots();

  float* factors = new float[maxShots];

  // tally the reload values
  for (int i = 0; i < maxShots; ++i) {
    const ShotPath* shot = target->getShot(i);
    factors[i] = 1.0f;
    if (shot) {
      const TimeKeeper currentTime = shot->getCurrentTime();
      const TimeKeeper startTime = shot->getStartTime();
      const float reloadTime = shot->getReloadTime();
      factors[i] = float(1 - ((reloadTime - (currentTime - startTime)) / reloadTime));
      if (factors[i] > 1.0f) factors[i] = 1.0f;
    }
  }

  // sort the reload values
  qsort(factors, maxShots, sizeof(float), compare_float);

  // draw the reload values
  glEnable(GL_BLEND);
  for (int i = 0; i < maxShots; ++i) {
    const int myWidth = int(indicatorWidth * factors[i]);
    const int myTop = indicatorTop + i * (indicatorHeight + indicatorSpace);
    if (factors[i] < 1.0f) {
      hudColor4f(0.0f, 1.0f, 0.0f, 0.5f); // green
      glRecti(indicatorLeft, myTop, indicatorLeft + myWidth, myTop + indicatorHeight);
      hudColor4f(1.0f, 0.0f, 0.0f, 0.5f); // red
      glRecti(indicatorLeft + myWidth + 1, myTop, indicatorLeft + indicatorWidth,
	      myTop + indicatorHeight);
    } else {
      hudColor4f(1.0f, 1.0f, 1.0f, 0.5f); // white
      glRecti(indicatorLeft, myTop, indicatorLeft + myWidth, myTop + indicatorHeight);
    }
  }
  glDisable(GL_BLEND);

  delete[] factors;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
