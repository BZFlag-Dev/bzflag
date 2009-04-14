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
#define _NO_LIST_ID 0xffffffff

// bzflag common headers
#include "common.h"

// interface header
#include "TankSceneNode.h"

// system headers
#include <math.h>
#include <string.h>

// common headers
#include "bzfgl.h"
#include "global.h"
#include "StateDatabase.h"
#include "SceneRenderer.h"
#include "TextureManager.h"
#include "BZDBCache.h"

// local headers
#include "ViewFrustum.h"

using namespace TankGeometryEnums;

static const float MuzzleMaxX = 4.94f;
static const float maxExplosionVel = 40.0f;
static const float vertExplosionRatio = 0.5f;


// parts: body, turret, barrel, left tread, right tread

const int		TankSceneNode::numLOD = 3;
int			TankSceneNode::maxLevel = numLOD;

TankSceneNode::TankSceneNode(const fvec3& pos, const fvec3& forward)
: leftTreadOffset(0.0f)
, rightTreadOffset(0.0f)
, leftWheelOffset(0.0f)
, rightWheelOffset(0.0f)
, useDimensions(false)
, useOverride(false)
, onlyShadows(false)
, clip(false)
, inTheCockpit(false)
, tankRenderNode(this)
, treadsRenderNode(this)
, shadowRenderNode(this)
, tankSize(TankGeometryEnums::Normal)
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

  color.a = 1.0f;
  setColor(1.0f, 1.0f, 1.0f);
  setExplodeFraction(0.0f);
  setJumpJets(0.0f);

  rebuildExplosion();

  shadowRenderNode.setShadow();
  shadowRenderNode.setTankLOD(LowTankLOD);

  jumpJetsRealLight.setAttenuation(0, 0.05f);
  jumpJetsRealLight.setAttenuation(1, 0.0f);
  jumpJetsRealLight.setAttenuation(2, 0.03f);
  jumpJetsRealLight.setOnlyReal(true);
  for (int i = 0; i < 4; i++) {
    jumpJetsGroundLights[i].setAttenuation(0, 0.05f);
    jumpJetsGroundLights[i].setAttenuation(1, 0.0f);
    jumpJetsGroundLights[i].setAttenuation(2, 0.03f);
    jumpJetsGroundLights[i].setOnlyGround(true);
  }

  return;
}


TankSceneNode::~TankSceneNode()
{
  // do nothing
}


void TankSceneNode::setColor(float r, float g, float b, float a)
{
  color.r = r;
  color.g = g;
  color.b = b;
  color.a = a;
  transparent = (color.a != 1.0f);
}


void TankSceneNode::setColor(const fvec4& rgba)
{
  color = rgba;
  transparent = (color.a != 1.0f);
}


void TankSceneNode::setMaterial(const OpenGLMaterial& mat)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setMaterial(mat, RENDERER.useQuality() > _LOW_QUALITY);
  gstate = builder.getState();
}


void TankSceneNode::setTexture(const int texture)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(texture);
  gstate = builder.getState();
}


void TankSceneNode::setJumpJetsTexture(const int texture)
{
  OpenGLGStateBuilder builder(jumpJetsGState);
  builder.setTexture(texture);
  jumpJetsGState = builder.getState();
}


void TankSceneNode::move(const fvec3& pos, const fvec3& forward)
{
  azimuth   =  RAD2DEG * atan2f(forward.y, forward.x);
  elevation = -RAD2DEG * atan2f(forward.z, forward.xy().length());
  setCenter(pos);

  // setup the extents
  const float maxRadius = 0.5f * (BZDBCache::tankLength + MuzzleMaxX);
  extents.mins.x = pos.x - maxRadius;
  extents.mins.y = pos.y - maxRadius;
  extents.mins.z = pos.z;
  extents.maxs.x = pos.x + maxRadius;
  extents.maxs.y = pos.y + maxRadius;
  extents.maxs.z = pos.z + BZDBCache::tankHeight;
  return;
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

  builder.setTexture(TextureManager::instance().getTextureID("treads"));
  treadState = builder.getState();

  OpenGLGStateBuilder builder2(lightsGState);
  if (BZDBCache::smooth) {
    builder2.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    builder2.setSmoothing();
  } else {
    builder2.resetBlending();
    builder2.setSmoothing(false);
  }
  lightsGState = builder2.getState();

  OpenGLGStateBuilder builder3(jumpJetsGState);
  builder3.setCulling(GL_NONE);
  builder3.setBlending(GL_SRC_ALPHA, GL_ONE);
  jumpJetsGState = builder3.getState();
}


