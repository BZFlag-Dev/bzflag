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
#define _NO_LIST_ID 0xffffffff

// bzflag common headers
#include "common.h"
#include "global.h"

// interface header
#include "TankSceneNode.h"

// system headers
#include <math.h>

// common implementation headers
#include "StateDatabase.h"
#include "BZDBCache.h"

// local implementation headers
#include "ViewFrustum.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

using namespace TankGeometryEnums;

const float MuzzleMaxX = 4.94f;


// parts: body, turret, barrel, left tread, right tread

const int		TankSceneNode::numLOD = 3;
int			TankSceneNode::maxLevel = numLOD;

TankSceneNode::TankSceneNode(const GLfloat pos[3], const GLfloat forward[3]) :
				leftTreadOffset(0.0f), rightTreadOffset(0.0f),
				leftWheelOffset(0.0f), rightWheelOffset(0.0f),
				useDimensions(false), useOverride(false),
				hidden(false), cloaked(false),
				clip(false), inTheCockpit(false),
				tankRenderNode(this),
				shadowRenderNode(this),
				tankSize(TankGeometryEnums::Normal)
{
  // setup style factors (BZDB isn't set up at global init time

			// prepare geometry
  move(pos, forward);
  float length = BZDBCache::tankLength;
  length = 0.5f * (length + MuzzleMaxX);
  const float width = BZDBCache::tankWidth;
  const float height = 0.5f * BZDBCache::tankHeight;

  baseRadius = (length * length) + (width * width) + (height * height);
  setRadius(baseRadius);

  color[3] = 1.0f;
  setColor(1.0f, 1.0f, 1.0f);
  setExplodeFraction(0.0f);

  rebuildExplosion();

  shadowRenderNode.setShadow();
  shadowRenderNode.setTankLOD(LowTankLOD);
}


TankSceneNode::~TankSceneNode()
{
  // do nothing
}


void TankSceneNode::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = a;
  transparent = (color[3] != 1.0f);
}


void TankSceneNode::setColor(const GLfloat* rgba)
{
  color[0] = rgba[0];
  color[1] = rgba[1];
  color[2] = rgba[2];
  color[3] = rgba[3];
  transparent = (color[3] != 1.0f);
}


void TankSceneNode::setDimensions(const float dims[3])
{
  memcpy (dimensions, dims, sizeof(float[3]));
  useDimensions = true;
  return;
}


void TankSceneNode::ignoreDimensions()
{
  useDimensions = false;
  return;
}


void TankSceneNode::setMaterial(const OpenGLMaterial& mat)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setMaterial(mat);
  gstate = builder.getState();
}


void TankSceneNode::setTexture(const int texture)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(texture);
  gstate = builder.getState();
}


void TankSceneNode::move(const GLfloat pos[3], const GLfloat forward[3])
{
  const float rad2deg = (180.0f / M_PI);
  azimuth = rad2deg * atan2f(forward[1], forward[0]);
  elevation = -rad2deg * atan2f(forward[2], hypotf(forward[0], forward[1]));
  setCenter(pos);
  const float maxRadius = 0.5f * (BZDBCache::tankLength + MuzzleMaxX);
  mins[0] = pos[0] - maxRadius;
  mins[1] = pos[1] - maxRadius;
  mins[2] = pos[2];
  maxs[0] = pos[0] + maxRadius;
  maxs[1] = pos[1] + maxRadius;
  maxs[2] = pos[2] + BZDBCache::tankHeight;
}


void TankSceneNode::addTreadOffsets(float left, float right)
{
  const float wheelScale = TankGeometryUtils::getWheelScale();
  const float treadScale = TankGeometryUtils::getTreadScale();
  const float treadTexLen = TankGeometryUtils::getTreadTexLen();

  leftTreadOffset += left * treadScale;
  leftTreadOffset = fmodf (leftTreadOffset, treadTexLen);
  leftWheelOffset += left * wheelScale;
  leftWheelOffset = fmodf (leftWheelOffset, 360.0f);

  rightTreadOffset += right * treadScale;
  rightTreadOffset = fmodf (rightTreadOffset, treadTexLen);
  rightWheelOffset += right * wheelScale;
  rightWheelOffset = fmodf (rightWheelOffset, 360.0f);

  return;
}


