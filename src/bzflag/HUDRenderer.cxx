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

#include "bzfgl.h"
#include "global.h"
#include "HUDRenderer.h"
#include "SceneRenderer.h"
#include "MainWindow.h"
#include "BzfDisplay.h"
#include "BzfWindow.h"
#include "LocalPlayer.h"
#include "RemotePlayer.h"
#include "DeadPlayer.h"
#include "World.h"
#include "OpenGLGState.h"
#include "texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

//
// FlashClock
//	keeps track of time for something that flashes
//

FlashClock::FlashClock() : duration(0.0f), onDuration(0.0f), flashDuration(0.0f)
{
  // do nothing
}

FlashClock::~FlashClock()
{
  // do nothing
}

void			FlashClock::setClock(float _duration)
{
  setClock(_duration, 0.0f, 0.0f);
}

void			FlashClock::setClock(float _duration,
						float onTime, float offTime)
{
  startTime = TimeKeeper::getTick();
  duration = _duration;
  if (onTime <= 0.0f || offTime <= 0.0f) {
    onDuration = 0.0f;
    flashDuration = 0.0f;
  }
  else {
    onDuration = onTime;
    flashDuration = onTime + offTime;
  }
}

boolean			FlashClock::isOn()
{
  if (duration == 0.0f) return False;
  const float dt = TimeKeeper::getTick() - startTime;
  if (duration > 0.0f && dt >= duration) {
    duration = 0.0f;
    return False;
  }
  if (flashDuration == 0.0f) return True;
  return (fmodf(dt, flashDuration) < onDuration);
}

//
// HUDRenderer
//

// headingOffset:  the number of degrees from the center of the heading
// strip display to either side.  altitudeOffset is similar.
const float		HUDRenderer::altitudeOffset = 20.0f;
const GLfloat		HUDRenderer::black[3] = { 0.0f, 0.0f, 0.0f };
BzfString		HUDRenderer::headingLabel[36];
BzfString		HUDRenderer::altitudeLabel[20];
BzfString		HUDRenderer::scoreSpacingLabel("888 (888-888)");
BzfString		HUDRenderer::scoreLabel("Score");
BzfString		HUDRenderer::killLabel("Kills");
BzfString		HUDRenderer::teamScoreLabel("Team Score");
BzfString		HUDRenderer::playerLabel("Player");
BzfString		HUDRenderer::restartLabelFormat("Press %s to start");
BzfString		HUDRenderer::resumeLabel("Press Pause to resume");
BzfString		HUDRenderer::gameOverLabel("GAME OVER");
const char*		HUDRenderer::flagHelpString[int(LastFlag) -
							int(FirstFlag) + 1] = {
"",
"You have no flag.",
"Your team's flag:  prevent other teams from capturing it!",
"Opponent's team flag:  take it to your base to capture it!",
"Velocity (+V):  Tank moves faster.  Outrun bad guys.",
"Angular velocity (+A):  Tank turns faster.  Dodge quicker.",
"Oscillation Overthruster (+OO):  Can drive through buildings.  "
		"Can't backup or shoot while inside.",
"rapid Fire (+F):  Shoots more often.  Shells go faster but not as far.",
"Machine Gun (+MG):  Very fast reload and very short range.",
"Guided Missile (+GM):  Shots track a target.  "
		"Lock on with right button.  "
		"Can lock on or retarget after firing.",
"Laser (+L):  Shoots a laser.  Infinite speed and range but long reload time.",
"Ricochet (+R):  Shots bounce off walls.  Don't shoot yourself!",
"SuperBullet (+SB):  Shoots through buildings.  Can kill Phantom Zone.",
"Invisible Bullet (+IB):  Your shots don't appear on other radars.  "
		"Can still see them out window.",
"STealth (+ST):  Tank is invisible on radar.  "
		"Shots are still visible.  Sneak up behind enemies!",
"Tiny (+T):  Tank is small and can get through small openings.  "
		"Very hard to hit.",
"Narrow (+N):  Tank is super thin.  Very hard to hit from front but is "
		"normal size from side.  Can get through small openings.",
"SHield (+SH):  Getting hit only drops flag.  Flag flys an extra-long time.",
"SteamRoller (+SR):  Destroys tanks you touch but you have to get really close.",
"Shock Wave (+SW):  Firing destroys all tanks nearby.  "
		"Don't kill teammates!  Can kill tanks on/in buildings.",
"Phantom Zone (+PZ):  Teleporting toggles Zoned effect.  "
		"Zoned tank can drive through buildings.  "
		"Zoned tank can't shoot or be shot (except by "
		"superbullet and shock wave).",
"Genocide (+G):  Killing one tank kills that tank's whole team.",
"JumPing (+JP):  Tank can jump.  Use Tab key.  Can't steer in the air.",
"IDentify (+ID):  Identifies type of nearest flag.",
"CLoaking (+CL):  Makes your tank invisible out-the-window.  "
		"Still visible on radar.",
"ColorBlindness (-CB):  Can't tell team colors.  Don't shoot teammates!",
"Obesity (-O):  Tank becomes very large.  Can't fit through teleporters.",
"left turn only (- <-):  Can't turn right.",
"right turn only (- ->):  Can't turn left.",
"Momentum (-M):  Tank has inertia.  Acceleration is limited.",
"Blindness (-B):  Can't see out window.  Radar still works.",
"JaMming (-JM):  Radar doesn't work.  Can still see.",
"Wide Angle (-WA):  Fish-eye lens distorts view."
			};

