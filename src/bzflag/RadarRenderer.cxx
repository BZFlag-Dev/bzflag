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

#include <stdlib.h>
#include "bzfgl.h"
#include "global.h"
#include "OpenGLTexture.h"
#include "RadarRenderer.h"
#include "SceneRenderer.h"
#include "MainWindow.h"
#include "World.h"
#include "LocalPlayer.h"
#include "RemotePlayer.h"
#include "Team.h"
#include "Flag.h"
#include "OpenGLGState.h"

RadarRenderer::RadarRenderer(const SceneRenderer& renderer,
						const World& _world) :
				world(_world),
				x(0),
				y(0),
				w(0),
				h(0),
				range(RadarHiRange),
				jammed(False),
				decay(0.01),
				list(0),
				noise(NULL)
{
  background[0] = 0.0f;
  background[1] = 0.0f;
  background[2] = 0.0f;
  background[3] = 1.0f;
  blend = renderer.useBlending();
  smooth = True;
#if defined(GLX_SAMPLES_SGIS) && defined(GLX_SGIS_multisample)
  GLint bits;
  glGetIntergerv(GL_SAMPLES_SGIS, &bits);
  if (bits > 0) smooth = False;
#endif

  // watch for context recreation
  OpenGLGState::registerContextInitializer(initContext, (void*)this);
  if (makeNoise()==true)
    makeNoiseTexture();
  else noiseTexture=0;
}

RadarRenderer::~RadarRenderer()
{
  OpenGLGState::unregisterContextInitializer(initContext, (void*)this);
  freeList();
  delete[] noise;
}

void			RadarRenderer::setShape(int _x, int _y, int _w, int _h)
{
  x = _x;
  y = _y;
  w = _w;
  h = _h;
  makeNoise();
}

void			RadarRenderer::setRange(float _range)
{
  range = _range;
}

void			RadarRenderer::setJammed(boolean _jammed)
{
  jammed = _jammed;
  decay = 0.01;
}

void			RadarRenderer::freeList()
{
  if (list == 0) return;
  glDeleteLists(list, 2);
  list = 0;
}

boolean			RadarRenderer::makeNoise()
{
  delete[] noise;
  const int size = 4 * 128 * 128;
  noise = new unsigned char[size];
  if (!noise) return false;
  for (int i = 0; i < size; i += 4 ) {
    unsigned char n = (unsigned char)floor(256.0 * bzfrand());
    noise[i+0] = n;
    noise[i+1] = n;
    noise[i+2] = n;
    noise[i+3] = n;
  }
  return true;
}

void			RadarRenderer::makeNoiseTexture()
{
  noiseTexture = new OpenGLTexture(128,128,noise,OpenGLTexture::Nearest);
}

void			RadarRenderer::drawShot(const ShotPath* shot)
{

  glBegin(GL_POINTS);
  glVertex2fv(shot->getPosition());
  glEnd();
}

void			RadarRenderer::drawTank(float x, float y,
						float z, float ps)
{
  GLfloat s = 2.5f + ps * (2.0f + 2.5f * z / (4.0f * BoxHeight));

  glBegin(GL_LINE_STRIP);
  glVertex2f(x-s,y);
  glVertex2f(x,y-s);
  glVertex2f(x+s,y);
  glVertex2f(x,y+s);
  glVertex2f(x-s,y);
  glEnd();

  s = 1.5f + ps * (2.0f + 2.0f * z / (4.0f * BoxHeight));
  glRectf(x - s, y - s, x + s, y + s);
}