void TankSceneNode::notifyStyleChange()
{
  sort = !BZDBCache::zbuffer;
  OpenGLGStateBuilder builder(gstate);
  builder.enableTexture(BZDBCache::texture);
  builder.enableMaterial(BZDBCache::lighting);
  builder.setSmoothing(BZDBCache::smooth);
  if (BZDBCache::blend && transparent) {
    builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    builder.setStipple(1.0f);
  } else {
    builder.resetBlending();
    builder.setStipple(transparent ? 0.5f : 1.0f);
  }
  gstate = builder.getState();

  OpenGLGStateBuilder builder2(lightsGState);
  if (BZDBCache::smooth) {
    builder2.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    builder2.setSmoothing();
  } else {
    builder2.resetBlending();
    builder2.setSmoothing(false);
  }
  lightsGState = builder2.getState();
}


void TankSceneNode::addRenderNodes(SceneRenderer& renderer)
{
  // don't draw hidden tanks.  this is mainly to avoid drawing player's
  // tank when player is using view from tank.  can't simply not include
  // player, though, cos then we wouldn't get the tank's shadow.
  if (hidden || (cloaked && (color[3] == 0.0f))) return;

  // pick level of detail
  const GLfloat* sphere = getSphere();
  const ViewFrustum& view = renderer.getViewFrustum();
  const float size = sphere[3] *
		     (view.getAreaFactor() /getDistance(view.getEye()));

  // set the level of detail
  TankLOD mode = LowTankLOD;
  if (BZDBCache::zbuffer) {
    if ((maxLevel == -1) || ((maxLevel > 2) && (size > 55.0f))) {
      mode = HighTankLOD;
    }
    else if ((maxLevel > 1) && (size > 25.0f)) {
      mode = MedTankLOD;
    }
    else {
      mode = LowTankLOD;
    }
  }
  else {
    // do BSP users a favor
    if ((maxLevel == -1) || ((maxLevel > 0) && (size > 25.0f))) {
      mode = MedTankLOD;
    } else {
      mode = LowTankLOD;
    }
  }
  tankRenderNode.setTankLOD(mode);

  // set the tank's scaling size
  tankRenderNode.setTankSize(tankSize);

  // if drawing in sorted order then decide which order
  if (sort || transparent) {
    const GLfloat* eye = view.getEye();
    GLfloat dx = eye[0] - sphere[0];
    GLfloat dy = eye[1] - sphere[1];
    const float radians = azimuth * M_PI / 180.0f;
    const float cos_val = cosf(radians);
    const float sin_val = sinf(radians);

    const float frontDot = (cos_val * dx) + (sin_val * dy);
    const bool towards = (frontDot > 0.0f);

    const float leftDot = (-sin_val * dx) + (cos_val * dy);
    const bool left = (leftDot > 0.0f);

    const bool above = eye[2] > sphere[2];

    tankRenderNode.sortOrder(above, towards, left);
  }

  renderer.addRenderNode(&tankRenderNode, &gstate);
}


void TankSceneNode::addShadowNodes(SceneRenderer& renderer)
{
  if (cloaked && (color[3] == 0.0f)) {
    return;
  }

  // use HighTankLOD shadows in experimental mode
  if ((TankSceneNode::maxLevel == -1) && BZDBCache::zbuffer) {
    shadowRenderNode.setTankLOD (HighTankLOD);
  } else {
    shadowRenderNode.setTankLOD (LowTankLOD);
  }
  renderer.addShadowNode(&shadowRenderNode);
}


void TankSceneNode::setNormal()
{
  tankSize = Normal;
  setRadius(baseRadius);
}


void TankSceneNode::setObese()
{
  tankSize = Obese;
  float factor = BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
  setRadius(factor*factor*baseRadius);
}


void TankSceneNode::setTiny()
{
  tankSize = Tiny;
  float factor = BZDB.eval(StateDatabase::BZDB_TINYFACTOR);
  setRadius(factor*factor*baseRadius);
}


