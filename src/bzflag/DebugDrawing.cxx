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


#include "common.h"

// interface header
#include "DebugDrawing.h"

// common headers
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "SceneRenderer.h"
#include "BZDBCache.h"
#include "LocalPlayer.h"
#include "LinkManager.h"
#include "MeshFace.h"
#include "Flag.h"
#include "Roster.h"
#include "GameTime.h"


//============================================================================//
//
//  drawTankDebug()
//

static void drawTankHitZone(const Player* tank)
{
  if ((tank == NULL) || (tank->getTeam() == ObserverTeam)) {
    return;
  }

  const bool isNarrow = (tank->getFlag() == Flags::Narrow);

  static BZDB_bool tankShotSpherical("_tankShotSpherical");
  static BZDB_fvec3 shotProxim(BZDBNAMES.TANKSHOTPROXIMITY);
  const fvec3& sp = shotProxim;

  fvec3        pos   = tank->getPosition();
  fvec3        dims  = tank->getDimensions();
  const float  angle = tank->getAngle();
  const fvec4& color = tank->getColor();

  if (tankShotSpherical && !isNarrow) {
    GLUquadric* quad = gluNewQuadric();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor4f(color.r, color.g, color.b, 0.8f);
    glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z + (dims.z * 0.5f));
    glRotatef(angle * RAD2DEGf, 0.0f, 0.0f, 1.0f);
    glScalef(sp.x, sp.y, sp.z);
    gluSphere(quad, 1.0f, 16, 8);
    glPopMatrix();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    gluDeleteQuadric(quad);
    return;
  }

  if (!isnan(sp.x)) {
    dims += sp;
  }

  if (isNarrow) {
    static BZDB_float shotRadius(BZDBNAMES.SHOTRADIUS);
    dims.y = shotRadius;
  }

  glColor4f(color.r, color.g, color.b, 0.8f);

  glPushMatrix();
  glTranslatef(pos.x, pos.y, pos.z);
  glRotatef(angle * RAD2DEGf, 0.0f, 0.0f, 1.0f);

  const fvec3 corners[8] = {
    fvec3(-dims.x, -dims.y, 0.0f),
    fvec3(+dims.x, -dims.y, 0.0f),
    fvec3(+dims.x, +dims.y, 0.0f),
    fvec3(-dims.x, +dims.y, 0.0f),
    fvec3(-dims.x, -dims.y, dims.z),
    fvec3(+dims.x, -dims.y, dims.z),
    fvec3(+dims.x, +dims.y, dims.z),
    fvec3(-dims.x, +dims.y, dims.z)
  };

  glBegin(GL_LINE_LOOP); {
    glVertex3fv(corners[0]);
    glVertex3fv(corners[1]);
    glVertex3fv(corners[2]);
    glVertex3fv(corners[3]);
  } glEnd();

  glBegin(GL_LINE_LOOP); {
    glVertex3fv(corners[4]);
    glVertex3fv(corners[5]);
    glVertex3fv(corners[6]);
    glVertex3fv(corners[7]);
  } glEnd();

  glBegin(GL_LINES); {
    glVertex3fv(corners[0]); glVertex3fv(corners[4]);
    glVertex3fv(corners[1]); glVertex3fv(corners[5]);
    glVertex3fv(corners[2]); glVertex3fv(corners[6]);
    glVertex3fv(corners[3]); glVertex3fv(corners[7]);
  } glEnd();

  glPopMatrix();
}


void DebugDrawing::drawTanks()
{
  static BZDB_bool bzdbDrawTanks("debugTankDraw");
  if (!bzdbDrawTanks) {
    return;
  }
  if (BZDB.isTrue("_forbidDebug")) {
    LocalPlayer* myTank = LocalPlayer::getMyTank();
    if (!myTank || (myTank->getTeam() != ObserverTeam)) {
      return; // debug forbidden, not an observer, no boxes for you!
    }
  }

  OpenGLGState::resetState();

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glShadeModel(GL_SMOOTH);
  glDisable(GL_CULL_FACE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);
  glLineWidth(2.49f);

  drawTankHitZone(LocalPlayer::getMyTank());
  for (int i = 0; i < curMaxPlayers; i++) {
    drawTankHitZone(remotePlayers[i]);
  }

  glLineWidth(1.0f);
  glDisable(GL_LINE_SMOOTH);
  glDisable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_CULL_FACE);
  glShadeModel(GL_FLAT);
  glPopAttrib();
}