void TankSceneNode::addRenderNodes(SceneRenderer& renderer)
{
  // don't draw onlyShadows tanks.  this is mainly to avoid drawing player's
  // tank when player is using view from tank.  can't simply not include
  // player, though, cos then we wouldn't get the tank's shadow.
  if (onlyShadows) {
    return;
  }

  // pick level of detail
  const fvec4& mySphere = getSphere();
  const ViewFrustum& view = renderer.getViewFrustum();
  const float size = mySphere.w *
		     (view.getAreaFactor() / getDistanceSq(view.getEye()));

  // set the level of detail
  TankLOD mode = LowTankLOD;
  if ((maxLevel == -1) || ((maxLevel > 2) && (size > 55.0f))) {
    mode = HighTankLOD;
  }
  else if ((maxLevel > 1) && (size > 25.0f)) {
    mode = MedTankLOD;
  }
  else {
    mode = LowTankLOD;
  }
  tankRenderNode.setTankLOD(mode);
  treadsRenderNode.setTankLOD(mode);

  // set the tank's scaling size
  tankRenderNode.setTankSize(tankSize);
  treadsRenderNode.setTankSize(tankSize);

  bool narrow = false;
  if ((tankSize == Narrow) &&
      (!useDimensions || (dimensions.y < 0.01f)) &&
      BZDBCache::animatedTreads && BZDBCache::zbuffer) {
    narrow = true;
  }
  tankRenderNode.setNarrowWithDepth(narrow);
  treadsRenderNode.setNarrowWithDepth(narrow);

  // if drawing in sorted order then decide which order
  if (sort || transparent || narrow) {
    const fvec3& eye = view.getEye();
    float dx = eye.x - mySphere.x;
    float dy = eye.y - mySphere.y;
    const float radians = (float)(azimuth * DEG2RAD);
    const float cos_val = cosf(radians);
    const float sin_val = sinf(radians);

    const float frontDot = (cos_val * dx) + (sin_val * dy);
    const bool towards = (frontDot > 0.0f);

    const float leftDot = (-sin_val * dx) + (cos_val * dy);
    const bool left = (leftDot > 0.0f);

    const bool above = (eye.z > mySphere.z);

    tankRenderNode.sortOrder(above, towards, left);
    treadsRenderNode.sortOrder(above, towards, left);
  }

  treadsRenderNode.setTreads(true);
  renderer.addRenderNode(&tankRenderNode, &gstate);
  renderer.addRenderNode(&treadsRenderNode, &treadState);
}


void TankSceneNode::addShadowNodes(SceneRenderer& renderer)
{
  // use HighTankLOD shadows in experimental mode
  if (TankSceneNode::maxLevel == -1) {
    shadowRenderNode.setTankLOD (HighTankLOD);
  } else {
    shadowRenderNode.setTankLOD (LowTankLOD);
  }
  renderer.addShadowNode(&shadowRenderNode);
}


void TankSceneNode::addLight(SceneRenderer& renderer)
{
  if (jumpJetsOn) {
    // the real light
    jumpJetsRealLight.setColor(jumpJetsScale * 1.5f * 2.0f,
			       jumpJetsScale * 1.0f * 2.0f,
			       jumpJetsScale * 0.5f * 2.0f);
    renderer.addLight(jumpJetsRealLight);
    // the ground lights
    for (int i = 0; i < 4; i++) {
      jumpJetsGroundLights[i].setColor(jumpJetsLengths[i] * 1.5f * 0.5f,
				       jumpJetsLengths[i] * 1.0f * 0.5f,
				       jumpJetsLengths[i] * 0.5f * 0.5f);
      renderer.addLight(jumpJetsGroundLights[i]);
    }
  }
  return;
}


void TankSceneNode::setNormal()
{
  tankSize = Normal;
  setRadius(baseRadius);
  useDimensions = false;
}


void TankSceneNode::setObese()
{
  tankSize = Obese;
  float factor = BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
  setRadius(factor*factor*baseRadius);
  useDimensions = false;
}