HUDRenderer::HUDRenderer(const BzfDisplay* _display,
				const SceneRenderer& renderer) :
				display(_display),
				window(renderer.getWindow()),
				firstRender(True),
				playing(False),
				dim(False),
				sDim(False),
				numPlayers(0),
				timeLeft(-1),
				playerHasHighScore(False),
				teamHasHighScore(False),
				heading(0.0),
				altitude(0.0),
				altitudeTape(False),
				fps(-1.0),
				drawTime(-1.0),
				restartLabel(restartLabelFormat),
				showCompose(False)
{
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

  composeFont = TextureFont::getTextureFont(TextureFont::FixedBold, True);
  bigFont = TextureFont::getTextureFont(TextureFont::HelveticaBoldItalic, True);
  alertFont = TextureFont::getTextureFont(TextureFont::HelveticaBold, True);
  majorFont = TextureFont::getTextureFont(TextureFont::TimesBold, True);
  minorFont = TextureFont::getTextureFont(TextureFont::FixedBold, True);
  headingFont = TextureFont::getTextureFont(TextureFont::Fixed, True);

  // make sure we're notified when MainWindow resizes
  window.getWindow()->addResizeCallback(resizeCallback, this);

  // initialize heading and altitude labels
  if (headingLabel[0].isNull()) {
    for (i = 0; i < 36; i++) {
      char buf[10];
      sprintf(buf, "%d", i * 10);
      headingLabel[i] = BzfString(buf);
    }
    for (i = 0; i < 20; i++) {
      char buf[10];
      sprintf(buf, "%d", i * 5);
      altitudeLabel[i] = BzfString(buf);
    }
  }

  // initialize miscellaneous stuff
  for (i = 0; i < MaxHUDMarkers; i++) {
    marker[i].on = False;
    marker[i].heading = 0.0f;
    marker[i].color[0] = 0.0f;
    marker[i].color[1] = 0.0f;
    marker[i].color[2] = 0.0f;
  }
  flagHelpIndex = 0;

  // initialize clocks
  globalClock.setClock(-1.0f, 0.8f, 0.4f);
  scoreClock.setClock(-1.0f, 0.5f, 0.2f);

  // create compose dialog
  composeTypeIn = new HUDuiTypeIn();
  composeTypeIn->setLabel("Send:");
  composeTypeIn->setMaxLength(MessageLen - 1);
  composeTypeIn->showFocus(False);
  composeTypeIn->setFont(composeFont);

  // initialize fonts
  resize(True);
}

HUDRenderer::~HUDRenderer()
{
  // don't notify me anymore (cos you can't wake the dead!)
  window.getWindow()->removeResizeCallback(resizeCallback, this);

  // release ui controls
  delete composeTypeIn;
}

void			HUDRenderer::resizeCallback(void* self)
{
  ((HUDRenderer*)self)->resize(False);
}

void			HUDRenderer::resize(boolean firstTime)
{
  // get important metrics
  const int w = firstTime ? 1280 : window.getWidth();
  const int h = firstTime ? 768 : window.getViewHeight();

  // compute good targeting box sizes
  {
    const float xScale = (float)w / 1280.0f;
    const float yScale = (float)h / 768.0f;
    const float scale = (xScale < yScale) ? xScale : yScale;
    maxMotionSize = (int)((float)MaxMotionSize * scale);
    noMotionSize = NoMotionSize; //(int)((float)NoMotionSize * scale);
    headingOffset = 45.0f * (scale > 1.0f ? 1.0f : scale);
  }

  // initialize readout spacings
  headingMarkSpacing = (int)(5.0f * float(maxMotionSize) / headingOffset);
  altitudeMarkSpacing = floorf(5.0f * float(maxMotionSize) / altitudeOffset);

  // initialize cracks
  for (int i = 0; i < HUDNumCracks; i++) {
    const float d = 0.5f * float(maxMotionSize) * ((float)bzfrand() + 0.5f);
    const float a = 2.0f * M_PI * (float(i) + (float)bzfrand()) /
							float(HUDNumCracks);
    cracks[i][0][0] = 0.0f;
    cracks[i][0][1] = 0.0f;
    cracks[i][1][0] = d * cosf(a);
    cracks[i][1][1] = d * sinf(a);
    makeCrack(i, 1, a);
  }

  // pick appropriate font sizes
  setBigFontSize(w, h);
  setAlertFontSize(w, h);
  setMajorFontSize(w, h);
  setMinorFontSize(w, h);
  setHeadingFontSize(w, h);
  setComposeFontSize(w, h);

  // set compose control positions and sizes
  {
    OpenGLTexFont font = composeTypeIn->getFont();
    if (font.isValid()) {
      const float dx = font.getWidth(composeTypeIn->getLabel()) + 2.0f;
      const float dy = font.getDescent() + 4.0f;
      const float x = dx + dy + 2.0f * font.getSpacing();
      const float y = dy;
      composeTypeIn->setLabelWidth(dx);
      composeTypeIn->setPosition(x, y);
      composeTypeIn->setSize(w - x - dy, font.getSpacing());
    }
  }
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
  const float s = (float)height / 10.0f;
  bigFont.setSize(s, s);

  restartLabelWidth = bigFont.getWidth(restartLabel);
  resumeLabelWidth = bigFont.getWidth(resumeLabel);
  gameOverLabelWidth = bigFont.getWidth(gameOverLabel);
}