void TankSceneNode::setNarrow()
{
  tankSize = Narrow;
  setRadius(baseRadius);
}


void TankSceneNode::setThief()
{
  tankSize = Thief;
  float factor = BZDB.eval(StateDatabase::BZDB_THIEFTINYFACTOR);
  setRadius(factor*factor*baseRadius);
}


void TankSceneNode::setExplodeFraction(float t)
{
  explodeFraction = t;
}


void TankSceneNode::setClipPlane(const GLfloat* plane)
{
  if (!plane) {
    clip = false;
  }
  else {
    clip = true;
    clipPlane[0] = GLdouble(plane[0]);
    clipPlane[1] = GLdouble(plane[1]);
    clipPlane[2] = GLdouble(plane[2]);
    clipPlane[3] = GLdouble(plane[3]);
  }
}


void TankSceneNode::setHidden(bool _hidden)
{
  hidden = _hidden;
  cloaked = false;
}


void TankSceneNode::setCloaked(bool _cloaked)
{
  cloaked = _cloaked;
  hidden = false;
}


void TankSceneNode::setMaxLOD(int _maxLevel)
{
  maxLevel = _maxLevel;
}


void TankSceneNode::setInTheCockpit(bool value)
{
  inTheCockpit = value;
}


void TankSceneNode::rebuildExplosion()
{
  // prepare explosion rotations and translations
  for (int i = 0; i < LastTankPart; i++) {
    // pick an unbiased rotation vector
    GLfloat d;
    do {
      spin[i][0] = (float)bzfrand() - 0.5f;
      spin[i][1] = (float)bzfrand() - 0.5f;
      spin[i][2] = (float)bzfrand() - 0.5f;
      d = hypotf(spin[i][0], hypotf(spin[i][1], spin[i][2]));
    } while (d < 0.001f || d > 0.5f);
    spin[i][0] /= d;
    spin[i][1] /= d;
    spin[i][2] /= d;

    // now an angular velocity -- make sure we get at least 2 complete turns
    spin[i][3] = 360.0f * (5.0f * (float)bzfrand() + 2.0f);

    // make arbitrary 2d translation
    vel[i][0] = 80.0f * ((float)bzfrand() - 0.5f);
    vel[i][1] = 80.0f * ((float)bzfrand() - 0.5f);
  }
  return;
}


//
// TankIDLSceneNode
//

TankIDLSceneNode::TankIDLSceneNode(const TankSceneNode* _tank) :
				   tank(_tank),
				   renderNode(this)
{
  static const GLfloat defaultPlane[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
  move(defaultPlane);
  float radius = BZDBCache::tankLength;
  radius = radius * 4.0f;
  setRadius(radius);

  OpenGLGStateBuilder builder(gstate);
  builder.setCulling(GL_NONE);
  builder.setShading(GL_SMOOTH);
  builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gstate = builder.getState();
}


TankIDLSceneNode::~TankIDLSceneNode()
{
  // do nothing
}


void TankIDLSceneNode::move(const GLfloat _plane[4])
{
  plane[0] = _plane[0];
  plane[1] = _plane[1];
  plane[2] = _plane[2];
  plane[3] = _plane[3];

  // compute new sphere
  const GLfloat* s = tank->getSphere();
  setCenter(s[0] + 1.5f * BZDBCache::tankLength * plane[0],
	    s[1] + 1.5f * BZDBCache::tankLength * plane[1],
	    s[2] + 1.5f * BZDBCache::tankLength * plane[2]);
}


void TankIDLSceneNode::notifyStyleChange()
{
  OpenGLGStateBuilder builder(gstate);
  if (BZDBCache::blend) {
    builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    builder.setStipple(1.0f);
  }
  else {
    builder.resetBlending();
    builder.setStipple(0.5f);
  }
  gstate = builder.getState();
}


void TankIDLSceneNode::addRenderNodes(SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, &gstate);
}

//
// TankIDLSceneNode::IDLRenderNode
//