void TankSceneNode::setTiny()
{
  tankSize = Tiny;
  float factor = BZDB.eval(StateDatabase::BZDB_TINYFACTOR);
  setRadius(factor*factor*baseRadius);
  useDimensions = false;
}


void TankSceneNode::setNarrow()
{
  tankSize = Narrow;
  setRadius(baseRadius);
  useDimensions = false;
}


void TankSceneNode::setThief()
{
  tankSize = Thief;
  float factor = BZDB.eval(StateDatabase::BZDB_THIEFTINYFACTOR);
  setRadius(factor*factor*baseRadius);
  useDimensions = false;
}


void TankSceneNode::setDimensions(const fvec3& dims)
{
  tankSize = Normal;
  dimensions = dims;
  useDimensions = true;
  return;
}


void TankSceneNode::setExplodeFraction(float t)
{
  explodeFraction = t;
  if (t != 0.0f) {
    const float radius = sqrtf(getSphere().w);
    const float radinc = t * maxExplosionVel;
    const float newrad = radius + radinc;
    setRadius(newrad * newrad);
  }
  return;
}


void TankSceneNode::setJumpJets(float scale)
{
  jumpJetsOn = false;
  if ((scale > 0.0f) && BZDBCache::zbuffer && BZDBCache::texture) {
    jumpJetsOn = true;
    jumpJetsScale = scale;

    // set the real light's position
    const fvec3& pos = getSphere().xyz();
    jumpJetsRealLight.setPosition(pos);

    // set the jet ground-light and model positions
    for (int i = 0; i < 4; i++) {
      const float radians = (float)(azimuth * DEG2RAD);
      const float cos_val = cosf(radians);
      const float sin_val = sinf(radians);
      const fvec3& scaleFactor = TankGeometryMgr::getScaleFactor(tankSize);
      const fvec3& jm = jumpJetsModel[i];
      const fvec2 v = jm.xy() * scaleFactor.xy();
      fvec3& jetPos = jumpJetsPositions[i];
      jetPos.x = pos.x + ((cos_val * v.x) - (sin_val * v.y));
      jetPos.y = pos.y + ((cos_val * v.y) + (sin_val * v.x));
      jetPos.z = pos.z + jm.z;
      jumpJetsGroundLights[i].setPosition(jetPos);

      // setup the random lengths
      const float randomFactor = (1.0f - (0.5f * (0.5f - (float)bzfrand())));
      jumpJetsLengths[i] = jumpJetsScale * randomFactor;
    }
  }
  return;
}


void TankSceneNode::setClipPlane(const fvec4* planePtr)
{
  if (planePtr == NULL) {
    clip = false;
  } else {
    clip = true;
    const fvec4& p = *planePtr;
    clipPlane.x = double(p.x);
    clipPlane.y = double(p.y);
    clipPlane.z = double(p.z);
    clipPlane.w = double(p.w);
  }
}


void TankSceneNode::setMaxLOD(int _maxLevel)
{
  maxLevel = _maxLevel;
  return;
}


void TankSceneNode::setOnlyShadows(bool _onlyShadows)
{
  onlyShadows = _onlyShadows;
  return;
}


void TankSceneNode::setInTheCockpit(bool value)
{
  inTheCockpit = value;
  return;
}


void TankSceneNode::rebuildExplosion()
{
  // prepare explosion rotations and translations
  for (int i = 0; i < LastTankPart; i++) {
    // pick an unbiased rotation vector
    float distSq;
    do {
      spin[i].x = (float)(bzfrand() - 0.5);
      spin[i].y = (float)(bzfrand() - 0.5);
      spin[i].z = (float)(bzfrand() - 0.5);
      distSq = spin[i].xyz().lengthSq();
    } while ((distSq < 1.0e-6) || (distSq > 0.25f));
    spin[i].xyz() *= (1.0f / sqrtf(distSq)); // normalize

    // now an angular velocity -- make sure we get at least 2 complete turns
    spin[i].w = 360.0f * (5.0f * (float)bzfrand() + 2.0f);

    // different cheezy spheroid explosion pattern
    const float vhMax = maxExplosionVel;
    const float vhAngleVert = (float)(0.25 * M_PI * bzfrand());
    const float vhMag = vhMax * cosf(vhAngleVert);
    const float vhAngle = (float)(2.0 * M_PI * bzfrand());
    vel[i].z = vhMax * sinf(vhAngleVert) * vertExplosionRatio;
    vel[i].y = vhMag * cosf(vhAngle);
    vel[i].x = vhMag * sinf(vhAngle);
  }
  return;
}