void			HUDRenderer::setAlertFontSize(int, int height)
{
  const float s = (float)height / 16.0f;
  alertFont.setSize(s, s);

  for (int i = 0; i < MaxAlerts; i++)
    if (alertClock[i].isOn())
      alertLabelWidth[i] = alertFont.getWidth(alertLabel[i]);
  alertY = -majorFont.getSpacing() +
	   -alertFont.getSpacing() + alertFont.getDescent();
}

void			HUDRenderer::setMajorFontSize(int, int height)
{
  const float s = (float)height / 16.0f;
  majorFont.setSize(s, s);

  alertY = -majorFont.getSpacing() +
	   -alertFont.getSpacing() + alertFont.getDescent();
}

void			HUDRenderer::setMinorFontSize(int, int height)
{
  const float s = (float)height / 28.0f;
  minorFont.setSize(s, s);

  scoreLabelWidth = minorFont.getWidth(scoreSpacingLabel);
  killsLabelWidth = minorFont.getWidth(killLabel);
  teamScoreLabelWidth = minorFont.getWidth(teamScoreLabel);
  if (scoreLabelWidth > teamScoreLabelWidth)
    teamScoreLabelWidth = scoreLabelWidth;
  flagHelpY = composeTypeIn->getFont().getSpacing() + 4.0f +
			minorFont.getDescent();

  const float spacing = minorFont.getWidth(" ");
  scoreLabelWidth += spacing;
  killsLabelWidth += spacing;

  // make flag help messages
  for (int i = 0; i < int(LastFlag) - int(FirstFlag) + 1; i++)
    flagHelp[i] = makeHelpString(flagHelpString[i]);
}

void			HUDRenderer::setHeadingFontSize(int, int height)
{
  const float s = (float)height / 64.0f;
  headingFont.setSize(s, s);

  // compute heading labels and (half) widths
  int i;
  for (i = 0; i < 36; i++)
    headingLabelWidth[i] = 0.5f * headingFont.getWidth(headingLabel[i]);

  // compute maximum width over all altitude labels
  altitudeLabelMaxWidth = 0.0f;
  for (i = 0; i < 20; i++) {
    const float w = headingFont.getWidth(altitudeLabel[i]);
    if (w > altitudeLabelMaxWidth) altitudeLabelMaxWidth = w;
  }
}

void			HUDRenderer::setComposeFontSize(int, int height)
{
  const float s = (float)height / 32.0f;
  composeTypeIn->setFontSize(s, s);
}

void			HUDRenderer::setColor(float r, float g, float b)
{
  hudColor[0] = r;
  hudColor[1] = g;
  hudColor[2] = b;
}

void			HUDRenderer::setPlaying(boolean _playing)
{
  playing = _playing;
}

void			HUDRenderer::setDim(boolean _dim)
{
  dim = _dim;
}

void			HUDRenderer::setPlayerHasHighScore(boolean hasHigh)
{
  playerHasHighScore = hasHigh;
}

void			HUDRenderer::setTeamHasHighScore(boolean hasHigh)
{
  teamHasHighScore = hasHigh;
}

void			HUDRenderer::setHeading(float angle)
{
  heading = 90.0f - 180.0f * angle / M_PI;
  while (heading < 0.0f) heading += 360.0f;
  while (heading >= 360.0f) heading -= 360.0f;
}

void			HUDRenderer::setAltitude(float _altitude)
{
  altitude = _altitude;
}

void			HUDRenderer::setAltitudeTape(boolean on)
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

void			HUDRenderer::setAlert(int index, const char* string,
						float duration, boolean warning)
{
  if (index < 0) index = 0;
  else if (index >= MaxAlerts) index = MaxAlerts - 1;
  if (!string) {
    alertClock[index].setClock(0.0f);
  }
  else {
    alertLabel[index] = string;
    alertLabelWidth[index] = alertFont.getWidth(alertLabel[index]);
    alertColor[index] = warning ? warningColor : messageColor;
    alertClock[index].setClock(duration);
  }
}

boolean			HUDRenderer::getComposing() const
{
  return showCompose;
}