const int TankIDLSceneNode::IDLRenderNode::idlFaces[][5] = {
  // body
  { 4,  1, 0, 4, 5 },
  { 4,  5, 4, 2, 3 },
  { 4,  3, 2, 7, 6 },
  { 4,  6, 7, 0, 1 },
  // turret
  { 3,  12, 15, 10 },
  { 3,  12, 10, 9 },
  { 3,  13, 8, 11 },
  { 3,  13, 11, 14 },
  { 4,  15, 14, 11, 10 },
  { 4,  10, 11, 8, 9 },
  { 4,  9, 8, 13, 12 },
  // barrel
  { 4,  21, 17, 18, 22 },
  { 4,  22, 18, 19, 23 },
  { 4,  23, 19, 16, 20 },
  { 4,  20, 16, 17, 21 },
  { 4,  17, 16, 19, 18 },
  // ltread
  { 4,  29, 26, 25, 28 },
  { 4,  28, 25, 27, 30 },
  { 4,  30, 27, 31, 24 },
  { 4,  24, 31, 26, 29 },
  { 4,  25, 26, 31, 27 },
  // rtread
  { 4,  37, 34, 33, 36 },
  { 4,  36, 33, 35, 38 },
  { 4,  38, 35, 39, 32 },
  { 4,  32, 39, 34, 37 },
  { 4,  37, 36, 38, 32 }
};

const GLfloat		TankIDLSceneNode::IDLRenderNode::idlVertex[][3] = {
  // body
  { 2.430f, 0.877f, 0.000f },
  { 2.430f, -0.877f, 0.000f },
  { -2.835f, 0.877f, 1.238f },
  { -2.835f, -0.877f, 1.238f },
  { 2.575f, 0.877f, 1.111f },
  { 2.575f, -0.877f, 1.111f },
  { -2.229f, -0.877f, 0.000f },
  { -2.229f, 0.877f, 0.000f },
  // turret
  { -1.370f, 0.764f, 2.050f },
  { -1.370f, -0.765f, 2.050f },
  { 1.580f, -0.434f, 1.790f },
  { 1.580f, 0.435f, 1.790f },
  { -0.456f, -1.060f, 1.040f },
  { -0.456f, 1.080f, 1.040f },
  { 1.480f, 0.516f, 1.040f },
  { 1.480f, -0.516f, 1.040f },
  // barrel
  { 4.940f, 0.047f, 1.410f },
  { 4.940f, -0.079f, 1.530f },
  { 4.940f, 0.047f, 1.660f },
  { 4.940f, 0.173f, 1.530f },
  { 1.570f, 0.047f, 1.350f },
  { 1.570f, -0.133f, 1.530f },
  { 1.570f, 0.047f, 1.710f },
  { 1.570f, 0.227f, 1.530f },
  // ltread
  { -2.229f, 0.877f, 0.000f },
  { 2.730f, 1.400f, 1.294f },
  { 2.597f, 1.400f, 0.000f },
  { -2.970f, 1.400f, 1.410f },
  { 2.730f, 0.877f, 1.294f },
  { 2.597f, 0.877f, 0.000f },
  { -2.970f, 0.877f, 1.410f },
  { -2.229f, 1.400f, 0.000f },
  // rtread
  { -2.229f, -1.400f, 0.000f },
  { 2.730f, -0.875f, 1.294f },
  { 2.597f, -0.875f, 0.000f },
  { -2.970f, -0.875f, 1.410f },
  { 2.730f, -1.400f, 1.294f },
  { 2.597f, -1.400f, 0.000f },
  { -2.970f, -1.400f, 1.410f },
  { -2.229f, -0.875f, 0.000f }
};


TankIDLSceneNode::IDLRenderNode::IDLRenderNode(
				const TankIDLSceneNode* _sceneNode) :
				sceneNode(_sceneNode)
{
  // do nothing
}


TankIDLSceneNode::IDLRenderNode::~IDLRenderNode()
{
  // do nothing
}


