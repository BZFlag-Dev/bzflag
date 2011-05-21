/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// bzflag common header
#include "common.h"

// interface header
#include "BillboardSceneNode.h"

// system headers
#include <stdlib.h>
#include <math.h>

// common headers
#include "bzfgl.h"
#include "BZDBCache.h"
#include "TextureManager.h"
#include "SceneRenderer.h" // FIXME (SceneRenderer.cpp is in src/bzflag)

// local headers
#include "ViewFrustum.h"
#include "StateDatabase.h"


BillboardSceneNode::BillboardSceneNode(const fvec3& pos)
  : show(false)
  , hasAlpha(false)
  , hasTexture(false)
  , hasTextureAlpha(false)
  , looping(false)
  , lightSource(false)
  , width(1.0f)
  , height(1.0f)
  , cu(1)
  , cv(1)
  , t(0.0f)
  , duration(0.0f)
  , renderNode(this) {
  OpenGLGStateBuilder builder(gstate);
  builder.setBlending();
  builder.setAlphaFunc();
  //builder.setTextureEnvMode(GL_DECAL);
  gstate = builder.getState();

  // prepare light
  lightScale = 1.0f;
  setLightColor(1.0f, 1.0f, 1.0f);
  setLightAttenuation(0.05f, 0.0f, 0.03f);
  setLightFadeStartTime(duration);
  setGroundLight(false);

  // prepare geometry
  move(pos);
  setAngle(0.0f);
  setSize(width, height);
  setColor(1.0f, 1.0f, 1.0f, 1.0f);

  // prepare animation
  resetTime();
}


BillboardSceneNode::~BillboardSceneNode() {
  // do nothing
}


BillboardSceneNode* BillboardSceneNode::copy() const {
  BillboardSceneNode* e = new BillboardSceneNode(getSphere().xyz());
  e->show = show;
  e->hasAlpha = hasAlpha;
  e->hasTexture = hasTexture;
  e->hasTextureAlpha = hasTextureAlpha;
  e->looping = looping;
  e->lightSource = lightSource;
  e->lightColor = lightColor;
  e->lightScale = lightScale;
  e->lightCutoffTime = lightCutoffTime;
  e->width = width;
  e->height = height;
  e->color = color;
  e->angle = angle;
  e->duration = duration;
  e->light = light;
  e->gstate = gstate;

  // begin at the beginning
  e->t = 0.0f;
  e->setTextureAnimation(cu, cv);
  e->prepLight();

  return e;
}


void BillboardSceneNode::setLoop(bool _looping) {
  looping = _looping;
}


void BillboardSceneNode::setDuration(float _duration) {
  duration = _duration;
  if (t > duration) { t = duration; }
  setFrame();
}


void BillboardSceneNode::resetTime() {
  t = 0.0f;
  setFrame();
}


void BillboardSceneNode::updateTime(float dt) {
  // change time by dt then make sure it's in bounds
  const float ot = t;
  t += dt;
  if (duration == 0.0f) { t = 0.0f; }
  else if (!looping) {
    if (t < 0.0f) { t = 0.0f; }
    else if (t > duration) { t = duration; }
  }
  else {
    if (t < 0.0f) { t = duration - fmodf(-t, duration); }
    else if (t >= duration) { t = fmodf(t, duration); }
  }
  setFrame();

  // update light intensity if it changed
  if (t > lightCutoffTime || ot > lightCutoffTime) {
    prepLight();
  }
}


bool BillboardSceneNode::isAtEnd() const {
  return (t == duration);
}


void BillboardSceneNode::setFrame() {
  // update frame
  if (duration == 0.0f) {
    renderNode.setFrame(0.0f, 0.0f);
  }
  else {
    int frame = (int)((t / duration) * (float)cu * (float)cv);
    if (frame >= cu * cv) { frame = cu * cv - 1; }
    renderNode.setFrame((float)(frame % cu) / (float)cu,
                        (float)(frame / cu) / (float)cv);
  }
}


bool BillboardSceneNode::isLight() const {
  return (lightSource && show);
}


void BillboardSceneNode::setLight(bool on) {
  if (lightSource == on) {
    return;
  }
  lightSource = on;
  if (lightSource) {
    prepLight();
  }
}


void BillboardSceneNode::setLightColor(float r, float g, float b) {
  lightColor = fvec4(r, g, b, 1.0f);
  prepLight();
}


void BillboardSceneNode::setLightAttenuation(float c, float l, float q) {
  light.setAttenuation(0, c);
  light.setAttenuation(1, l);
  light.setAttenuation(2, q);
}


void BillboardSceneNode::setLightScaling(float s) {
  lightScale = s;
  prepLight();
}


void BillboardSceneNode::setLightFadeStartTime(float _t) {
  lightCutoffTime = _t;
  prepLight();
}


void BillboardSceneNode::setGroundLight(bool value) {
  groundLight = value;
  light.setOnlyGround(value);
}


void BillboardSceneNode::prepLight() {
  if (!lightSource) { return; }
  const float s = (t <= lightCutoffTime || lightCutoffTime >= duration) ? 1.0f :
                  (1.0f - (t - lightCutoffTime) / (duration - lightCutoffTime));
  light.setColor(lightColor.r * lightScale * s,
                 lightColor.g * lightScale * s,
                 lightColor.b * lightScale * s);
}