BzfString		HUDRenderer::getComposeString() const
{
  return composeTypeIn->getString();
}

void			HUDRenderer::setComposing(const BzfString &prompt)
{
  showCompose = (!prompt.isNull());
  if (showCompose) {
    composeTypeIn->setLabel(prompt);
    composeTypeIn->setString("");
    composeTypeIn->setFocus();

    OpenGLTexFont font = composeTypeIn->getFont();
    if (font.isValid()) {
      const float dx = font.getWidth(composeTypeIn->getLabel()) + 2.0f;
      const float dy = font.getDescent() + 4.0f;
      const float x = dx + dy + 2.0f * font.getSpacing();
      const float y = dy;
      composeTypeIn->setLabelWidth(dx);
      composeTypeIn->setPosition(x, y);
      composeTypeIn->setSize(window.getWidth() - x - dy, font.getSpacing());
    }
  }
  else {
    HUDui::setFocus(NULL);
  }
}

void			HUDRenderer::setFlagHelp(FlagId id, float duration)
{
  if (id == NoFlag)
    flagHelpIndex = 1;
  else if (LocalPlayer::getMyTank() &&
		int(id) == int(LocalPlayer::getMyTank()->getTeam()))
    flagHelpIndex = 2;
  else if (int(id) >= int(FirstTeamFlag) && int(id) <= int(LastTeamFlag))
    flagHelpIndex = 3;
  else if (int(id) < int(FirstSuperFlag) || int(id) > int(LastSuperFlag))
    flagHelpIndex = 0;
  else
    flagHelpIndex = int(id) - int(FirstFlag);
  flagHelpClock.setClock(duration);

  // count the number of lines in the help message
  flagHelpLines = 0;
  const int helpLength = flagHelp[flagHelpIndex].getLength();
  const char* helpMsg = flagHelp[flagHelpIndex].getString();
  for (int i = 0; i < helpLength; i++)
    if (helpMsg[i] == '\0')
      flagHelpLines++;
}

void			HUDRenderer::setCracks(boolean _showCracks)
{
  showCracks = _showCracks;
}

void			HUDRenderer::setMarker(int index, boolean on)
{
  if (index < 0 || index >= MaxHUDMarkers) return;
  marker[index].on = on;
}

void			HUDRenderer::setMarkerHeading(int index, float _heading)
{
  if (index < 0 || index >= MaxHUDMarkers) return;
  _heading = 90.0f - 180.0f * _heading / M_PI;
  while (_heading < 0.0f) _heading += 360.0f;
  while (_heading >= 360.0f) _heading -= 360.0f;
  marker[index].heading = _heading;
}

void			HUDRenderer::setMarkerColor(int index,
						float r, float g, float b)
{
  if (index < 0 || index >= MaxHUDMarkers) return;
  marker[index].color[0] = r;
  marker[index].color[1] = g;
  marker[index].color[2] = b;
}

void			HUDRenderer::setRestartKeyLabel(const BzfString& label)
{
  char buffer[250];
  sprintf(buffer, restartLabelFormat, (const char*)label);
  restartLabel = buffer;
  restartLabelWidth = bigFont.getWidth(restartLabel);
}

void			HUDRenderer::setTimeLeft(int _timeLeft)
{
  timeLeft = _timeLeft;
  timeSet = TimeKeeper::getTick();
}

BzfString		HUDRenderer::makeHelpString(const char* help) const
{
  if (!help) return BzfString();

  // find sections of string not more than maxWidth pixels wide
  // and put them into a BzfString separated by NUL's.
  const float maxWidth = 3.0f * (float)maxMotionSize;
  BzfString msg;
  const char* scan = help;
  while (*scan) {
    // find next blank (can only break lines at spaces)
    const char* base = scan;
    do {
      while (*scan && isspace(*scan)) scan++;
      while (*scan && !isspace(*scan)) scan++;
    } while (*scan && minorFont.getWidth(base, scan - base) < maxWidth);

    // add chunk and NUL separator
    msg.append(base, scan - base);
    msg.append("", 1);

    // skip over blanks
    while (*scan && isspace(*scan)) scan++;
  }
  return msg;
}

void			HUDRenderer::makeCrack(int n, int l, float a)
{
  if (l >= (1 << (HUDCrackLevels - 1))) return;
  float d = 0.5f * float(maxMotionSize) *
		((float)bzfrand() + 0.5f) * powf(0.5f, 0.69f * logf(float(l)));
  float newAngle = a + M_PI * (float)bzfrand() / float(HUDNumCracks);
  cracks[n][2*l][0] = cracks[n][l][0] + d * cosf(newAngle);
  cracks[n][2*l][1] = cracks[n][l][1] + d * sinf(newAngle);
  makeCrack(n, 2*l, newAngle);
  d = 0.5f * float(maxMotionSize) *
		((float)bzfrand() + 0.5f) * powf(0.5f, 0.69f * logf(float(l)));
  newAngle = a - M_PI * (float)bzfrand() / float(HUDNumCracks);
  cracks[n][2*l+1][0] = cracks[n][l][0] + d * cosf(newAngle);
  cracks[n][2*l+1][1] = cracks[n][l][1] + d * sinf(newAngle);
  makeCrack(n, 2*l+1, newAngle);
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
  else if (sDim)
    glColor3f(0.5f * c[0], 0.5f * c[1], 0.5f * c[2]);
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
    firstRender = False;
    resize(False);
  }

  OpenGLGState::resetState();
  if (playing)
    renderPlaying(renderer);
  else
    renderNotPlaying(renderer);
}