void TankIDLSceneNode::IDLRenderNode::render()
{
  static const GLfloat innerColor[4] = { 1.0f, 1.0f, 1.0f, 0.75f };
  static const GLfloat outerColor[4] = { 1.0f, 1.0f, 1.0f, 0.0f };

  // compute plane in tank's space
  const GLfloat* sphere = sceneNode->tank->getSphere();
  const GLfloat* _plane = sceneNode->plane;
  const GLfloat azimuth = sceneNode->tank->azimuth;
  const GLfloat ca = cosf(-azimuth * M_PI / 180.0f);
  const GLfloat sa = sinf(-azimuth * M_PI / 180.0f);
  GLfloat plane[4];
  plane[0] = ca * _plane[0] - sa * _plane[1];
  plane[1] = sa * _plane[0] + ca * _plane[1];
  plane[2] = _plane[2];
  plane[3] = (sphere[0] * _plane[0] + sphere[1] * _plane[1] +
	      sphere[2] * _plane[2] + _plane[3]);

  // compute projection point -- one TankLengthy in from plane
  const GLfloat pd = -1.0f * BZDBCache::tankLength - plane[3];
  GLfloat origin[3];
  origin[0] = pd * plane[0];
  origin[1] = pd * plane[1];
  origin[2] = pd * plane[2];

  glPushMatrix();
    glTranslatef(sphere[0], sphere[1], sphere[2]);
    glRotatef(azimuth, 0.0f, 0.0f, 1.0f);

    glBegin(GL_QUADS);
    const int numFaces = countof(idlFaces);
    for (int i = 0; i < numFaces; i++) {
      // get distances from plane
      const int* face = idlFaces[i] + 1;
      const int numVertices = idlFaces[i][0];
      GLfloat d[4];
      int j;
      for (j = 0; j < numVertices; j++)
	d[j] = idlVertex[face[j]][0] * plane[0] +
	       idlVertex[face[j]][1] * plane[1] +
	       idlVertex[face[j]][2] * plane[2] +
	       plane[3];

      // get crossing points
      GLfloat cross[2][3];
      int crossings = 0, k;
      for (j = 0, k = numVertices-1; j < numVertices; k = j++) {
	if ((d[k] < 0.0f && d[j] >= 0.0f) || (d[k] >= 0.0f && d[j] < 0.0f)) {
	  const GLfloat t = d[k] / (d[k] - d[j]);
	  cross[crossings][0] =  (1.0f - t) * idlVertex[face[k]][0] +
					  t * idlVertex[face[j]][0];
	  cross[crossings][1] =  (1.0f - t) * idlVertex[face[k]][1] +
					  t * idlVertex[face[j]][1];
	  cross[crossings][2] =  (1.0f - t) * idlVertex[face[k]][2] +
					  t * idlVertex[face[j]][2];
	  if (++crossings == 2) break;
	}
      }

      // if not enough crossings then skip
      if (crossings != 2) continue;

      // project points out
      GLfloat project[2][3];
      const GLfloat dist = 2.0f + 0.3f * ((float)bzfrand() - 0.5f);
      project[0][0] = origin[0] + dist * (cross[0][0] - origin[0]);
      project[0][1] = origin[1] + dist * (cross[0][1] - origin[1]);
      project[0][2] = origin[2] + dist * (cross[0][2] - origin[2]);
      project[1][0] = origin[0] + dist * (cross[1][0] - origin[0]);
      project[1][1] = origin[1] + dist * (cross[1][1] - origin[1]);
      project[1][2] = origin[2] + dist * (cross[1][2] - origin[2]);

      // draw it
      myColor4fv(innerColor);
      glVertex3fv(cross[0]);
      glVertex3fv(cross[1]);
      myColor4fv(outerColor);
      glVertex3fv(project[1]);
      glVertex3fv(project[0]);
    }
    glEnd();

  glPopMatrix();
}


//
// TankSceneNode::TankRenderNode
//