void BillboardSceneNode::setSize(float side) {
  setSize(side, side);
}


void BillboardSceneNode::setSize(float _width, float _height) {
  width = 0.5f * _width;
  height = 0.5f * _height;
  setRadius(width * width + height * height);
}


void BillboardSceneNode::setColor(float r, float g, float b, float a) {
  color = fvec4(r, g, b, a);
  hasAlpha = (color.a != 1.0f || hasTextureAlpha);
}


void BillboardSceneNode::setColor(const fvec4& rgba) {
  setColor(rgba.r, rgba.g, rgba.b, rgba.a);
}



void BillboardSceneNode::setTexture(const int texture) {
  hasTexture = texture >= 0;
  TextureManager& tm = TextureManager::instance();

  hasTextureAlpha = hasTexture && tm.getInfo(texture).alpha;
  hasAlpha = (color.a != 1.0f || hasTextureAlpha);
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(texture);
  builder.enableTexture(hasTexture);
  gstate = builder.getState();
}


void BillboardSceneNode::setTextureAnimation(int _cu, int _cv) {
  cu = _cu;
  cv = _cv;
  renderNode.setFrameSize(1.0f / (float)cu, 1.0f / (float)cv);
  setFrame();
}


void BillboardSceneNode::move(const fvec3& pos) {
  setCenter(pos);
  light.setPosition(pos);
}


void BillboardSceneNode::setAngle(float _angle) {
  angle = RAD2DEGf * _angle;
}


void BillboardSceneNode::addLight(SceneRenderer& renderer) {
  if (show && lightSource) {
    renderer.addLight(light);
  }
}


void BillboardSceneNode::notifyStyleChange() {
  show = hasTexture && BZDBCache::texture && (!hasAlpha || BZDBCache::blend);
  if (show) {
    OpenGLGStateBuilder builder(gstate);
    if (hasAlpha) {
      if (RENDERER.useQuality() >= _EXPERIMENTAL_QUALITY) {
        builder.setBlending(GL_SRC_ALPHA, GL_ONE);
      }
      else {
        builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      }
      builder.setAlphaFunc();
    }
    else {
      builder.resetBlending();
      builder.resetAlphaFunc();
    }
    gstate = builder.getState();
  }
}


void BillboardSceneNode::addRenderNodes(SceneRenderer& renderer) {
  if (show) {
    renderer.addRenderNode(&renderNode, &gstate);
  }
}


//
// BillboardSceneNode::BillboardRenderNode
//

BillboardSceneNode::BillboardRenderNode::BillboardRenderNode(
  const BillboardSceneNode* _sceneNode)
  : sceneNode(_sceneNode) {
  setFrame(0.0f, 0.0f);
  setFrameSize(1.0f, 1.0f);
}

BillboardSceneNode::BillboardRenderNode::~BillboardRenderNode() {
  // do nothing
}


void BillboardSceneNode::BillboardRenderNode::setFrame(float _u, float _v) {
  u = _u;
  v = _v;
}


void BillboardSceneNode::BillboardRenderNode::setFrameSize(float _du, float _dv) {
  du = _du;
  dv = _dv;
}


void BillboardSceneNode::BillboardRenderNode::render() {
  static const GLdouble groundPlane[] = { 0.0, 0.0, 1.0, 0.0 };

  glClipPlane(GL_CLIP_PLANE0, groundPlane);
  glEnable(GL_CLIP_PLANE0);

  // want to move the billboard directly towards the eye a little bit.
  // can't translate in Z after the billboard is applied because that
  // will move in the direction of the view, which isn't necessarily
  // the direction to the billboard from the eye.
  ViewFrustum& frustum = RENDERER.getViewFrustum();
  const fvec3& eye = frustum.getEye();
  const fvec4& sphere = sceneNode->getSphere();

  const fvec3 dir = eye - sphere.xyz();
  const float dist = sceneNode->width / dir.length();
  const fvec3 trans = sphere.xyz() + (dist * dir);

  const bool blackFog = sceneNode->hasAlpha &&
                        RENDERER.isFogActive() &&
                        (RENDERER.useQuality() >= _EXPERIMENTAL_QUALITY);
  if (blackFog) {
    glFogfv(GL_FOG_COLOR, fvec4(0.0f, 0.0f, 0.0f, 0.0f));
  }

  glPushMatrix();
  {
    glTranslatef(trans.x, trans.y, trans.z);
    frustum.executeBillboard();
    glRotatef(sceneNode->angle, 0.0f, 0.0f, 1.0f);

    // draw billboard
    myColor4fv(sceneNode->color);
    glBegin(GL_QUADS);
    glTexCoord2f(u,    v); glVertex2f(-sceneNode->width, -sceneNode->height);
    glTexCoord2f(du + u,    v); glVertex2f(sceneNode->width, -sceneNode->height);
    glTexCoord2f(du + u, dv + v); glVertex2f(sceneNode->width,  sceneNode->height);
    glTexCoord2f(u, dv + v); glVertex2f(-sceneNode->width,  sceneNode->height);
    glEnd();
  }
  glPopMatrix();

  if (blackFog) {
    glFogfv(GL_FOG_COLOR, RENDERER.getFogColor());
  }

  addTriangleCount(2);

  glDisable(GL_CLIP_PLANE0);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