//============================================================================//
//
//  drawLinkDebug(), and friends
//

static void srcSTPPoint(const fvec3& center, const fvec3& dir, float scale)
{
  scale = (scale == 0.0f) ? 1.0f : (1.0f / scale);
  glVertex3fv(center + (scale * dir));
}


static void srcSTPLine(const fvec3& center, const fvec3& dir, float scale)
{
  scale = (scale == 0.0f) ? 1.0f : (1.0f / scale);
  glVertex3fv(center);
  glVertex3fv(center + (scale * dir));
}


static void dstSTPPoint(const fvec3& center, const fvec3& dir, float scale)
{
  scale = (scale == 0.0f) ? 1.0f : scale;
  glVertex3fv(center + (scale * dir));
}


static void dstSTPLine(const fvec3& center, const fvec3& dir, float scale)
{
  scale = (scale == 0.0f) ? 1.0f : scale;
  glVertex3fv(center);
  glVertex3fv(center + (scale * dir));
}


void DebugDrawing::drawLinks()
{
  static BZDB_string bzdbStr("debugLinkDraw");
  const std::string str = bzdbStr;
  if (str.empty() || (str == "0")) {
    return;
  }
  if (BZDB.isTrue("_forbidDebug")) {
    return;
  }

  const std::string::size_type npos = std::string::npos;
  const bool drawSrc = (str.find('s') != npos) || (str == "1");
  const bool drawDst = (str.find('d') != npos) || (str == "1");
  const bool drawCon = (str.find('c') != npos) || (str == "1");
  if (!drawSrc && !drawDst && !drawCon) {
    return;
  }

  const float alpha = 0.3f;
  const fvec4 colors[3] = {
    fvec4(0.8f, 0.0f, 0.0f, alpha), // red
    fvec4(0.0f, 0.6f, 0.0f, alpha), // green
    fvec4(0.0f, 0.0f, 1.0f, alpha)  // blue
  };
  const fvec4 nrmlColor(0.5f, 0.5f, 0.5f, alpha);

  OpenGLGState::resetState();

  const bool blackFog = RENDERER.isFogActive();
  if (blackFog) {
    glFogfv(GL_FOG_COLOR, fvec4(0.0f, 0.0f, 0.0f, 0.0f));
  }

  glShadeModel(GL_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POINT_SMOOTH);
  glPointSize(4.0f);
  glLineWidth(2.49f);

  if (drawSrc) {
    const LinkManager::FaceSet& linkSrcs = linkManager.getLinkSrcSet();
    LinkManager::FaceSet::const_iterator srcIt;
    for (srcIt = linkSrcs.begin(); srcIt != linkSrcs.end(); ++srcIt) {
      const MeshFace* face = *srcIt;
      const MeshFace::SpecialData* sd = face->getSpecialData();
      const MeshFace::LinkGeometry& geo = sd->linkSrcGeo;
      const fvec3& normal = face->getPlane().xyz();
      glBegin(GL_LINES);
      glColor4fv(nrmlColor); srcSTPLine(geo.center, normal,   1.0f);
      glColor4fv(colors[0]); srcSTPLine(geo.center, geo.sDir, geo.sScale);
      glColor4fv(colors[1]); srcSTPLine(geo.center, geo.tDir, geo.tScale);
      glColor4fv(colors[2]); srcSTPLine(geo.center, geo.pDir, geo.pScale);
      glEnd();
      glBegin(GL_POINTS);
      glColor4fv(nrmlColor); srcSTPPoint(geo.center, normal,   1.0f);
      glColor4fv(colors[0]); srcSTPPoint(geo.center, geo.sDir, geo.sScale);
      glColor4fv(colors[1]); srcSTPPoint(geo.center, geo.tDir, geo.tScale);
      glColor4fv(colors[2]); srcSTPPoint(geo.center, geo.pDir, geo.pScale);
      glColor4fv(colors[0]); glVertex3fv(geo.center);
      glEnd();
    }
  }

  if (drawDst) {
    const LinkManager::FaceSet& linkDsts = linkManager.getLinkSrcSet();
    LinkManager::FaceSet::const_iterator dstIt;
    for (dstIt = linkDsts.begin(); dstIt != linkDsts.end(); ++dstIt) {
      const MeshFace* face = *dstIt;
      const MeshFace::SpecialData* sd = face->getSpecialData();
      const MeshFace::LinkGeometry& geo = sd->linkDstGeo;
      const fvec3& normal = face->getPlane().xyz();
      glBegin(GL_LINES);
      glColor4fv(nrmlColor); dstSTPLine(geo.center, normal,   1.0f);
      glColor4fv(colors[0]); dstSTPLine(geo.center, geo.sDir, geo.sScale);
      glColor4fv(colors[1]); dstSTPLine(geo.center, geo.tDir, geo.tScale);
      glColor4fv(colors[2]); dstSTPLine(geo.center, geo.pDir, geo.pScale);
      glEnd();
      glBegin(GL_POINTS);
      glColor4fv(nrmlColor); dstSTPPoint(geo.center, normal,   1.0f);
      glColor4fv(colors[0]); dstSTPPoint(geo.center, geo.sDir, geo.sScale);
      glColor4fv(colors[1]); dstSTPPoint(geo.center, geo.tDir, geo.tScale);
      glColor4fv(colors[2]); dstSTPPoint(geo.center, geo.pDir, geo.pScale);
      glColor4fv(colors[0]); glVertex3fv(geo.center);
      glEnd();
    }
  }

  glPointSize(10.0f);

  if (drawCon) {
    // load a basic 1D texture
    const float texData[8] = {
      1.0f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f
    };
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, 0);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_ALPHA, countof(texData), 0,
                 GL_ALPHA, GL_FLOAT, texData);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    const float texDist = 10.0f;
    const float texScale = (1.0f / texDist);
    const float texPeriod = 0.5f;
    const float phase = (float)fmod(GameTime::getStepTime(), (double)texPeriod);
    const float offset = phase / texPeriod;

    glBegin(GL_LINES);
    const LinkManager::LinkMap& linkMap = linkManager.getLinkMap();
    LinkManager::LinkMap::const_iterator mapIt;
    for (mapIt = linkMap.begin(); mapIt != linkMap.end(); ++mapIt) {
      const MeshFace* src = mapIt->first;
      std::set<const MeshFace*> doneFaces;
      const LinkManager::IntVec& dstIDs = mapIt->second.dstIDs;
      for (size_t d = 0; d < dstIDs.size(); d++) {
        const MeshFace* dst = linkManager.getLinkDstFace(dstIDs[d]);
        if (doneFaces.find(dst) == doneFaces.end()) {
          doneFaces.insert(dst);
          const fvec3 srcPos = src->calcCenter();
          const fvec3 dstPos = dst->calcCenter();
          const float len = (srcPos - dstPos).length();
          const float txcd0 = offset;
          const float txcd1 = offset + (len * texScale);
          glColor4fv(colors[0]); glTexCoord1f(txcd0); glVertex3fv(srcPos);
          glColor4fv(colors[1]); glTexCoord1f(txcd1); glVertex3fv(dstPos);
          if (src == dst) {
            glEnd();
            glDisable(GL_TEXTURE_1D);
            glBegin(GL_POINTS);
            const fvec4 yellow(1.0f, 1.0f, 0.0f, alpha);
            glColor4fv(yellow); glVertex3fv(src->calcCenter());
            glEnd();
            glEnable(GL_TEXTURE_1D);
            glBegin(GL_LINES);
          }
        }
      }
    }
    glEnd();

    glDisable(GL_TEXTURE_1D);
  }

  glPointSize(1.0f);
  glLineWidth(1.0f);
  glDisable(GL_POINT_SMOOTH);
  glDisable(GL_LINE_SMOOTH);
  glDisable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glShadeModel(GL_FLAT);

  if (blackFog) {
    glFogfv(GL_FOG_COLOR, RENDERER.getFogColor());
  }
}





// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