void TankSceneNode::renderRadar()
{
  const float angleCopy = azimuth;
  const fvec4& mySphere = getSphere();
  fvec3 posCopy = mySphere.xyz();

  // allow negative values for burrowed clipping
  fvec3 tankPos;
  tankPos.x = 0.0f;
  tankPos.y = 0.0f;
  if (mySphere.z >= 0.0f) {
    tankPos.z = 0.0f;
  } else {
    tankPos.z = mySphere.z;
  }

  setCenter(tankPos);
  azimuth = 0.0f;

  float oldAlpha = color.a;
  if (color.a < 0.15f) {
    color.a = 0.15f;
  }

  if (BZDBCache::animatedTreads) {
    treadState.setState();
    treadsRenderNode.setRadar(true);
    treadsRenderNode.sortOrder(true /* above */, false, false);
    treadsRenderNode.render();
    treadsRenderNode.setRadar(false);
  }

  gstate.setState();
  tankRenderNode.setRadar(true);
  tankRenderNode.sortOrder(true /* above */, false, false);
  tankRenderNode.render();
  tankRenderNode.setRadar(false);

  color.a = oldAlpha;

  setCenter(posCopy);
  azimuth = angleCopy;

  return;
}


bool TankSceneNode::cullShadow(int planeCount, const fvec4* planes) const
{
  const fvec4& s = getSphere();
  for (int i = 0; i < planeCount; i++) {
    const fvec4& p = planes[i];
    const float d = p.planeDist(s.xyz());
    if ((d < 0.0f) && ((d * d) > s.w)) {
      return true;
    }
  }
  return false;
}


//============================================================================//
//
// TankIDLSceneNode
//

TankIDLSceneNode::TankIDLSceneNode(const TankSceneNode* _tank) :
				   tank(_tank),
				   renderNode(this)
{
  static const fvec4 defaultPlane(1.0f, 0.0f, 0.0f, 0.0f);
  move(defaultPlane);
  float radius = BZDBCache::tankLength;
  radius = radius * 4.0f;
  setRadius(radius);

  OpenGLGStateBuilder builder(gstate);
  builder.setCulling(GL_NONE);
  builder.setShading(GL_SMOOTH);
  builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gstate = builder.getState();
  return;
}


TankIDLSceneNode::~TankIDLSceneNode()
{
  // do nothing
  return;
}


void TankIDLSceneNode::move(const fvec4& _plane)
{
  plane = _plane;

  // compute new sphere
  const fvec3& center = tank->getCenter();
  setCenter(center + 1.5f * BZDBCache::tankLength * plane.xyz());
  return;
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
  return;
}


void TankIDLSceneNode::addRenderNodes(SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, &gstate);
  return;
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

