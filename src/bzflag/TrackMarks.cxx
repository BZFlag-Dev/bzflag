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


// BZFlag common header
#include "common.h"

// System headers
#include <math.h>
#include <string>
#include <list>

// Interface header
#include "TrackMarks.h"

// Common interface headers
#include "TimeKeeper.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "Ray.h"
#include "CollisionManager.h"
#include "PhysicsDriver.h"
#include "bzfgl.h"
#include "TextureManager.h"
#include "SceneRenderer.h"
#include "OpenGLGState.h"
#include "OpenGLMaterial.h"


using namespace TrackMarks;

enum TrackType {
  treads = 0,
  puddle = 1
};

enum TrackSides {
  LeftTread =  (1 << 0),
  RightTread = (1 << 1),
  BothTreads = (LeftTread | RightTread)
};

typedef struct {
  float pos[3];
  float angle;
  float scale;
  char sides;
  int phydrv;
  TimeKeeper startTime;
} TrackEntry;


// Local Variables
//////////////////

static std::list<TrackEntry> TreadsList;
static std::list<TrackEntry> PuddleList;
static float TrackFadeTime = 5.0f;
static float UserFadeScale = 1.0f;
static AirCullStyle AirCull = FullAirCull;

// FIXME - get these from AnimatedTreads
static const float TreadOutside = 1.4f;
static const float TreadInside = 0.875f;

static const float TreadMiddle = 0.5f * (TreadOutside + TreadInside);
static const float TreadMarkWidth = 0.2f;

static OpenGLGState treadsState;
static OpenGLGState puddleState;
static const char puddleTexture[] = "puddle";

static float TextureHeightOffset = 0.05f;


// Function Prototypes
//////////////////////

//static void bzdbCallback(const std::string& name, void* data);
static void setup();
static void initContext(void* data);
static void drawPuddle(const TrackEntry& te, float lifetime);
static void drawTreads(const TrackEntry& te, float lifetime);
static bool onBuilding(const float pos[3]);


void TrackMarks::init()
{
  clear();
  setup();
  setUserFade(BZDB.eval("userTrackFade"));
  OpenGLGState::registerContextInitializer(initContext, NULL);

  return;
}


void TrackMarks::kill()
{
  clear();
  OpenGLGState::unregisterContextInitializer(initContext, NULL);
  
  return;
}


void TrackMarks::clear()
{
  TreadsList.clear();
  PuddleList.clear();
  return;
}


void TrackMarks::setUserFade(float value)
{
  if (value < 0.0f) {
    value = 0.0f;
  }
  else if (value > 1.0f) {
    value = 1.0f;
  }

  UserFadeScale = value;
  BZDB.setFloat("userTrackFade", value);

  if (UserFadeScale < 1.0f) { // FIXME
    AirCull = NoAirCull;
  } else {
    AirCull = FullAirCull;
  }

  return;
}


float TrackMarks::getUserFade()
{
  return BZDB.eval("userTrackFade");
}


void TrackMarks::setAirCulling(AirCullStyle style)
{
  AirCull = style;
  return;
}


AirCullStyle TrackMarks::getAirCulling()
{
  return AirCull;
}