void			HUDRenderer::renderAlerts(SceneRenderer& renderer)
{
  const float centerx = 0.5f * (float)renderer.getWindow().getWidth();
  float y = (float)renderer.getWindow().getViewHeight() + alertY;
  for (int i = 0; i < MaxAlerts; i++) {
    if (alertClock[i].isOn()) {
      hudColor3fv(alertColor[i]);
      alertFont.draw(alertLabel[i], centerx - 0.5f * alertLabelWidth[i], y);
    }
    y -= alertFont.getSpacing();
  }
}

void			HUDRenderer::renderStatus(SceneRenderer& renderer)
{
  LocalPlayer* player = LocalPlayer::getMyTank();
  if (!player || !World::getWorld()) return;

  char buffer[60];
  const float h = majorFont.getSpacing();
  float x = 0.25f * h, y = (float)renderer.getWindow().getViewHeight() - h;
  TeamColor teamIndex = player->getTeam();
  FlagId flag = player->getFlag();

  // print player name and score in upper left corner in team (radar) color
  if (!playerHasHighScore || scoreClock.isOn()) {
    sprintf(buffer, "%s: %d", player->getCallSign(), player->getScore());
    hudColor3fv(Team::getRadarColor(teamIndex));
    majorFont.draw(buffer, x, y);
  }

  // print flag if player has one in upper right
  if (flag != NoFlag) {
    sprintf(buffer, "%s", Flag::getName(flag));
    x = (float)renderer.getWindow().getWidth() -
			0.25f * h - majorFont.getWidth(buffer);
    if (Flag::getType(flag) == FlagSticky)
      hudColor3fv(warningColor);
    else
      hudColor3fv(messageColor);
    majorFont.draw(buffer, x, y);
  }

  // print status top-center
  static const GLfloat redColor[3] = { 1.0f, 0.0f, 0.0f };
  static const GLfloat yellowColor[3] = { 1.0f, 1.0f, 0.0f };
  static const GLfloat greenColor[3] = { 0.0f, 1.0f, 0.0f };
  const GLfloat* statusColor = warningColor;
  if (timeLeft >= 0) {
    int t = timeLeft - (int)(TimeKeeper::getTick() - timeSet);
    if (t < 0) t = 0;
    if (t >= 3600)
      sprintf(buffer, "%d:%02d:%02d   ", t / 3600, (t / 60) % 60, t % 60);
    else if (t >= 60)
      sprintf(buffer, "%d:%02d   ", t / 60, t % 60);
    else
      sprintf(buffer, "0:%02d   ", t);
  }
  else {
    strcpy(buffer, "");
  }
  switch (player->getFiringStatus()) {
    case LocalPlayer::Deceased:
      strcat(buffer, "Dead");
      break;

    case LocalPlayer::Ready:
      if (flag != NoFlag && Flag::getType(flag) == FlagSticky &&
				World::getWorld()->allowShakeTimeout()) {
	/* have a bad flag -- show time left 'til we shake it */
	statusColor = yellowColor;
	sprintf(buffer, "%.1f", player->getFlagShakingTime());
      }
      else {
	statusColor = greenColor;
	strcat(buffer, "Ready");
      }
      break;

    case LocalPlayer::Loading:
      statusColor = redColor;
      sprintf(buffer, "Reloaded in %.1f", player->getReloadTime());
      break;

    case LocalPlayer::Sealed:
      strcat(buffer, "Sealed");
      break;

    case LocalPlayer::Zoned:
      strcat(buffer, "Zoned");
      break;
  }

  x = 0.5f * ((float)renderer.getWindow().getWidth() -
					majorFont.getWidth(buffer));
  hudColor3fv(statusColor);
  majorFont.draw(buffer, x, y);
}