const fvec3 TankIDLSceneNode::IDLRenderNode::idlVertex[] = {
  // body
  fvec3(2.430f,   0.877f, 0.000f),
  fvec3(2.430f,  -0.877f, 0.000f),
  fvec3(-2.835f,  0.877f, 1.238f),
  fvec3(-2.835f, -0.877f, 1.238f),
  fvec3(2.575f,   0.877f, 1.111f),
  fvec3(2.575f,  -0.877f, 1.111f),
  fvec3(-2.229f, -0.877f, 0.000f),
  fvec3(-2.229f,  0.877f, 0.000f),
  // turret
  fvec3(-1.370f,  0.764f, 2.050f),
  fvec3(-1.370f, -0.765f, 2.050f),
  fvec3(1.580f,  -0.434f, 1.790f),
  fvec3(1.580f,   0.435f, 1.790f),
  fvec3(-0.456f, -1.060f, 1.040f),
  fvec3(-0.456f,  1.080f, 1.040f),
  fvec3(1.480f,   0.516f, 1.040f),
  fvec3(1.480f,  -0.516f, 1.040f),
  // barrel
  fvec3(4.940f,  0.047f, 1.410f),
  fvec3(4.940f, -0.079f, 1.530f),
  fvec3(4.940f,  0.047f, 1.660f),
  fvec3(4.940f,  0.173f, 1.530f),
  fvec3(1.570f,  0.047f, 1.350f),
  fvec3(1.570f, -0.133f, 1.530f),
  fvec3(1.570f,  0.047f, 1.710f),
  fvec3(1.570f,  0.227f, 1.530f),
  // ltread
  fvec3(-2.229f, 0.877f, 0.000f),
  fvec3(2.730f,  1.400f, 1.294f),
  fvec3(2.597f,  1.400f, 0.000f),
  fvec3(-2.970f, 1.400f, 1.410f),
  fvec3(2.730f,  0.877f, 1.294f),
  fvec3(2.597f,  0.877f, 0.000f),
  fvec3(-2.970f, 0.877f, 1.410f),
  fvec3(-2.229f, 1.400f, 0.000f),
  // rtread
  fvec3(-2.229f, -1.400f, 0.000f),
  fvec3(2.730f,  -0.875f, 1.294f),
  fvec3(2.597f,  -0.875f, 0.000f),
  fvec3(-2.970f, -0.875f, 1.410f),
  fvec3(2.730f,  -1.400f, 1.294f),
  fvec3(2.597f,  -1.400f, 0.000f),
  fvec3(-2.970f, -1.400f, 1.410f),
  fvec3(-2.229f, -0.875f, 0.000f)
};


TankIDLSceneNode::IDLRenderNode::IDLRenderNode(
				const TankIDLSceneNode* _sceneNode) :
				sceneNode(_sceneNode)
{
  // do nothing
  return;
}


TankIDLSceneNode::IDLRenderNode::~IDLRenderNode()
{
  // do nothing
  return;
}