const GLfloat  // FIXME: setup so these come from TANKGEOMMGR
  TankSceneNode::TankRenderNode::centerOfGravity[LastTankPart][3] = {
  { 0.000f,  0.0f, 1.5f * 0.68f }, // body
  { 3.252f,  0.0f, 1.532f },	   // barrel
  { 0.125f,  0.0f, 2.5f * 0.68f }, // turret
  { 0.000f, +0.7f, 0.5f * 0.68f }, // left case
  { 0.000f, -0.7f, 0.5f * 0.68f }, // right case
  { 0.000f, +0.7f, 0.7f }, // left tread
  { 0.000f, -0.9f, 0.7f }, // right tread
  { -2.25f, +0.9f, 0.7f }, // left wheel0
  { -0.75f, +0.9f, 0.7f }, // left wheel1
  { +0.75f, +0.9f, 0.7f }, // left wheel2
  { +2.25f, +0.9f, 0.7f }, // left wheel3
  { -2.25f, -0.9f, 0.7f }, // right wheel0
  { -0.75f, -0.9f, 0.7f }, // right wheel1
  { +0.75f, -0.9f, 0.7f }, // right wheel2
  { +2.25f, -0.9f, 0.7f }  // right wheel3
};


TankSceneNode::TankRenderNode::TankRenderNode(const TankSceneNode* _sceneNode) :
				sceneNode(_sceneNode), isShadow(false),
				above(false), towards(false)
{
  drawLOD = LowTankLOD;
  drawSize = Normal;
}


TankSceneNode::TankRenderNode::~TankRenderNode()
{
  return;
}


void TankSceneNode::TankRenderNode::setShadow()
{
  isShadow = true;
}


void TankSceneNode::TankRenderNode::sortOrder(
				bool _above, bool _towards, bool _left)
{
  above = _above;
  towards = _towards;
  left = _left;
}


void TankSceneNode::TankRenderNode::setTankLOD(TankLOD lod)
{
  drawLOD = lod;
  return;
}


void TankSceneNode::TankRenderNode::setTankSize(TankSize size)
{
  drawSize = size;
  return;
}


void TankSceneNode::TankRenderNode::render()
{
  if (!sceneNode->useDimensions) {
    drawSize = sceneNode->tankSize;
  } else {
    // for animated resizing effects, setup with the Normal size,
    // and let useDimensions and glScalef() handle the scaling
    drawSize = Normal;
  }

  explodeFraction = sceneNode->explodeFraction;
  isExploding = (explodeFraction != 0.0f);
  color = sceneNode->color;
  alpha = sceneNode->color[3];

  if (!BZDBCache::blend && sceneNode->transparent) {
    myStipple(alpha);
  }

  if (sceneNode->clip) {
    glClipPlane(GL_CLIP_PLANE0, sceneNode->clipPlane);
    glEnable(GL_CLIP_PLANE0);
  }

  const GLfloat* sphere = sceneNode->getSphere();

  // save the MODELVIEW matrix
  glPushMatrix();

  glTranslatef(sphere[0], sphere[1], sphere[2]);
  glRotatef(sceneNode->azimuth, 0.0f, 0.0f, 1.0f);
  glRotatef(sceneNode->elevation, 0.0f, 1.0f, 0.0f);
  if (sceneNode->useDimensions) {
    const float* dims = sceneNode->dimensions;
    glScalef(dims[0], dims[1], dims[2]);
    glEnable(GL_NORMALIZE);
  }

  // disable the dynamic lights, if it might help
  const bool switchLights = BZDBCache::lighting && (drawLOD == HighTankLOD);
  if (switchLights) {
    RENDERER.disableLights(sceneNode->mins, sceneNode->maxs);
  }

  if (!isShadow && (sceneNode->sort || sceneNode->transparent)) {
    // draw is some sorted order
    if (sceneNode->explodeFraction == 0.0f) {
      // normal state
      renderParts();
    }
    else {
      // exploding -- draw back facing stuff first then draw front facing stuff
      OpenGLGState::setCullFace(GL_FRONT);
      renderParts();
      OpenGLGState::setCullFace(GL_BACK);
      renderParts();
    }
  }
  else {
    // any old order is fine.  if exploding then draw both sides.
    if (isExploding) {
      glDisable(GL_CULL_FACE);
    }
    renderPart(Body);
    renderPart(Turret);
    renderPart(Barrel);
    renderPart(LeftCasing);
    renderPart(RightCasing);
    if (drawLOD == HighTankLOD) {
      for (int i = 0; i < 4; i++) {
	if (isShadow && ((i == 1) || (i == 2)) && !isExploding) {
	  continue;
	}
	renderPart((TankPart)(LeftWheel0 + i));
	renderPart((TankPart)(RightWheel0 + i));
      }
      renderPart(LeftTread);
      renderPart(RightTread);
    }
    if (isExploding) {
      glEnable(GL_CULL_FACE);
    }
  }

  // re-enable the dynamic lights
  if (switchLights) {
    RENDERER.reenableLights();
  }

  if (sceneNode->useDimensions) {
    glDisable(GL_NORMALIZE);
  }

  // restore the MODELVIEW matrix
  glPopMatrix();

  if (!isExploding && !isShadow) {
    // FIXME -- add flare lights using addFlareLight().
    // pass light position in world space.
  }

  glShadeModel(GL_FLAT);
  if (!BZDBCache::blend && sceneNode->transparent) myStipple(0.5f);
  if (sceneNode->clip) glDisable(GL_CLIP_PLANE0);
}


