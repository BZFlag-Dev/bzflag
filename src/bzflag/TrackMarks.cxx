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
#include "PhysicsDriver.h"
#include "bzfgl.h"
#include "TextureManager.h"
#include "SceneRenderer.h"
#include "OpenGLGState.h"
#include "OpenGLMaterial.h"


using namespace TrackMarks;

typedef struct {
  TrackType type;
  float pos[3];
  float angle;
  float scale;
  int phydrv;
  TimeKeeper startTime;
} TrackEntry;


// Local Variables
//////////////////

static std::list<TrackEntry> TrackList;
static int TrackTexture = -1;
static float TrackFadeTime = 5.0f;
//static float TrackWidth = 1.0f;


// FIXME - get these from AnimatedTreads
static const float TreadOutside = 1.4f;
static const float TreadInside = 0.875f;

static const float TreadMiddle = 0.5f * (TreadOutside + TreadInside);
static const float TreadMarkWidth = 0.2f;

static OpenGLGState treadsState;
static OpenGLGState puddleState;
static const std::string puddleTexture = "puddle";

static float TextureHeightOffset = 0.1f;


// Function Prototypes
//////////////////////

//static void bzdbCallback(const std::string& name, void* data);
static void drawPuddle(const TrackEntry& te, float lifetime);
static void drawTreads(const TrackEntry& te, float lifetime);


void TrackMarks::init()
{
  clear();
  
  TextureManager &tm = TextureManager::instance();
  TrackTexture = tm.getTextureID(puddleTexture.c_str(), false);
  
  OpenGLGStateBuilder gb;
  
  const float grey[4] = {0.5f, 0.5f, 0.5f, 1.0f};
  OpenGLMaterial puddleMaterial(grey, grey, 0.5f);
  gb.reset();
  gb.setShading(GL_FLAT);
  gb.setAlphaFunc(GL_GEQUAL, 0.1f);
  gb.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gb.setMaterial(puddleMaterial);
  gb.setTexture(TrackTexture);
  puddleState = gb.getState();
  
  const float black[4] = {0.0f, 0.0f, 0.f, 1.0f};
  OpenGLMaterial treadsMaterial(black, black, 0.0f);
  gb.reset();
  gb.setShading(GL_FLAT);
  gb.setAlphaFunc(GL_GEQUAL, 0.1f);
  gb.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gb.setMaterial(treadsMaterial);
  treadsState = gb.getState();
  
//  BZDB.addCallback("_trackMarkTime", bzdbCallback, NULL);
  
  return;
}


void TrackMarks::kill()
{
  clear();
  
//  BZDB.removeCallback("_trackMarkTime", bzdbCallback, NULL);
  
  return;
}


//static void bzdbCallback(const std::string& /*name*/, void* /*data*/)
//{
//  
//  TrackFadeTime = BZDB.eval("_trackFade");
//
//  return;
//}


void TrackMarks::clear()
{
  TrackList.clear();
  return;
}


bool TrackMarks::addMark(const float pos[3], float scale, float angle,
                         int phydrv)
{
  TrackEntry te;
  
  if ((pos[2] <= 0.1f) && BZDB.get(StateDatabase::BZDB_MIRROR) != "none") {
    te.type = puddle;
  } else {
    te.type = treads;
  }
  te.startTime = TimeKeeper::getCurrent();
  memcpy (te.pos, pos, sizeof(float[3]));
  if (pos[2] > 0.0f) {
    te.pos[2] += TextureHeightOffset;
  }
  te.scale = scale;
  te.angle = angle * (180.0f / M_PI) ;
  te.phydrv = phydrv;
  TrackList.push_back(te);
  return true;
}


void TrackMarks::update(float dt)
{
  std::list<TrackEntry>::iterator it;
  for (it = TrackList.begin(); it != TrackList.end(); it++) {
    TrackEntry& te = *it;
    // update for the Physics Driver
    if (te.phydrv >= 0) {
      const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(te.phydrv);
      if (phydrv != NULL) {
        const float* v = phydrv->getVelocity();
        te.pos[0] += (v[0] * dt);
        te.pos[1] += (v[1] * dt);
        
        const float av = phydrv->getAngularVel();
        if (av != 0.0f) {
          const float* ap = phydrv->getAngularPos();
          const float da = ((av * 2.0f * M_PI) * dt);
          const float cos_val = cosf(da);
          const float sin_val = sinf(da);
          const float dx = te.pos[0] - ap[0];
          const float dy = te.pos[1] - ap[1];
          te.pos[0] = ap[0] + ((cos_val * dx) - (sin_val * dy));
          te.pos[1] = ap[1] + ((cos_val * dy) + (sin_val * dx));
          te.angle += da * (180.0f / M_PI);
        }
      }
    }
  }
  
  return;
}


void TrackMarks::render()
{
  TrackFadeTime = BZDB.eval(StateDatabase::BZDB_TRACKFADE);
  
  if ((TrackFadeTime <= 0.0f) || 
      (RENDERER.useQuality() < 3) || !BZDBCache::zbuffer) {
    clear();
    return;
  }

  TimeKeeper nowTime = TimeKeeper::getCurrent();

  std::list<TrackEntry>::iterator it = TrackList.begin();
  
  glDisable(GL_LIGHTING);
  
  while (it != TrackList.end()) {  
    std::list<TrackEntry>::iterator next = it;
    next++;
    TrackEntry& te = *it;
    float timeDiff = nowTime - te.startTime;
    if (timeDiff > TrackFadeTime) {
      TrackList.erase(it);
      it = next;
      continue;
    }
    it = next;
    
    if (te.type == treads) {
      drawTreads(te, timeDiff);
    } else {
      drawPuddle(te, timeDiff);
    }
  }
  
  glEnable(GL_LIGHTING);
  
  return;
}


static void drawPuddle(const TrackEntry& te, float lifetime)
{
  const float ratio = (lifetime / TrackFadeTime);
  const float scale = 2.0f * ratio;
  const float offset = te.scale * TreadMiddle;
    
  puddleState.setState();
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f - ratio);

  glPushMatrix();
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
  glPopMatrix();

  glPushMatrix();
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
  glPopMatrix();

  return;
}


static void drawTreads(const TrackEntry& te, float lifetime)
{
  const float ratio = (lifetime / TrackFadeTime);
  
  treadsState.setState();
  glColor4f(0.0f, 0.0f, 0.0f, 1.0f - ratio);
  
  glPushMatrix();
  
  glTranslatef(te.pos[0], te.pos[1], te.pos[2]);
  glRotatef(te.angle, 0.0f, 0.0f, 1.0f);
  glScalef(1.0f, te.scale, 1.0f);
  
  const float halfWidth = 0.5f * TreadMarkWidth;

  glBegin(GL_QUADS);
  {
    glVertex3f(-halfWidth, +TreadOutside, 0.0f);
    glVertex3f(-halfWidth, +TreadInside, 0.0f);
    glVertex3f(+halfWidth, +TreadInside, 0.0f);
    glVertex3f(+halfWidth, +TreadOutside, 0.0f);
    glVertex3f(-halfWidth, -TreadInside, 0.0f);
    glVertex3f(-halfWidth, -TreadOutside, 0.0f);
    glVertex3f(+halfWidth, -TreadOutside, 0.0f);
    glVertex3f(+halfWidth, -TreadInside, 0.0f);
  } 
  glEnd();
  
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