void			HUDRenderer::renderScoreboard(SceneRenderer& renderer)
{
  int i, j;

  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (!myTank || !World::getWorld()) return;

  const float x1 = 0.0125f * renderer.getWindow().getWidth();
  const float x2 = x1 + scoreLabelWidth;
  const float x3 = x2 + scoreLabelWidth;
  const float x5 = (1.0f - 0.01f) * renderer.getWindow().getWidth() -
							teamScoreLabelWidth;
  const float y0 = (float)renderer.getWindow().getViewHeight() -
				2.0f * alertFont.getSpacing();
  hudColor3fv(messageColor);
  minorFont.draw(scoreLabel, x1, y0);
  minorFont.draw(killLabel, x2, y0);
  minorFont.draw(playerLabel, x3, y0);
  minorFont.draw(teamScoreLabel, x5, y0);
  const float dy = minorFont.getSpacing();
  int y = (int)(y0 - dy);
  drawPlayerScore(myTank, x1, x2, x3, (float)y);
  y -= (int)dy;
  RemotePlayer **players_unsorted;
  RemotePlayer **players;
  int hiScore;
  int hiScoreIndex;
  RemotePlayer* hiScorePlayer;
  int plrCount = 0;
  int maxPlayers = World::getWorld()->getMaxPlayers();
  // run a sort by score
  players_unsorted = (RemotePlayer **)malloc(maxPlayers * sizeof(RemotePlayer *));
  players = (RemotePlayer **)malloc(maxPlayers * sizeof(RemotePlayer *));
  for (j = 0; j < maxPlayers; j++) {
    players_unsorted[j] = World::getWorld()->getPlayer(j);
    if (players_unsorted[j]) plrCount++;
  }
  for (i = 0; i < plrCount; i++) {
    hiScoreIndex = -1;
    hiScore = 0;
    hiScorePlayer = (RemotePlayer *)NULL;
    for (j = 0; j < maxPlayers; j++) {
      if (players_unsorted[j] && (hiScoreIndex < 0 || (players_unsorted[j]->getScore() > hiScore))) {
	hiScore = players_unsorted[j]->getScore();
	hiScorePlayer = players_unsorted[j];
        hiScoreIndex = j;
      }
    }
    if (hiScoreIndex >= 0) {
      players[i] = hiScorePlayer;
      players_unsorted[hiScoreIndex] = (RemotePlayer *)NULL;
    }
  }
  free(players_unsorted);
  for (i = 0; i < plrCount; i++) {
    RemotePlayer* player = players[i];
    if (!player) continue;
    y -= (int)dy;
    drawPlayerScore(player, x1, x2, x3, (float)y);
  }
  free(players);
  y -= (int)dy;
  const int maxDeadPlayers = World::getWorld()->getMaxDeadPlayers();
  DeadPlayer** deadPlayers = World::getWorld()->getDeadPlayers();
  for (i = 0; i < maxDeadPlayers; i++) {
    if (!deadPlayers[i]) continue;
    y -= (int)dy;
    drawDeadPlayerScore(deadPlayers[i], x1, x2, x3, (float)y);
  }

  y = (int)y0;
  for (i = RedTeam; i < NumTeams; i++) {
    const Team* team = World::getWorld()->getTeams() + i;
    if (team->activeSize == 0) continue;
    y -= (int)dy;
    drawTeamScore(i, x5, (float)y);
  }
}