void TankSceneNode::TankRenderNode::renderTopParts()
{
  if (towards) {
    renderPart(Turret);
    renderPart(Barrel);
  }
  else {
    renderPart(Barrel);
    renderPart(Turret);
  }
  return;
}


void TankSceneNode::TankRenderNode::renderLeftParts()
{
  renderPart(LeftCasing);
  if (drawLOD == HighTankLOD) {
    for (int i = 0; i < 4; i++) {
      // don't need the middle two wheels for shadows
      if (isShadow && ((i == 1) || (i == 2)) && !isExploding) {
	continue;
      }
      renderPart((TankPart)(LeftWheel0 + i));
    }
    renderPart(LeftTread);
  }
  return;
}


void TankSceneNode::TankRenderNode::renderRightParts()
{
  renderPart(RightCasing);
  if (drawLOD == HighTankLOD) {
    for (int i = 0; i < 4; i++) {
      // don't need the middle two wheels for shadows
      if (isShadow && ((i == 1) || (i == 2)) && !isExploding) {
	continue;
      }
      renderPart((TankPart)(RightWheel0 + i));
    }
    renderPart(RightTread);
  }
  return;
}


void TankSceneNode::TankRenderNode::renderParts()
{
  if (!above) {
    renderTopParts();
  }

  if (left) {
    renderRightParts();
  } else {
    renderLeftParts();
  }

  if (!sceneNode->inTheCockpit) {
    renderPart(Body);
  }

  if (left) {
    renderLeftParts();
  } else {
    renderRightParts();
  }

  if (sceneNode->inTheCockpit) {
    renderPart(Body);
  }

  if (above) {
    renderTopParts();
  }

  return;
}


void TankSceneNode::TankRenderNode::renderPart(TankPart part)
{
  // apply explosion transform
  if (isExploding) {
    glPushMatrix();
    const float* vel = sceneNode->vel[part];
    const float* spin = sceneNode->spin[part];
    const float* cog = centerOfGravity[part];
    glTranslatef(cog[0] + (explodeFraction * vel[0]),
		 cog[1] + (explodeFraction * vel[1]),
		 cog[2]);
    glRotatef(spin[3] * explodeFraction, spin[0], spin[1], spin[2]);
    glTranslatef(-cog[0], -cog[1], -cog[2]);
  }

  // setup the animation texture matrix
  bool usingTexMat = false;
  if (!isShadow && (drawLOD == HighTankLOD) && (part >= BasicTankParts)) {
    usingTexMat = setupTextureMatrix(part);
  }

  // set color
  if (!isShadow) {
    setupPartColor(part);
  }

  // get the list
  GLuint list;
  TankShadow shadow = isShadow ? ShadowOn : ShadowOff;
  list = TankGeometryMgr::getPartList(shadow, part, drawSize, drawLOD);

  // draw the part
  glCallList(list);

  if (part == Turret && !isExploding && !isShadow) {
    renderLights();
  }

  // restore texture transform
  if (usingTexMat) {
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
  }

  // restore modelview transform
  if (isExploding) {
    glPopMatrix();
  }
}