bool TrackMarks::addMark(const float pos[3], float scale, float angle,
			 int phydrv)
{
  TrackEntry te;
  TrackType type;

  if ((pos[2] <= 0.1f) && BZDB.get(StateDatabase::BZDB_MIRROR) != "none") {
    type = puddle;
  } else {
    type = treads;
    if (scale < 0.01f) {
      return false; // Narrow tanks don't draw treads
    }
  }

  te.startTime = TimeKeeper::getCurrent();
  te.pos[0] = pos[0];
  te.pos[1] = pos[1];
  te.pos[2] = pos[2] + TextureHeightOffset;
  memcpy (te.pos, pos, sizeof(float[3]));
  if (pos[2] > 0.0f) {
    te.pos[2] += TextureHeightOffset;
  }
  te.scale = scale;
  te.angle = angle * (180.0f / M_PI); // in degress, for glRotatef()
  
  // only use the physics driver if it matters
  const PhysicsDriver* driver = PHYDRVMGR.getDriver(phydrv);
  if (driver == NULL) {
    te.phydrv = -1;
  } else {
    const float* v = driver->getVelocity();
    const float av = driver->getAngularVel();
    if ((v[0] == 0.0f) && (v[1] == 0.0f) && (av == 0.0f)) {
      te.phydrv = -1;
    } else {
      te.phydrv = phydrv;
    }
  }

  if (type == puddle) {
    // Puddle track marks
    PuddleList.push_back(te);
  }
  else {
    // Treads track marks
    if ((AirCull & InitAirCull) == 0) {
      // do not cull the air marks
      te.sides = BothTreads;
      TreadsList.push_back(te);
    } 
    else {
      // cull based on track mark support
      if (pos[2] == 0.0f) {
        // no culling required
        te.sides = BothTreads;
        TreadsList.push_back(te);
      }
      else {
        te.sides = 0;
        float markPos[3];
        markPos[2] = pos[2] - TextureHeightOffset;
        const float dx = -sinf(angle) * TreadMiddle;
        const float dy = +cosf(angle) * TreadMiddle;
        // left tread
        markPos[0] = pos[0] + dx;
        markPos[1] = pos[1] + dy;
        if (onBuilding(markPos)) {
          te.sides |= LeftTread;
        }
        // right tread
        markPos[0] = pos[0] - dx;
        markPos[1] = pos[1] - dy;
        if (onBuilding(markPos)) {
          te.sides |= RightTread;
        }
        // add if required
        if (te.sides != 0) {
          TreadsList.push_back(te);
        } else {
          return false;
        }
      }
    }
  }
  
  return true;
}


static bool onBuilding(const float pos[3])
{
  const float dir[3] = {0.0f, 0.0f, -1.0f};
  const float org[3] = {pos[0], pos[1], pos[2] + 0.1f};
  Ray ray(org, dir);
  const ObsList* olist = COLLISIONMGR.rayTest (&ray, 0.5f);
  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    const float top = obs->getPosition()[2] + obs->getHeight();
    if (fabsf(top - pos[2]) < 0.2f) {
      const float hitTime = obs->intersect(ray);
      if (hitTime >= 0.0f) {
        return true;
      }
    }
  }
  return false;
}


static void updateList(std::list<TrackEntry>& list, float dt)
{
  const TimeKeeper cullTime = TimeKeeper::getSunGenesisTime();
  
  std::list<TrackEntry>::iterator it;
  for (it = list.begin(); it != list.end(); it++) {
    TrackEntry& te = *it;

    // update for the Physics Driver
    const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(te.phydrv);
    if (phydrv != NULL) {
    
      const float* v = phydrv->getVelocity();
      te.pos[0] += (v[0] * dt);
      te.pos[1] += (v[1] * dt);

      const float av = phydrv->getAngularVel();
      if (av != 0.0f) {
        const float* ap = phydrv->getAngularPos();
        const float da = (av * dt);
        const float cos_val = cosf(da);
        const float sin_val = sinf(da);
        const float dx = te.pos[0] - ap[0];
        const float dy = te.pos[1] - ap[1];
        te.pos[0] = ap[0] + ((cos_val * dx) - (sin_val * dy));
        te.pos[1] = ap[1] + ((cos_val * dy) + (sin_val * dx));
        te.angle += da * (180.0f / M_PI);
      }

      if ((AirCull & PhyDrvAirCull) != 0) {
        // no need to cull ground marks
        if (te.pos[2] == 0.0f) {
          continue;
        }
        // cull the track marks if they aren't supported
        float markPos[3];
        markPos[2] = te.pos[2] - TextureHeightOffset;
        const float radians = te.angle * (M_PI / 180.0f);
        const float dx = -sinf(radians) * TreadMiddle;
        const float dy = +cosf(radians) * TreadMiddle;
        // left tread
        if ((te.sides & LeftTread) != 0) {
          markPos[0] = te.pos[0] + dx;
          markPos[1] = te.pos[1] + dy;
          if (!onBuilding(markPos)) {
            te.sides &= ~LeftTread;
          }
        }
        // right tread
        if ((te.sides & RightTread) != 0) {
          markPos[0] = te.pos[0] - dx;
          markPos[1] = te.pos[1] - dy;
          if (!onBuilding(markPos)) {
            te.sides &= ~RightTread;
          }
        }
        // cull this node
        if (te.sides == 0) {
          te.startTime = cullTime;
        }
      }
    }
  }

  return;
}


void TrackMarks::update(float dt)
{
  updateList(TreadsList, dt);
  updateList(PuddleList, dt);
  return;
}