void			RadarRenderer::render(SceneRenderer& renderer,
							boolean blank)
{
  const boolean smoothingOn = smooth && renderer.useSmoothing();

  // if opaque then clear
  const int ox = renderer.getWindow().getOriginX();
  const int oy = renderer.getWindow().getOriginY();
  glScissor(ox + x, oy + y, w, h);
  if (blank || !blend || background[3] == 1.0f) {
    glClearColor(background[0], background[1], background[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    if (blank) return;
  }

  // prepare transforms
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  const int xSize = renderer.getWindow().getWidth();
  const int ySize = renderer.getWindow().getHeight();
  const double xCenter = double(x) + 0.5 * double(w);
  const double yCenter = double(y) + 0.5 * double(h);
  const double xUnit = 2.0 * range / double(w);
  const double yUnit = 2.0 * range / double(h);
  glOrtho(-xCenter * xUnit, (xSize - xCenter) * xUnit,
		-yCenter * yUnit, (ySize - yCenter) * yUnit, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  OpenGLGState::resetState();

  // if background transparent and blending on then blend in background
  if (blend && background[3] != 1.0f) {
    glEnable(GL_BLEND);
    glColor4fv(background);
    glRectf(-range, -range, range, range);
    glDisable(GL_BLEND);
  }

  // if jammed then draw white noise.  occasionally draw a good frame.
  if (jammed && bzfrand() > decay) {

    glColor3f(1.0,1.0,1.0);

    if (noiseTexture != 0 && renderer.useQuality()>0) {

      const int sequences = 10;

      static float np[] =
          { 0, 0, 1, 1,
            1, 1, 0, 0,
            0.5 , 0.5, 1.5, 1.5,
            1.5, 1.5, 0.5, 0.5,
            0.25, 0.25, 1.25, 1.25,
            1.25, 1.25, 0.25, 0.25,
            0, 0.5, 1, 1.5,
            1, 1.5, 0, 0.5,
            0.5, 0, 1.5, 1,
            1.4, 1, 0.5, 0,
            0.75, 0.75, 1.75, 1.75,
            1.75, 1.75, 0.75, 0.75,
          };

      int noisePattern = 4 * int(floor(sequences * bzfrand()));

      glEnable(GL_TEXTURE_2D);
      noiseTexture->execute();
      glBegin(GL_QUADS);

        glTexCoord2f(np[noisePattern+0],np[noisePattern+1]);
        glVertex2f(-range,-range);
        glTexCoord2f(np[noisePattern+2],np[noisePattern+1]);
        glVertex2f( range,-range);
        glTexCoord2f(np[noisePattern+2],np[noisePattern+3]);
        glVertex2f( range, range);
        glTexCoord2f(np[noisePattern+0],np[noisePattern+3]);
        glVertex2f(-range, range);

      glEnd();
      glDisable(GL_TEXTURE_2D);
    }

    else if (noiseTexture != 0 && renderer.useTexture()==true &&
	renderer.useQuality()==0) {
      glEnable(GL_TEXTURE_2D);
      noiseTexture->execute();
      glBegin(GL_QUADS);

        glTexCoord2f(0,0);
        glVertex2f(-range,-range);
        glTexCoord2f(1,0);
        glVertex2f( range,-range);
        glTexCoord2f(1,1);
        glVertex2f( range, range);
        glTexCoord2f(0,1);
        glVertex2f(-range, range);

      glEnd();
      glDisable(GL_TEXTURE_2D);
    }
    if (decay > 0.015f) decay *= 0.5f;
  }

  // only draw if there's a local player
  else if (LocalPlayer::getMyTank()) {
    // if decay is sufficiently small then boost it so it's more
    // likely a jammed radar will get a few good frames closely
    // spaced in time.  value of 1 guarantees at least two good
    // frames in a row.
    if (decay <= 0.015f) decay = 1.0f;
    else decay *= 0.5f;

    // get size of pixel in model space (assumes radar is square)
    GLfloat ps = 2.0f * range / GLfloat(w);

    // relative to my tank
    const LocalPlayer* myTank = LocalPlayer::getMyTank();
    const float* pos = myTank->getPosition();
    float angle = myTank->getAngle();
    glPushMatrix();
    glRotatef(90.0f - angle * 180.0f / M_PI, 0.0f, 0.0f, 1.0f);
    glPushMatrix();
    glTranslatef(-pos[0], -pos[1], 0.0f);

    // draw buildings and stuff
    if (list == 0) {
      list = glGenLists(2);

      glNewList(list, GL_COMPILE);
      makeList(False);
      glEndList();

      glNewList(list + 1, GL_COMPILE);
      makeList(True);
      glEndList();
    }
    glCallList(smoothingOn ? list + 1 : list);

    // antialiasing on for lines and points unless we're multisampling,
    // in which case it's automatic and smoothing makes them look worse.
    if (smoothingOn) {
      glEnable(GL_BLEND);
      glEnable(GL_LINE_SMOOTH);
      glEnable(GL_POINT_SMOOTH);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    // draw my shots
    const int maxShots = world.getMaxShots();
    glColor3f(1.0f, 1.0f, 1.0f);
    int i;
    for (i = 0; i < maxShots; i++) {
      const ShotPath* shot = myTank->getShot(i);
      if (shot) shot->radarRender();
    }

    // draw other tanks (and any flags on them)
    // note about flag drawing.  each line segment is drawn twice
    // (once in each direction);  this degrades the antialiasing
    // but on systems that don't do correct filtering of endpoints
    // not doing it makes (half) the endpoints jump wildly.
    const int maxPlayers = world.getMaxPlayers();
    GLfloat s;
    for (i = 0; i < maxPlayers; i++) {
      RemotePlayer* player = world.getPlayer(i);
      if (!player || !player->isAlive() || player->getFlag() == StealthFlag)
	continue;

      GLfloat x = player->getPosition()[0];
      GLfloat y = player->getPosition()[1];
      if (player->getFlag() != NoFlag) {
	GLfloat s = 5.0f * ps;
	glBegin(GL_LINES);
	glColor3fv(Flag::getColor(player->getFlag()));
	glVertex2f(x - s, y);
	glVertex2f(x + s, y);
	glVertex2f(x + s, y);
	glVertex2f(x - s, y);
	glVertex2f(x, y - s);
	glVertex2f(x, y + s);
	glVertex2f(x, y + s);
	glVertex2f(x, y - s);
	glEnd();
      }

      if (myTank->getFlag() == ColorblindnessFlag)
	glColor3fv(Team::getRadarColor(RogueTeam));
      else {
	if (player->isPaused() || player->isNotResponding()) {
	  const float dimfactor=0.4;
	  const float *color = Team::getRadarColor(myTank->getFlag() == 
	    ColorblindnessFlag ? RogueTeam : player->getTeam());
	  float dimmedcolor[3];
	  dimmedcolor[0] = color[0] * dimfactor;
	  dimmedcolor[1] = color[1] * dimfactor;
	  dimmedcolor[2] = color[2] * dimfactor;
	  glColor3fv(dimmedcolor);
	} else
	  glColor3fv(Team::getRadarColor(player->getTeam()));
      }
      drawTank(x, y, player->getPosition()[2], ps);
    }

    // draw other tanks' shells
    glColor3f(1.0f, 1.0f, 1.0f);
    for (i = 0; i < maxPlayers; i++) {
      RemotePlayer* player = world.getPlayer(i);
      if (!player) continue;
      for (int j = 0; j < maxShots; j++) {
	// color radar patch -- CB
	glColor3fv(Team::getRadarColor(player->getTeam()));
	const ShotPath* shot = player->getShot(j);
	if (shot && shot->getFlag() != InvisibleBulletFlag)
	  shot->radarRender();
      }
    }

    // draw flags not on tanks.
    s = 3.0f * ps;
    glBegin(GL_LINES);
    const int maxFlags = world.getMaxFlags();
    for (i = 0; i < maxFlags; i++) {
      const Flag& flag = world.getFlag(i);
      if (flag.status == FlagNoExist || flag.status == FlagOnTank)
	continue;
      glColor3fv(Flag::getColor(flag.id));
      glVertex2f(flag.position[0] - s, flag.position[1]);
      glVertex2f(flag.position[0] + s, flag.position[1]);
      glVertex2f(flag.position[0] + s, flag.position[1]);
      glVertex2f(flag.position[0] - s, flag.position[1]);
      glVertex2f(flag.position[0], flag.position[1] - s);
      glVertex2f(flag.position[0], flag.position[1] + s);
      glVertex2f(flag.position[0], flag.position[1] + s);
      glVertex2f(flag.position[0], flag.position[1] - s);
    }
    // draw antidote flag
    const float* antidotePos =
		LocalPlayer::getMyTank()->getAntidoteLocation();
    if (antidotePos) {
      glColor3f(1.0f, 1.0f, 0.0f);
      glVertex2f(antidotePos[0] - s, antidotePos[1]);
      glVertex2f(antidotePos[0] + s, antidotePos[1]);
      glVertex2f(antidotePos[0] + s, antidotePos[1]);
      glVertex2f(antidotePos[0] - s, antidotePos[1]);
      glVertex2f(antidotePos[0], antidotePos[1] - s);
      glVertex2f(antidotePos[0], antidotePos[1] + s);
      glVertex2f(antidotePos[0], antidotePos[1] + s);
      glVertex2f(antidotePos[0], antidotePos[1] - s);
    }
    glEnd();

    // draw these markers above all others
    glColor3f(1.0f, 1.0f, 1.0f);

    // north marker
    glPopMatrix();	// always centered
    GLfloat ns = 0.05f * range, ny = 0.9f * range;
    glBegin(GL_LINE_STRIP);
    glVertex2f(-ns, ny - ns);
    glVertex2f(-ns, ny + ns);
    glVertex2f(ns, ny - ns);
    glVertex2f(ns, ny + ns);
    glEnd();

    // forward tick
    glPopMatrix();	// always up
    glBegin(GL_LINES);
    glVertex2f(0.0f, range - ps);
    glVertex2f(0.0f, range - 4.0f * ps);
    glEnd();

    // view frustum edges
    glColor3f(1.0f, 0.625f, 0.125f);
    const float fovx = renderer.getViewFrustum().getFOVx();
    const float viewWidth = range * tanf(0.5f * fovx);
    glBegin(GL_LINE_STRIP);
    glVertex2f(-viewWidth, range);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(viewWidth, range);
    glEnd();

    if (smoothingOn) {
      glDisable(GL_BLEND);
      glDisable(GL_LINE_SMOOTH);
      glDisable(GL_POINT_SMOOTH);
    }

    // my tank
    glColor3f(1.0f, 1.0f, 1.0f);
    drawTank(0.0f, 0.0f, myTank->getPosition()[2], ps);

    // my flag
    if (myTank->getFlag() != NoFlag) {
      glColor3fv(Flag::getColor(myTank->getFlag()));
      glBegin(GL_LINES);
      glVertex2f(-5.0f*ps, 0.0f);
      glVertex2f(5.0f*ps, 0.0f);
      glVertex2f(5.0f*ps, 0.0f);
      glVertex2f(-5.0f*ps, 0.0f);
      glVertex2f(0.0f, -5.0f*ps);
      glVertex2f(0.0f, 5.0f*ps);
      glVertex2f(0.0f, 5.0f*ps);
      glVertex2f(0.0f, -5.0f*ps);
      glEnd();
    }
  }

  // restore GL state
  glPopMatrix();
}

void			RadarRenderer::makeList(boolean smoothingOn)
{
  // antialias if smoothing is on.
  if (smoothingOn) {
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  // draw walls.  walls are flat so a line will do.
  const WallObstacles& walls = world.getWalls();
  int count = walls.getLength();
  glColor3f(0.25f, 0.5f, 0.5f);
  glBegin(GL_LINES);
  int i;
  for (i = 0; i < count; i++) {
    const WallObstacle& wall = walls[i];
    const float w = wall.getBreadth();
    const float c = w * cosf(wall.getRotation());
    const float s = w * sinf(wall.getRotation());
    const float* pos = wall.getPosition();
    glVertex2f(pos[0] - s, pos[1] + c);
    glVertex2f(pos[0] + s, pos[1] - c);
  }
  glEnd();

  // draw team bases
  if (world.allowTeamFlags()) {
    for (i = 1; i < NumTeams; i++) {
      const float* base = world.getBase(i);
      glColor3fv(Team::getRadarColor(TeamColor(i)));
      glBegin(GL_LINE_LOOP);
	glVertex2f(base[0] - base[4], base[1] - base[5]);
	glVertex2f(base[0] + base[4], base[1] - base[5]);
	glVertex2f(base[0] + base[4], base[1] + base[5]);
	glVertex2f(base[0] - base[4], base[1] + base[5]);
      glEnd();
    }
  }

  // don't blend the polygons
  if (smoothingOn) glDisable(GL_BLEND);

  // draw box buildings.
  const BoxBuildings& boxes = world.getBoxes();
  count = boxes.getLength();
  glColor3f(0.25f, 0.5f, 0.5f);
  glBegin(GL_QUADS);
  for (i = 0; i < count; i++) {
    const BoxBuilding& box = boxes[i];
    const float c = cosf(box.getRotation());
    const float s = sinf(box.getRotation());
    const float wx = c * box.getWidth(), wy = s * box.getWidth();
    const float hx = -s * box.getBreadth(), hy = c * box.getBreadth();
    const float* pos = box.getPosition();
    glVertex2f(pos[0] - wx - hx, pos[1] - wy - hy);
    glVertex2f(pos[0] + wx - hx, pos[1] + wy - hy);
    glVertex2f(pos[0] + wx + hx, pos[1] + wy + hy);
    glVertex2f(pos[0] - wx + hx, pos[1] - wy + hy);
  }
  glEnd();

  // draw pyramid buildings
  const PyramidBuildings& pyramids = world.getPyramids();
  count = pyramids.getLength();
  glBegin(GL_QUADS);
  for (i = 0; i < count; i++) {
    const PyramidBuilding& pyr = pyramids[i];
    const float c = cosf(pyr.getRotation());
    const float s = sinf(pyr.getRotation());
    const float wx = c * pyr.getWidth(), wy = s * pyr.getWidth();
    const float hx = -s * pyr.getBreadth(), hy = c * pyr.getBreadth();
    const float* pos = pyr.getPosition();
    glVertex2f(pos[0] - wx - hx, pos[1] - wy - hy);
    glVertex2f(pos[0] + wx - hx, pos[1] + wy - hy);
    glVertex2f(pos[0] + wx + hx, pos[1] + wy + hy);
    glVertex2f(pos[0] - wx + hx, pos[1] - wy + hy);
  }
  glEnd();

  // now draw antialiased outlines around the polygons
  if (smoothingOn) {
    glEnable(GL_BLEND);
    count = boxes.getLength();
    for (i = 0; i < count; i++) {
      const BoxBuilding& box = boxes[i];
      const float c = cosf(box.getRotation());
      const float s = sinf(box.getRotation());
      const float wx = c * box.getWidth(), wy = s * box.getWidth();
      const float hx = -s * box.getBreadth(), hy = c * box.getBreadth();
      const float* pos = box.getPosition();
      glBegin(GL_LINE_LOOP);
      glVertex2f(pos[0] - wx - hx, pos[1] - wy - hy);
      glVertex2f(pos[0] + wx - hx, pos[1] + wy - hy);
      glVertex2f(pos[0] + wx + hx, pos[1] + wy + hy);
      glVertex2f(pos[0] - wx + hx, pos[1] - wy + hy);
      glEnd();
    }

    count = pyramids.getLength();
    for (i = 0; i < count; i++) {
      const PyramidBuilding& pyr = pyramids[i];
      const float c = cosf(pyr.getRotation());
      const float s = sinf(pyr.getRotation());
      const float wx = c * pyr.getWidth(), wy = s * pyr.getWidth();
      const float hx = -s * pyr.getBreadth(), hy = c * pyr.getBreadth();
      const float* pos = pyr.getPosition();
      glBegin(GL_LINE_LOOP);
      glVertex2f(pos[0] - wx - hx, pos[1] - wy - hy);
      glVertex2f(pos[0] + wx - hx, pos[1] + wy - hy);
      glVertex2f(pos[0] + wx + hx, pos[1] + wy + hy);
      glVertex2f(pos[0] - wx + hx, pos[1] - wy + hy);
      glEnd();
    }
  }

  // draw teleporters.  teleporters are pretty thin so use lines
  // (which, if longer than a pixel, are guaranteed to draw something;
  // not so for a polygon).  just in case the system doesn't correctly
  // filter the ends of line segments, we'll draw the line in each
  // direction (which degrades the antialiasing).  Newport graphics
  // is one system that doesn't do correct filtering.
  const Teleporters& teleporters = world.getTeleporters();
  count = teleporters.getLength();
  glColor3f(1.0f, 1.0f, 0.25f);
  glBegin(GL_LINES);
  for (i = 0; i < count; i++) {
    const Teleporter& tele = teleporters[i];
    const float w = tele.getBreadth();
    const float c = w * cosf(tele.getRotation());
    const float s = w * sinf(tele.getRotation());
    const float* pos = tele.getPosition();
    glVertex2f(pos[0] - s, pos[1] + c);
    glVertex2f(pos[0] + s, pos[1] - c);
    glVertex2f(pos[0] + s, pos[1] - c);
    glVertex2f(pos[0] - s, pos[1] + c);
  }
  glEnd();

  if (smoothingOn) {
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
  }
}

void			RadarRenderer::doInitContext()
{
  // forget about old lists
  list = 0;
}

void			RadarRenderer::initContext(void* self)
{
  ((RadarRenderer*)self)->doInitContext();
}