void TankIDLSceneNode::IDLRenderNode::render()
{
  static const fvec4 innerColor(1.0f, 1.0f, 1.0f, 0.75f);
  static const fvec4 outerColor(1.0f, 1.0f, 1.0f, 0.0f);

  // compute plane in tank's space
  const fvec4& sphere = sceneNode->tank->getSphere();
  const fvec4& _plane = sceneNode->plane;
  const float azimuth = sceneNode->tank->azimuth;
  fvec4 plane;
  plane.xyz() = _plane.xyz().rotateZ(-azimuth * DEG2RAD);
  plane.w = _plane.planeDist(sphere.xyz());

  // compute projection point -- one TankLength in from plane
  const float pd = -1.0f * BZDBCache::tankLength - plane.w;
  fvec3 origin = pd * plane.xyz();

  glPushMatrix();
    glTranslatef(sphere.x, sphere.y, sphere.z);
    glRotatef(azimuth, 0.0f, 0.0f, 1.0f);

    glBegin(GL_QUADS);
    const int numFaces = countof(idlFaces);
    for (int i = 0; i < numFaces; i++) {
      // get distances from plane
      const int* face = idlFaces[i] + 1;
      const int numVertices = idlFaces[i][0];
      float d[4];
      int j;
      for (j = 0; j < numVertices; j++) {
	d[j] = plane.planeDist(idlVertex[face[j]]);
      }

      // get crossing points
      fvec3 cross[2];
      int crossings = 0;
      int k;
      for (j = 0, k = numVertices-1; j < numVertices; k = j++) {
	if (((d[k] <  0.0f) && (d[j] >= 0.0f)) ||
	    ((d[k] >= 0.0f) && (d[j] <  0.0f))) {
	  const float t = d[k] / (d[k] - d[j]);
	  cross[crossings] = ((1.0f - t) * idlVertex[face[k]])
                                   + (t  * idlVertex[face[j]]);
	  if (++crossings == 2) {
	    break;
          }
	}
      }

      // if not enough crossings then skip
      if (crossings != 2) {
        continue;
      }

      // project points out
      fvec3 project[2];
      const float dist = 2.0f + 0.3f * ((float)bzfrand() - 0.5f);
      project[0] = origin + (dist * (cross[0] - origin));
      project[1] = origin + (dist * (cross[1] - origin));

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
  return;
}


//============================================================================//
//
// TankSceneNode::TankRenderNode
//

// FIXME: setup so these come from TANKGEOMMGR
const fvec3 TankSceneNode::TankRenderNode::centerOfGravity[LastTankPart] = {
  fvec3(0.000f,  0.0f, 1.5f * 0.68f), // body
  fvec3(3.252f,  0.0f, 1.532f),	      // barrel
  fvec3(0.125f,  0.0f, 2.5f * 0.68f), // turret
  fvec3(0.000f, +0.7f, 0.5f * 0.68f), // left case
  fvec3(0.000f, -0.7f, 0.5f * 0.68f), // right case
  fvec3(0.000f, +0.7f, 0.7f), // left tread
  fvec3(0.000f, -0.9f, 0.7f), // right tread
  fvec3(-2.25f, +0.9f, 0.7f), // left wheel0
  fvec3(-0.75f, +0.9f, 0.7f), // left wheel1
  fvec3(+0.75f, +0.9f, 0.7f), // left wheel2
  fvec3(+2.25f, +0.9f, 0.7f), // left wheel3
  fvec3(-2.25f, -0.9f, 0.7f), // right wheel0
  fvec3(-0.75f, -0.9f, 0.7f), // right wheel1
  fvec3(+0.75f, -0.9f, 0.7f), // right wheel2
  fvec3(+2.25f, -0.9f, 0.7f)  // right wheel3
};


TankSceneNode::TankRenderNode::TankRenderNode(const TankSceneNode* _sceneNode) :
				sceneNode(_sceneNode), isShadow(false),
				above(false), towards(false)
{
  narrowWithDepth = false;
  drawLOD = LowTankLOD;
  drawSize = Normal;
  isRadar = false;
  isTreads = false;
  return;
}


TankSceneNode::TankRenderNode::~TankRenderNode()
{
  return;
}


void TankSceneNode::TankRenderNode::setRadar(bool radar)
{
  isRadar = radar;
  return;
}

void TankSceneNode::TankRenderNode::setTreads(bool treads )
{
  isTreads = treads;
}



void TankSceneNode::TankRenderNode::setShadow()
{
  isShadow = true;
  return;
}


void TankSceneNode::TankRenderNode::sortOrder(
				bool _above, bool _towards, bool _left)
{
  above = _above;
  towards = _towards;
  left = _left;
  return;
}


void TankSceneNode::TankRenderNode::setNarrowWithDepth(bool narrow)
{
  narrowWithDepth = narrow;
  return;
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
  color = &sceneNode->color;
  alpha = sceneNode->color.a;

  if (!BZDBCache::blend && sceneNode->transparent) {
    myStipple(alpha);
  }

  if (sceneNode->clip && !isShadow) {
    glClipPlane(GL_CLIP_PLANE0, sceneNode->clipPlane);
    glEnable(GL_CLIP_PLANE0);
  }

  const fvec3& center = sceneNode->getCenter();

  // save the MODELVIEW matrix
  glPushMatrix();

  glTranslatef(center.x, center.y, center.z);
  glRotatef(sceneNode->azimuth, 0.0f, 0.0f, 1.0f);
  glRotatef(sceneNode->elevation, 0.0f, 1.0f, 0.0f);
  if (sceneNode->useDimensions) {
    const fvec3& dims = sceneNode->dimensions;
    glScalef(dims.x, dims.y, dims.z);
    glEnable(GL_NORMALIZE);
  }

  // disable the dynamic lights, if it might help
  const bool switchLights = BZDBCache::lighting &&
			    !isShadow && (drawLOD == HighTankLOD);
  if (switchLights) {
    RENDERER.disableLights(sceneNode->extents);
  }

  if (isRadar && !isExploding) {
    if (BZDBCache::animatedTreads) {
      renderPart(LeftTread);
      renderPart(RightTread);
    }
    renderPart(LeftCasing);
    renderPart(RightCasing);
    renderPart(Body);
    renderPart(Turret);
    renderPart(Barrel);
  }
  else if (!isShadow && (sceneNode->sort || sceneNode->transparent)) {
    // draw is some sorted order
    if (sceneNode->explodeFraction == 0.0f) {
      // normal state
      renderParts();
    }
    else {
      // exploding -- draw back facing stuff first then draw front facing stuff
      glCullFace(GL_FRONT);
      renderParts();
      glCullFace(GL_BACK);
      renderParts();
    }
  }
  else if (narrowWithDepth) {
    renderNarrowWithDepth();
  }
  else if (isShadow && (center.z < 0.0f)) {
    // burrowed or burrowing tank, just render the top shadows
    renderPart(Turret);
    renderPart(Barrel);
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
    if (BZDBCache::animatedTreads) {
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

  // render the jumpJets
  if (!isExploding && !isShadow) {
    renderJumpJets(); // after the matrix has been restored
  }

  // FIXME -- add flare lights using addFlareLight().
  //	  pass light position in world space.

  glShadeModel(GL_FLAT);
  if (!BZDBCache::blend && sceneNode->transparent) {
    myStipple(0.5f);
  }
  if (sceneNode->clip) {
    glDisable(GL_CLIP_PLANE0);
  }

  return;
}


void TankSceneNode::TankRenderNode::renderLeftParts()
{
  renderPart(LeftCasing);

  if (BZDBCache::animatedTreads) {
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

  if (BZDBCache::animatedTreads) {
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


void TankSceneNode::TankRenderNode::renderNarrowWithDepth()
{
  // render the middle stuff
  renderPart(Body);
  renderPart(Turret);
  renderPart(Barrel);

  // use a fill depth buffer offset to avoid flickering
  GLboolean usingPolyOffset;
  float factor, units;
  glGetBooleanv(GL_POLYGON_OFFSET_FILL, &usingPolyOffset);
  if (usingPolyOffset == GL_TRUE) {
    glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &factor);
    glGetFloatv(GL_POLYGON_OFFSET_UNITS, &units);
  } else {
    glEnable(GL_POLYGON_OFFSET_FILL);
  }

  const float offsetFactor = -0.1f;
  const float offsetDepth = -1.0f;

  //glDepthFunc(GL_LEQUAL);

  glPolygonOffset(1.0f * offsetFactor, 1.0f * offsetDepth);
  if (left) {
    renderPart(LeftCasing);
  } else {
    renderPart(RightCasing);
  }

  glPolygonOffset(2.0f * offsetFactor, 2.0f * offsetDepth);
  for (int i = 0; i < 4; i++) {
    if (isShadow && ((i == 1) || (i == 2)) && !isExploding) {
      continue;
    }
    if (left) {
      renderPart((TankPart)(LeftWheel0 + i));
    } else {
      renderPart((TankPart)(RightWheel0 + i));
    }
  }

  glPolygonOffset(3.0f * offsetFactor, 3.0f * offsetDepth);
  if (left) {
    renderPart(LeftTread);
  } else {
    renderPart(RightTread);
  }

  //glDepthFunc(GL_LEQUAL);

  if (usingPolyOffset) {
    glPolygonOffset(factor, units);
  } else {
    glDisable(GL_POLYGON_OFFSET_FILL);
  }

  return;
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
  if (isTreads)
  {
    if ( part != RightTread && part != LeftTread )
      return;
  }
  else
  {
    if ( part == RightTread || part == LeftTread )
      return;
  }

  // apply explosion transform
  if (isExploding) {
    glPushMatrix();
    const fvec3& vel  = sceneNode->vel[part];
    const fvec4& spin = sceneNode->spin[part];
    const fvec3& cog  = centerOfGravity[part];
    const fvec3 pos = cog + (explodeFraction * vel);
    glTranslatef(pos.x, pos.y, pos.z);
    glRotatef(spin.w * explodeFraction, spin.x, spin.y, spin.z);
    glTranslatef(-cog.x, -cog.y, -cog.z);
  }

  // setup the animation texture matrix
  bool usingTexMat = false;
  if (!isShadow && BZDBCache::animatedTreads && (part >= BasicTankParts)) {
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

  // add to the triangle count
  addTriangleCount(TankGeometryMgr::getPartTriangleCount(
				      shadow, part, drawSize, drawLOD));

  // draw the lights on the turret
  if ((part == Turret) && !isExploding && !isShadow) {
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

  return;
}


void TankSceneNode::TankRenderNode::setupPartColor(TankPart part)
{
  const fvec4 white(1.0f, 1.0f, 1.0f, 1.0f);
  const fvec4* clr = color;

  // do not use color modulation with tank textures
  if (BZDBCache::texture) {
    clr = &white;
  }

  switch (part) {
    case Barrel: {
      myColor4f(0.25f, 0.25f, 0.25f, alpha);
      break;
    }
    case Body: {
      myColor4fv(fvec4(clr->rgb(), alpha));
      break;
    }
    case Turret: {
      myColor4fv(fvec4(clr->rgb() * 0.9f, alpha));
      break;
    }
    case LeftCasing:
    case RightCasing: {
      myColor4fv(fvec4(clr->rgb() * 0.7f, alpha));
      break;
    }
    case LeftTread:
    case RightTread: {
      myColor4fv(fvec4(clr->rgb() * 0.3f, alpha));
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
      myColor4fv(fvec4(clr->rgb() * 0.4f, alpha));
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

  float treadUScale = 12.0f;

  switch (part) {
    case LeftTread: {
      glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glScalef(treadUScale, 1.0f, 1.0f);
      glTranslatef(sceneNode->leftTreadOffset, 0.0f, 0.0f);
      glMatrixMode(GL_MODELVIEW);
      break;
    }
    case RightTread: {
      glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glScalef(treadUScale, 1.0f, 1.0f);
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
  if (isTreads)
    return;

  struct LightVertex {
    fvec3 vertex;
    fvec4 color;
  };
  static const LightVertex lights[3] = {
    { fvec3(-1.53f,  0.00f, 2.1f ), fvec4(1.0f, 1.0f, 1.0f, 1.0f) },
    { fvec3( 0.10f,  0.75f, 2.1f ), fvec4(1.0f, 0.0f, 0.0f, 1.0f) },
    { fvec3( 0.10f, -0.75f, 2.1f ), fvec4(0.0f, 1.0f, 0.0f, 1.0f) }
  };

  sceneNode->lightsGState.setState();
  glPointSize(2.0f);

  glBegin(GL_POINTS);
  {
    const fvec3& scale = TankGeometryMgr::getScaleFactor(sceneNode->tankSize);
    myColor3fv(lights[0].color); glVertex3fv(lights[0].vertex * scale);
    myColor3fv(lights[1].color); glVertex3fv(lights[1].vertex * scale);
    myColor3fv(lights[2].color); glVertex3fv(lights[2].vertex * scale);
  }
  glEnd();

  glPointSize(1.0f);
  sceneNode->gstate.setState();

  addTriangleCount(4);

  return;
}


fvec3 TankSceneNode::jumpJetsModel[4] = {
  fvec3(-1.5f, -0.6f, +0.25f),
  fvec3(-1.5f, +0.6f, +0.25f),
  fvec3(+1.5f, -0.6f, +0.25f),
  fvec3(+1.5f, +0.6f, +0.25f)
};

void TankSceneNode::TankRenderNode::renderJumpJets()
{
  if (isTreads)
    return;

  if (!sceneNode->jumpJetsOn) {
    return;
  }

  struct JetVertex {
    fvec3 vertex;
    fvec2 texcoord;
  };
  static const JetVertex jet[3] = {
    { fvec3(+0.3f,  0.0f, 0.0f), fvec2(0.0f, 1.0f) },
    { fvec3(-0.3f,  0.0f, 0.0f), fvec2(1.0f, 1.0f) },
    { fvec3( 0.0f, -1.0f, 0.0f), fvec2(0.5f, 0.0f) }
  };

  myColor4f(1.0f, 1.0f, 1.0f, 0.5f);

  // use a clip plane, because the ground has no depth
  const GLdouble clipPlane[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
  glClipPlane(GL_CLIP_PLANE1, clipPlane);
  glEnable(GL_CLIP_PLANE1);

  sceneNode->jumpJetsGState.setState();
  glDepthMask(GL_FALSE);
  for (int j = 0; j < 4; j++) {
    glPushMatrix();
    {
      const fvec3& pos = sceneNode->jumpJetsPositions[j];
      glTranslatef(pos.x, pos.y, pos.z);
      glScalef(1.0f, 1.0f, sceneNode->jumpJetsLengths[j]);

      RENDERER.getViewFrustum().executeBillboard();

      glBegin(GL_TRIANGLES);
      {
	for (int v = 0; v < 3; v++) {
	  glTexCoord2fv(jet[v].texcoord);
	  glVertex3fv(jet[v].vertex);
	}
      }
      glEnd();
    }
    glPopMatrix();
  }
  glDepthMask(GL_TRUE);
  sceneNode->gstate.setState();

  glDisable(GL_CLIP_PLANE1);

  addTriangleCount(4);

  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