void TankSceneNode::TankRenderNode::setupPartColor(TankPart part)
{
  const GLfloat white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  const GLfloat* clr = color;

  // do not use color modulation with tank textures
  if (BZDBCache::texture) {
    clr = white;
  }

  switch (part) {
    case Body: {
      myColor4f(clr[0], clr[1], clr[2], alpha);
      break;
    }
    case Barrel: {
      myColor4f(0.25f, 0.25f, 0.25f, alpha);
      break;
    }
    case Turret: {
      myColor4f(0.9f * clr[0], 0.9f * clr[1], 0.9f * clr[2], alpha);
      break;
    }
    case LeftCasing:
    case RightCasing: {
      myColor4f(0.7f * clr[0], 0.7f * clr[1], 0.7f * clr[2], alpha);
      break;
    }
    case LeftTread:
    case RightTread: {
      myColor4f(0.3f * clr[0], 0.3f * clr[1], 0.3f * clr[2], alpha);
      break;
    }
    case LeftWheel0:
    case LeftWheel1:
    case LeftWheel2:
    case LeftWheel3:
    case RightWheel0:
    case RightWheel1:
    case RightWheel2:
    case RightWheel3: {
      myColor4f(0.4f * clr[0], 0.4f * clr[1], 0.4f * clr[2], alpha);
      break;
    }
    default: { // avoid warnings about unused enumerated values
      break;
    }
  }
  return;
}


bool TankSceneNode::TankRenderNode::setupTextureMatrix(TankPart part)
{
  bool usingTexMat = true;

  switch (part) {
    case LeftTread: {
      glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glTranslatef(sceneNode->leftTreadOffset, 0.0f, 0.0f);
      glMatrixMode(GL_MODELVIEW);
      break;
    }
    case RightTread: {
      glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glTranslatef(sceneNode->rightTreadOffset, 0.0f, 0.0f);
      glMatrixMode(GL_MODELVIEW);
      break;
    }
    case LeftWheel0:
    case LeftWheel1:
    case LeftWheel2:
    case LeftWheel3: {
      glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glTranslatef(+0.5f, +0.5f, 0.0f);
      glRotatef(sceneNode->leftWheelOffset, 0.0f, 0.0f, 1.0f);
      glTranslatef(-0.5f, -0.5f, 0.0f);
      glMatrixMode(GL_MODELVIEW);
      break;
    }
    case RightWheel0:
    case RightWheel1:
    case RightWheel2:
    case RightWheel3: {
      glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glTranslatef(+0.5f, +0.5f, 0.0f);
      glRotatef(sceneNode->rightWheelOffset, 0.0f, 0.0f, 1.0f);
      glTranslatef(-0.5f, -0.5f, 0.0f);
      glMatrixMode(GL_MODELVIEW);
      break;
    }
    default: {
      usingTexMat = false;
      break;
    }
  }

  return usingTexMat;
}


void TankSceneNode::TankRenderNode::renderLights()
{
  static const GLfloat lights[3][6] = {
    { 1.0f, 1.0f, 1.0f, -1.53f,  0.00f, 2.1f },
    { 1.0f, 0.0f, 0.0f,  0.10f,  0.75f, 2.1f },
    { 0.0f, 1.0f, 0.0f,  0.10f, -0.75f, 2.1f }
  };
  sceneNode->lightsGState.setState();
  glPointSize(2.0f);

  glBegin(GL_POINTS);
  {
    const float* scale = TankGeometryMgr::getScaleFactor(sceneNode->tankSize);
    myColor3fv(lights[0]);
    glVertex3f(lights[0][3] * scale[0],
	       lights[0][4] * scale[1],
	       lights[0][5] * scale[2]);
    myColor3fv(lights[1]);
    glVertex3f(lights[1][3] * scale[0],
	       lights[1][4] * scale[1],
	       lights[1][5] * scale[2]);
    myColor3fv(lights[2]);
    glVertex3f(lights[2][3] * scale[0],
	       lights[2][4] * scale[1],
	       lights[2][5] * scale[2]);
  }
  glEnd();

  glPointSize(1.0f);
  sceneNode->gstate.setState();
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