void TrackMarks::render()
{
  TrackFadeTime = BZDB.eval(StateDatabase::BZDB_TRACKFADE);
  TrackFadeTime = TrackFadeTime * UserFadeScale;

  if ((TrackFadeTime <= 0.0f) || !BZDBCache::zbuffer) {
    clear();
    return;
  }

  TimeKeeper nowTime = TimeKeeper::getCurrent();

  std::list<TrackEntry>::iterator it;

  // draw treads
  treadsState.setState();
  it = TreadsList.begin();
  while (it != TreadsList.end()) {
    std::list<TrackEntry>::iterator next = it;
    next++;
    TrackEntry& te = *it;
    float timeDiff = nowTime - te.startTime;
    if (timeDiff > TrackFadeTime) {
      TreadsList.erase(it);
      it = next;
      continue;
    }
    it = next;
    drawTreads(te, timeDiff);
  }

  // draw puddles
  puddleState.setState();
  it = PuddleList.begin();
  while (it != PuddleList.end()) {
    std::list<TrackEntry>::iterator next = it;
    next++;
    TrackEntry& te = *it;
    float timeDiff = nowTime - te.startTime;
    if (timeDiff > TrackFadeTime) {
      PuddleList.erase(it);
      it = next;
      continue;
    }
    it = next;
    drawPuddle(te, timeDiff);
  }

  return;
}


static void setup()
{
  TextureManager &tm = TextureManager::instance();
  int puddleTexId = tm.getTextureID(puddleTexture, false);
  
  OpenGLGStateBuilder gb;
  
  gb.reset();
  gb.setShading(GL_FLAT);
  gb.setAlphaFunc(GL_GEQUAL, 0.1f);
  gb.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gb.enableMaterial(false); // no lighting
  gb.setTexture(puddleTexId);
  puddleState = gb.getState();
  
  gb.reset();
  gb.setShading(GL_FLAT);
  gb.setAlphaFunc(GL_GEQUAL, 0.1f);
  gb.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gb.enableMaterial(false); // no lighting
  treadsState = gb.getState();
}
  

static void initContext(void* /*data*/)
{
  setup();
  return;
}


static void drawPuddle(const TrackEntry& te, float lifetime)
{
  const float ratio = (lifetime / TrackFadeTime);
  const float scale = 2.0f * ratio;
  const float offset = te.scale * TreadMiddle;

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f - ratio);

  glPushMatrix();
  {
    glTranslatef(te.pos[0], te.pos[1], te.pos[2]);
    glRotatef(te.angle, 0.0f, 0.0f, 1.0f);
    glTranslatef(0.0f, +offset, 0.0f);
    glScalef(scale, scale, 1.0f);
    glBegin(GL_QUADS);
    {
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(-1.0f, -1.0f, 0.0f);
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(+1.0f, -1.0f, 0.0f);
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f(+1.0f, +1.0f, 0.0f);
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(-1.0f, +1.0f, 0.0f);
    }
    glEnd();
  }
  glPopMatrix();

  // Narrow tanks only need 1 puddle
  if (offset > 0.01f) {
    glPushMatrix();
    {
      glTranslatef(te.pos[0], te.pos[1], te.pos[2]);
      glRotatef(te.angle, 0.0f, 0.0f, 1.0f);
      glTranslatef(0.0f, -offset, 0.0f);
      glScalef(scale, scale, 1.0f);

      glBegin(GL_QUADS);
      {
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(+1.0f, -1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(+1.0f, +1.0f, 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, +1.0f, 0.0f);
      }
      glEnd();
    }
    glPopMatrix();
  }

  return;
}


static void drawTreads(const TrackEntry& te, float lifetime)
{
  const float ratio = (lifetime / TrackFadeTime);

  glColor4f(0.0f, 0.0f, 0.0f, 1.0f - ratio);

  glPushMatrix();
  {
    glTranslatef(te.pos[0], te.pos[1], te.pos[2]);
    glRotatef(te.angle, 0.0f, 0.0f, 1.0f);
    glScalef(1.0f, te.scale, 1.0f);

    const float halfWidth = 0.5f * TreadMarkWidth;

    if ((te.sides & LeftTread) != 0) {
      glRectf(-halfWidth, +TreadInside, +halfWidth, +TreadOutside);
    }
    if ((te.sides & RightTread) != 0) {
      glRectf(-halfWidth, -TreadOutside, +halfWidth, -TreadInside);
    }
  }
  glPopMatrix();

  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