void			HUDRenderer::renderCracks(SceneRenderer& renderer)
{
  glPushMatrix();
  glTranslatef(GLfloat(renderer.getWindow().getWidth() >> 1),
		GLfloat(renderer.getWindow().getViewHeight() >> 1), 0.0f);
  glLineWidth(3.0);
  hudColor3f(1.0f, 1.0f, 1.0f);
  glBegin(GL_LINES);
    for (int i = 0; i < HUDNumCracks; i++) {
      glVertex2fv(cracks[i][0]);
      glVertex2fv(cracks[i][1]);
      for (int j = 0; j < HUDCrackLevels-1; j++) {
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

void			HUDRenderer::renderTimes(SceneRenderer& renderer)
{
  const int centerx = renderer.getWindow().getWidth() >> 1;
  const int centery = renderer.getWindow().getViewHeight() >> 1;

  // draw frames per second
  if (fps > 0.0f) {
    char buf[20];
    sprintf(buf, "FPS: %d", int(fps));
    hudColor3f(1.0f, 1.0f, 1.0f);
    headingFont.draw(buf, (float)(centerx - maxMotionSize),
		(float)centery - (float)maxMotionSize -
		headingFont.getSpacing() + headingFont.getDescent());
  }
  if (drawTime > 0.0f) {
    char buf[20];
    sprintf(buf, "time: %dms", (int)(drawTime * 1000.0f));
    hudColor3f(1.0f, 1.0f, 1.0f);
    headingFont.draw(buf, (float)centerx,
		(float)centery - (float)maxMotionSize -
		headingFont.getSpacing() + headingFont.getDescent());
  }
}

void			HUDRenderer::renderPlaying(SceneRenderer& renderer)
{
  // get view metrics
  const int width = renderer.getWindow().getWidth();
  const int height = renderer.getWindow().getViewHeight();
  const int ox = renderer.getWindow().getOriginX();
  const int oy = renderer.getWindow().getOriginY();
  const int centerx = width >> 1;
  const int centery = height >> 1;
  int i;

  // use one-to-one pixel projection
  glScissor(ox, oy + renderer.getWindow().getPanelHeight(),
		width, renderer.getWindow().getHeight());
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, width,
	-GLdouble(renderer.getWindow().getPanelHeight()),
	GLdouble(renderer.getWindow().getViewHeight()),
	-1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // draw cracks
  if (showCracks)
    renderCracks(renderer);

  // draw status line
  renderStatus(renderer);

  // draw alert messages
  renderAlerts(renderer);

  // show player scoreboard
  float x, y;
  if (renderer.getScore()) renderScoreboard(renderer);

  // draw flag help
  if (flagHelpClock.isOn()) {
    hudColor3fv(messageColor);
    y = flagHelpY + (float)flagHelpLines * minorFont.getSpacing();
    const char* flagHelpBase = flagHelp[flagHelpIndex].getString();
    for (i = 0; i < flagHelpLines; i++) {
      y -= minorFont.getSpacing();
      minorFont.draw(flagHelpBase, (float)(centerx - 1.5f * maxMotionSize), y);
      while (*flagHelpBase) flagHelpBase++;
      flagHelpBase++;
    }
  }

  // draw times
  renderTimes(renderer);

  // draw message composition
  if (showCompose)
    renderCompose(renderer);

  OpenGLGState::resetState();
  const boolean smooth = renderer.useSmoothing();

  // draw targeting box
  hudColor3fv(hudColor);
  glBegin(GL_LINE_LOOP);
    glVertex2i(centerx - noMotionSize, centery - noMotionSize);
    glVertex2i(centerx + noMotionSize, centery - noMotionSize);
    glVertex2i(centerx + noMotionSize, centery + noMotionSize);
    glVertex2i(centerx - noMotionSize, centery + noMotionSize);
  glEnd();
  glBegin(GL_LINE_LOOP);
    glVertex2i(centerx - maxMotionSize, centery - maxMotionSize);
    glVertex2i(centerx + maxMotionSize, centery - maxMotionSize);
    glVertex2i(centerx + maxMotionSize, centery + maxMotionSize);
    glVertex2i(centerx - maxMotionSize, centery + maxMotionSize);
  glEnd();

  // draw heading strip
  if (True /* always draw heading strip */) {
    // first clip to area
    glScissor(ox + centerx - maxMotionSize, oy + centery + maxMotionSize - 5 +
		renderer.getWindow().getPanelHeight(),
		2 * maxMotionSize, 15 + (int)(headingFont.getSpacing() + 0.5f));

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

    boolean smoothLabel = smooth;
    x = (float)centerx - basex;
    y = floorf(8.0f + headingFont.getDescent()) +
	(float)(centery + maxMotionSize);
    if (smoothLabel) {
      x -= 0.5f;
      hudColor4f(hudColor[0], hudColor[1], hudColor[2], basex - floorf(basex));
    }
    for (i = minMark; i <= maxMark; i++) {
      headingFont.draw(headingLabel[(i + 36) % 36],
			x - headingLabelWidth[(i + 36) % 36], y);
      x += 2.0f * headingMarkSpacing;
    }
    if (smoothLabel) {
      x = (float)centerx - basex + 0.5f;
      basex -= floorf(basex);
      hudColor4f(hudColor[0], hudColor[1], hudColor[2], 1.0f - basex);
      for (i = minMark; i <= maxMark; i++) {
	headingFont.draw(headingLabel[(i + 36) % 36],
			x - headingLabelWidth[(i + 36) % 36], y);
	x += 2.0f * headingMarkSpacing;
      }
    }
    OpenGLGState::resetState();

    // draw markers (give 'em a little more space on the sides)
    glScissor(ox + centerx - maxMotionSize - 8, oy + centery + maxMotionSize +
		renderer.getWindow().getPanelHeight(),
		2 * maxMotionSize + 16, 10);
    glPushMatrix();
    glTranslatef((float)centerx, (float)(centery + maxMotionSize), 0.0f);
    for (i = 0; i < MaxHUDMarkers; i++) if (marker[i].on) {
      const float relAngle = fmodf(360.0f + marker[i].heading - heading, 360.0f);
      hudColor3fv(marker[i].color);
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
      }
      else if (relAngle <= 180.0) {
	// off to the right
	glBegin(GL_TRIANGLES);
	  glVertex2f((float)maxMotionSize, 0.0f);
	  glVertex2f((float)maxMotionSize + 4.0f, 4.0f);
	  glVertex2f((float)maxMotionSize, 8.0f);
	glEnd();
      }
      else {
	// off to the left
	glBegin(GL_TRIANGLES);
	  glVertex2f(-(float)maxMotionSize, 0.0f);
	  glVertex2f(-(float)maxMotionSize, 8.0f);
	  glVertex2f(-(float)maxMotionSize - 4.0f, 4.0f);
	glEnd();
      }
    }
    glPopMatrix();
  }

  // draw altitude strip
  if (altitudeTape) {
    // clip to area
    glScissor(ox + centerx + maxMotionSize - 5, oy +
		centery - maxMotionSize + renderer.getWindow().getPanelHeight(),
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
    int minMark = 0;
    int maxMark = baseMark + int(altitudeOffset / 5.0f) + 1;
    if (maxMark > 19) maxMark = 19;

    // draw tick marks
    glPushMatrix();
    if (smooth) {
      glEnable(GL_LINE_SMOOTH);
      glEnable(GL_BLEND);
    }
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

    boolean smoothLabel = smooth;
    x = (float)(10 + centerx + maxMotionSize);
    y = floorf(headingFont.getBaselineFromCenter()) + (float)centery - basey;
    if (smoothLabel) {
      y -= 0.5f;
      hudColor4f(hudColor[0], hudColor[1], hudColor[2], basey - floorf(basey));
    }
    for (i = minMark; i <= maxMark; i++) {
      headingFont.draw(altitudeLabel[i], x, y);
      y += altitudeMarkSpacing;
    }
    if (smoothLabel) {
      y = floorf(headingFont.getBaselineFromCenter()) + (float)centery - basey;
      y += 0.5f;
      basey -= floorf(basey);
      hudColor4f(hudColor[0], hudColor[1], hudColor[2], 1.0f - basey);
      for (i = minMark; i <= maxMark; i++) {
	headingFont.draw(altitudeLabel[i], x, y);
	y += altitudeMarkSpacing;
      }
    }
  }

  // restore graphics state
  glPopMatrix();
}

void			HUDRenderer::renderNotPlaying(SceneRenderer& renderer)
{
  extern boolean gameOver;

  // get view metrics
  const int width = renderer.getWindow().getWidth();
  const int height = renderer.getWindow().getViewHeight();
  const int ox = renderer.getWindow().getOriginX();
  const int oy = renderer.getWindow().getOriginY();

  // use one-to-one pixel projection
  glScissor(ox, oy + renderer.getWindow().getPanelHeight(),
		width, renderer.getWindow().getHeight());
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, width,
	-GLdouble(renderer.getWindow().getPanelHeight()),
	GLdouble(renderer.getWindow().getViewHeight()),
	-1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // draw cracks
  if (showCracks)
    renderCracks(renderer);

  // draw status line
  renderStatus(renderer);

  // draw alert messages
  renderAlerts(renderer);

  // show player scoreboard
  renderScoreboard(renderer);

  // draw times
  renderTimes(renderer);

  // draw message composition
  if (showCompose)
    renderCompose(renderer);

  // tell player what to do to start/resume playing
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank && globalClock.isOn()) {
    float y = 0.5f * (float)height + bigFont.getSpacing();
    if (gameOver) {
      hudColor3fv(messageColor);
      bigFont.draw(gameOverLabel,
			0.5f * ((float)width - gameOverLabelWidth), y);
    }
    else if (!myTank->isAlive() && !myTank->isExploding()) {
      hudColor3fv(messageColor);
      bigFont.draw(restartLabel,
			0.5f * ((float)width - restartLabelWidth), y);
    }
    else if (myTank->isPaused()) {
      hudColor3fv(messageColor);
      bigFont.draw(resumeLabel,
			0.5f * ((float)width - resumeLabelWidth), y);
    }
  }

  // restore graphics state
  glPopMatrix();
}

void			HUDRenderer::drawPlayerScore(const Player* player,
					float x1, float x2, float x3, float y)
{
  char score[40], kills[40];
#ifndef DEBUG
  char email[EmailLen + 6];
  sprintf(email, " (%s)%s", player->getEmailAddress(),Flag::getAbbreviation(player->getFlag()));
#else
  char email[EmailLen + 27];
  const PlayerId& id = player->getId();
  sprintf(email, " %s:%04x-%1x(%s)%s", inet_ntoa(id.serverHost),
      ntohs(id.port), ntohs(id.number), player->getEmailAddress(),
      Flag::getAbbreviation(player->getFlag()));
#endif  
  sprintf(score, "%d (%d-%d)", player->getScore(),
      player->getWins(), player->getLosses());
  if (LocalPlayer::getMyTank() != player)
    sprintf(kills, "%d/%d", player->getLocalWins(), player->getLocalLosses());
  else
    strcpy(kills, "");

  const float callSignWidth = minorFont.getWidth(player->getCallSign());

  hudSColor3fv(Team::getRadarColor(player->getTeam()));
  minorFont.draw(score, x1, y);
  minorFont.draw(kills, x2, y);
  minorFont.draw(player->getCallSign(), x3, y);
  minorFont.draw(email, x3 + callSignWidth, y);
}

void			HUDRenderer::drawDeadPlayerScore(const Player* player,
					float x1, float x2, float x3, float y)
{
  // draw dead player scores in a darker shade
  sDim = True;
  drawPlayerScore(player, x1, x2, x3, y);
  sDim = False;
}

void			HUDRenderer::drawTeamScore(int teamIndex,
					float x1, float y)
{
  char score[40];
  Team& team = World::getWorld()->getTeam(teamIndex);
  sprintf(score, "%d (%d-%d)", team.won - team.lost, team.won, team.lost);

  hudColor3fv(Team::getRadarColor((TeamColor)teamIndex));
  minorFont.draw(score, x1, y);
}
