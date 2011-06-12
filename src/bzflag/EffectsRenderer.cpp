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

// interface header
#include "EffectsRenderer.h"

// common headers
#include "bzfgl.h"
#include "3D/TextureManager.h"
#include "common/StateDatabase.h"
#include "common/BzTime.h"
#include "game/Flag.h"
#include "ogl/OpenGLUtils.h"
#include "game/BZDBCache.h"


static BZDB_bool useFancyEffects("useFancyEffects");


template <>
EffectsRenderer* Singleton<EffectsRenderer>::_instance = (EffectsRenderer*)0;


// utils for geo
static void drawRingYZ(float rad, float z, float topsideOffset = 0,
                       float bottomUV = 0, float ZOffset = 0,
                       float topUV = 1.0f, int segments = 32,
                       float uScale = 2.0f);
static void drawRingXY(float rad, float z, float topsideOffset = 0,
                       float bottomUV = 0, float topUV = 1.0f,
                       int segments = 32, float uScale = 2.0f);
static void drawRingZ(float innerRad, float outerRad,
                      float innerUV = 0.0f , float outerUV = 1.0f,
                      float ZOffset = 0, int segments = 32,
                      float uScale = 1.0f);

static void RadialToCartesian(float angle, float rad, fvec3& pos);


EffectsRenderer::EffectsRenderer() {
}

EffectsRenderer::~EffectsRenderer() {
  for (unsigned int i = 0; i < effectsList.size(); i++) {
    delete(effectsList[i]);
  }

  effectsList.clear();
}

void EffectsRenderer::init(void) {
  for (unsigned int i = 0; i < effectsList.size(); i++) {
    delete(effectsList[i]);
  }

  effectsList.clear();
}

void EffectsRenderer::update(void) {
  const BzTime time = BzTime::getTick();

  tvEffectsList::iterator i = effectsList.begin();
  while (i != effectsList.end()) {
    if ((*i)->update(time)) {
      delete((*i));
      i = effectsList.erase(i);
    }
    else {
      i++;
    }
  }
}

void EffectsRenderer::draw(const SceneRenderer& sr) {
  // FIXME: really should check here for only the things that are VISIBLE!!!
  for (unsigned int i = 0; i < effectsList.size(); i++) {
    effectsList[i]->draw(sr);
  }
}

void EffectsRenderer::addSpawnEffect(const fvec4& rgba, const fvec3& pos) {
  if (!useFancyEffects) {
    return;
  }

  BasicEffect* effect = NULL;
  switch (BZDB.evalInt("spawnEffect")) {
    case 1:
      effect = new StdSpawnEffect;
      break;

    case 2:
      effect = new ConeSpawnEffect;
      break;

    case 3:
      effect = new RingSpawnEffect;
      break;

    default:
      // includes "0" (no effect)
      break;
  }

  if (effect) {
    effect->setPos(pos);
    effect->setStartTime();
    effect->setColor(rgba);
    effectsList.push_back(effect);
  }
}

std::vector<std::string> EffectsRenderer::getSpawnEffectTypes(void) {
  std::vector<std::string> ret;
  ret.push_back(std::string("Off"));
  ret.push_back(std::string("Blossom"));
  ret.push_back(std::string("Cone"));
  ret.push_back(std::string("Rings"));

  return ret;
}

void EffectsRenderer::addShotEffect(const fvec4& rgba, const fvec3& pos,
                                    float rot, const fvec3& vel, int _type) {
  if (!useFancyEffects) {
    return;
  }

  int flashType = (_type >= 0) ? _type : BZDB.evalInt("shotEffect");

  BasicEffect* effect = NULL;
  switch (flashType) {
    case 1:
      effect = new StdShotEffect;
      break;
    case 2:
      effect = new FlashShotEffect;
      break;
    case 3:
      // composite effect
      addShotEffect(rgba, pos, rot, vel, 1);
      addShotEffect(rgba, pos, rot, vel, 2);
      break;
  }

  if (effect) {
    effect->setPos(pos);
    effect->setRot(fvec3(0.0f, 0.0f, rot));
    effect->setStartTime();
    if (BZDB.isTrue("useVelOnShotEffects")) {
      effect->setVel(vel);
    }
    effect->setColor(rgba);

    effectsList.push_back(effect);
  }
}

std::vector<std::string> EffectsRenderer::getShotEffectTypes(void) {
  std::vector<std::string> ret;
  ret.push_back(std::string("Off"));
  ret.push_back(std::string("Smoke Rings"));
  ret.push_back(std::string("Muzzle Flash"));
  ret.push_back(std::string("Smoke and Flash"));

  return ret;
}

void EffectsRenderer::addGMPuffEffect(const fvec3& pos, const fvec2& rot,
                                      const fvec3* velPtr) {
  if (!useFancyEffects) {
    return;
  }

  BasicEffect* effect = NULL;
  switch (BZDB.evalInt("gmPuffEffect")) {
    case 1:
      // handled outside this manager in the "old" code
      // FIXME: it'd be nice to move that here
      break;

    case 2:
      effect = new StdGMPuffEffect;
      break;

    default:
      // includes "0" (no effect)
      break;
  }

  if (effect) {
    effect->setPos(pos);
    effect->setRot(fvec3(0.0f, rot.y, rot.x));
    effect->setStartTime();
    if (velPtr && BZDB.isTrue("useVelOnShotEffects")) {
      effect->setVel(*velPtr);
    }
    effectsList.push_back(effect);
  }
}

std::vector<std::string> EffectsRenderer::getGMPuffEffectTypes(void) {
  std::vector<std::string> ret;
  ret.push_back(std::string("Off"));
  ret.push_back(std::string("Classic Puff"));
  ret.push_back(std::string("Shock Cone"));

  return ret;
}

void EffectsRenderer::addDeathEffect(const fvec4& rgba, const fvec3& pos,
                                     float rot) {
  if (!useFancyEffects) {
    return;
  }

  BasicEffect* effect = NULL;
  switch (BZDB.evalInt("deathEffect")) {
    case 1:
      effect = new StdDeathEffect;
      break;

    default:
      // includes "0" (no effect)
      break;
  }

  if (effect) {
    effect->setPos(pos);
    effect->setRot(fvec3(0.0f, 0.0f, rot));
    effect->setStartTime();
    effect->setColor(rgba);
    effectsList.push_back(effect);
  }
}

std::vector<std::string> EffectsRenderer::getDeathEffectTypes(void) {
  std::vector<std::string> ret;
  ret.push_back(std::string("Off"));
  ret.push_back(std::string("We Got Death Star"));

  return ret;
}

// landing effects
void EffectsRenderer::addLandEffect(const fvec4& rgba, const fvec3& pos, float rot) {
  if (!useFancyEffects) {
    return;
  }

  BasicEffect* effect = NULL;
  switch (BZDB.evalInt("landEffect")) {
    case 1:
      effect = new StdLandEffect;
      break;
  }

  if (effect) {
    effect->setPos(pos);
    effect->setRot(fvec3(0.0f, 0.0f, rot));
    effect->setStartTime();
    effect->setColor(rgba);
    effectsList.push_back(effect);
  }
}

std::vector<std::string> EffectsRenderer::getLandEffectTypes(void) {
  std::vector<std::string> ret;
  ret.push_back(std::string("Off"));
  ret.push_back(std::string("Dirt Flash"));

  return ret;
}

void EffectsRenderer::addRicoEffect(const fvec3& pos,
                                    const fvec3& normal,
                                    const fvec3* velPtr) {
  if (!useFancyEffects) {
    return;
  }

  BasicEffect* effect = NULL;
  switch (BZDB.evalInt("ricoEffect")) {
    case 1:
      effect = new StdRicoEffect;
      break;

    default:
      // includes "0" (no effect)
      break;
  }

  if (effect) {
    fvec3 rots(0.0f, 0.0f, 0.0f);
    rots.y = atan2f(normal.z, normal.xy().length());
    rots.z = atan2f(normal.y, normal.x);

    effect->setPos(pos);
    effect->setRot(rots);
    effect->setStartTime();
    if (velPtr && BZDB.isTrue("useVelOnShotEffects")) {
      effect->setVel(*velPtr);
    }

    effectsList.push_back(effect);
  }
}

std::vector<std::string> EffectsRenderer::getRicoEffectTypes(void) {
  std::vector<std::string> ret;
  ret.push_back(std::string("Off"));
  ret.push_back(std::string("Ring"));
  // ret.push_back(std::string("Sparks"));

  return ret;
}

void EffectsRenderer::addShotTeleportEffect(const fvec3& pos,
                                            const fvec3& vel,
                                            const fvec4* clipPlane) {
  if (!useFancyEffects) {
    return;
  }

  fvec3 p = pos;

  BasicEffect* effect = NULL;
  switch (BZDB.evalInt("tpEffect")) {
    case 1: {
      effect = new StdShotTeleportEffect(2.0f, NULL);
      break;
    }
    case 2: { // with clipping
      if (!clipPlane) {
        effect = new StdShotTeleportEffect(2.0f, NULL);
      }
      else {
        effect = new StdShotTeleportEffect(3.0f, clipPlane);
        p -= vel.normalize();
      }
      break;
    }
    default: { // includes "0" (no effect)
      break;
    }
  }

  if (effect) {
    fvec3 rots(0.0f, 0.0f, 0.0f);
    rots.y = atan2f(-vel.z, vel.xy().length());
    rots.z = atan2f(vel.y, vel.x);

    effect->setPos(p);
    effect->setRot(rots);
    effect->setStartTime();
    if (false && BZDB.isTrue("useVelOnShotEffects")) {
      effect->setVel(vel);
    }
    effectsList.push_back(effect);
  }
}

std::vector<std::string> EffectsRenderer::getShotTeleportEffectTypes(void) {
  std::vector<std::string> ret;
  ret.push_back(std::string("None"));
  ret.push_back(std::string("IDL"));
  ret.push_back(std::string("IDL(fancy)"));
  // ret.push_back(std::string("Sparks"));
  return ret;
}


//****************** effects base class*******************************

BasicEffect::BasicEffect()
  : position(0.0f, 0.0f, 0.0f)
  , rotation(0.0f, 0.0f, 0.0f)
  , velocity(0.0f, 0.0f, 0.0f)
  , color(0.0f, 0.0f, 0.0f, 1.0f) {
  startTime = BzTime::getCurrent();

  lifeTime = 0;
  lastTime = startTime;
  deltaTime = 0;
}

void BasicEffect::setPos(const fvec3& pos) {
  position = pos;
}

void BasicEffect::setRot(const fvec3& rot) {
  rotation = rot;
}

void BasicEffect::setVel(const fvec3& vel) {
  velocity = vel;
}

void BasicEffect::setColor(const fvec4& rgba) {
  color = rgba;
}

void BasicEffect::setStartTime() {
  startTime = BzTime::getTick();
  lastTime = startTime;
  deltaTime = 0;
}

bool BasicEffect::update(const BzTime time) {
  age = (float)(time - startTime);

  if (age >= lifeTime) {
    return true;
  }

  deltaTime = (float)(time - lastTime);
  lastTime = time;
  return false;
}

//******************StdSpawnEffect****************
StdSpawnEffect::StdSpawnEffect() : BasicEffect() {
  texture = TextureManager::instance().getTextureID("wavy_flare", false);
  lifeTime = 1.5f;
  radius = 1.75f;

  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA, (GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  if (texture > -1) {
    gstate.setTexture(texture);
  }

  ringState = gstate.getState();
}

StdSpawnEffect::~StdSpawnEffect() {
}

bool StdSpawnEffect::update(const BzTime time) {
  // see if it's time to die
  // if not update all those fun times
  if (BasicEffect::update(time)) {
    return true;
  }

  radius += deltaTime * 7;
  return false;
}

void StdSpawnEffect::draw(const SceneRenderer&) {
  glPushMatrix();

  glTranslatef(position.x, position.y, position.z + 0.1f);

  ringState.setState();

  float ageParam = age / lifeTime;

  glColor4f(color.r, color.g, color.b, 1.0f - ageParam);
  glDepthMask(0);

  drawRingXY(radius * 0.1f, 2.5f + (age * 2), 0, 0, 1.0f, 32, 5.0f);
  drawRingXY(radius * 0.5f, 1.5f + (ageParam / 1.0f * 2), 0.5f, 0.5f);
  drawRingXY(radius, 2, 0, 0, 1.0f, 32, 1.0f);

  glColor4f(1, 1, 1, 1);
  glDepthMask(1);
  glPopMatrix();
}

//******************ConeSpawnEffect****************
bool ConeSpawnEffect::update(const BzTime time) {
  // see if it's time to die
  // if not update all those fun times
  if (BasicEffect::update(time)) {
    return true;
  }

  radius += deltaTime * 5;
  return false;
}

void ConeSpawnEffect::draw(const SceneRenderer&) {
  glPushMatrix();

  glTranslatef(position.x, position.y, position.z + 0.1f);

  ringState.setState();

  glColor4f(color.r, color.g, color.b, 1.0f - (age / lifeTime));
  glDepthMask(0);

  drawRingXY(radius * 0.5f, 1.25f);

  glTranslatef(0, 0, 2);
  drawRingXY(radius * 0.6f, 1.5f);

  glTranslatef(0, 0, 2);
  drawRingXY(radius * 0.75f, 1.75f);

  glTranslatef(0, 0, 2);
  drawRingXY(radius * 0.85f, 1.89f);

  glTranslatef(0, 0, 2);
  drawRingXY(radius, 2.0f);

  glColor4f(1, 1, 1, 1);
  glDepthMask(1);
  glPopMatrix();
}


//******************RingSpawnEffect****************
RingSpawnEffect::RingSpawnEffect() {
  radius = 4.0f;
  maxZ = 10.0f;
}

bool RingSpawnEffect::update(const BzTime time) {
  // see if it's time to die
  // if not update all those fun times
  if (BasicEffect::update(time)) {
    return true;
  }

  return false;
}

void RingSpawnEffect::draw(const SceneRenderer&) {
  glPushMatrix();

  glTranslatef(position.x, position.y, position.z);

  ringState.setState();

  glDepthMask(0);

  ringRange = lifeTime / 4.0f;  // first 3/4ths of the life are rings, last is fade
  ringRange = (ringRange * 3) / 4.0f; // of the ring section there are 4 ring segments

  const float bigRange = ringRange * 3;

  float coreAlpha = 1;
  if (age >= bigRange) {
    coreAlpha = 1.0f - ((age - bigRange) / (lifeTime - bigRange));
  }

  for (int n = 0; n < 4; ++n) {
    drawRing(n, coreAlpha);
  }

  glColor4f(1, 1, 1, 1);
  glDepthMask(1);
  glPopMatrix();
}

void RingSpawnEffect::drawRing(int n, float coreAlpha) {
  float posZ = 0;
  float alpha;

  if (age > (ringRange * (n - 1))) { // this ring in?
    if (age < ringRange * n) { // the ring is still coming in
      posZ = maxZ - ((age - ringRange * (n - 1)) / ringRange) * (maxZ - n * 2.5f);
      alpha = (age - ringRange) / (ringRange * n);
    }
    else {
      posZ = n * 2.5f;
      alpha = coreAlpha;
    }

    glPushMatrix();
    glTranslatef(0, 0, posZ);
    glColor4f(color.r, color.g, color.b, alpha);
    drawRingXY(radius, 2.5f * n);
    glPopMatrix();
  }
}

//******************StdShotEffect****************
StdShotEffect::StdShotEffect() : BasicEffect() {
  texture = TextureManager::instance().getTextureID("wavy_flare", false);
  lifeTime = 1.5f;
  radius = 0.125f;


  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA, (GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  if (texture > -1) {
    gstate.setTexture(texture);
  }

  ringState = gstate.getState();
}

StdShotEffect::~StdShotEffect() {
}

bool StdShotEffect::update(const BzTime time) {
  // see if it's time to die
  // if not update all those fun times
  if (BasicEffect::update(time)) {
    return true;
  }

  radius += deltaTime * 6;
  return false;
}

void StdShotEffect::draw(const SceneRenderer&) {
  glPushMatrix();

  fvec3 pos = position + (age * velocity);

  glTranslatef(pos.x, pos.y, pos.z);
  glRotatef(180.0f + (rotation.z * RAD2DEGf), 0.0f, 0.0f, 1.0f);

  ringState.setState();

  color.r = color.g = color.b = 1;

  float alpha = 0.5f - (age / lifeTime);
  if (alpha < 0.001f) {
    alpha = 0.001f;
  }

  glColor4f(color.r, color.g, color.b, alpha);
  glDepthMask(0);

  drawRingYZ(radius, 0.5f, 1.0f + age * 5, 0.65f, pos.z, 1.0f, 32, 5.0f);

  glColor4f(1, 1, 1, 1);
  glDepthMask(1);
  glPopMatrix();
}

//******************FlashShotEffect****************
FlashShotEffect::FlashShotEffect() : StdShotEffect() {
  // we use the jump jet texture upside-down to get a decent muzzle flare effect
  texture = TextureManager::instance().getTextureID("jumpjets", false);
  lifeTime = 0.75f;
  radius = 0.5f;

  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA, (GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  if (texture > -1) {
    gstate.setTexture(texture);
  }

  ringState = gstate.getState();
}

bool FlashShotEffect::update(const BzTime time) {
  // see if it's time to die
  // if not update all those fun times
  if (BasicEffect::update(time)) {
    return true;
  }

  // do stuff that may be need to be done every time to an image
  if (age < lifeTime / 2) {
    length = 6 * (age / lifeTime);
  }
  else {
    length = 6 * (1 - (age / lifeTime));
  }

  return false;
}

void FlashShotEffect::draw(const SceneRenderer&) {
  glPushMatrix();

  fvec3 pos = position + (age * velocity);

  glTranslatef(pos.x, pos.y, pos.z);
  glRotatef(270.0f + (rotation.z * RAD2DEGf), 0.0f, 0.0f, 1.0f);

  ringState.setState();

  color.r = color.g = color.b = 1;

  float alpha = 0.8f - (age / lifeTime);
  if (alpha < 0.001f) {
    alpha = 0.001f;
  }

  glColor4f(color.r, color.g, color.b, alpha);
  glDepthMask(0);

  // draw me here
  glBegin(GL_QUADS);

  // side 1
  glTexCoord2f(0, 1);
  glVertex3f(0, 0, radius);

  glTexCoord2f(0, 0);
  glVertex3f(0, length, radius);

  glTexCoord2f(1, 0);
  glVertex3f(0, length, -radius);

  glTexCoord2f(1, 1);
  glVertex3f(0, 0, -radius);

  // side 2
  glTexCoord2f(0, 0);
  glVertex3f(0, 0, -radius);

  glTexCoord2f(0, 1);
  glVertex3f(0, length, -radius);

  glTexCoord2f(1, 1);
  glVertex3f(0, length, radius);

  glTexCoord2f(1, 0);
  glVertex3f(0, 0, radius);

  glEnd();

  glColor4f(1, 1, 1, 1);
  glDepthMask(1);
  glPopMatrix();
}

//******************StdDeathEffect****************
StdDeathEffect::StdDeathEffect() : BasicEffect() {
  texture = TextureManager::instance().getTextureID("blend_flash", false);
  lifeTime = 1.25f;
  radius = 2.0f;


  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA, (GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  if (texture > -1) {
    gstate.setTexture(texture);
  }

  ringState = gstate.getState();
}

StdDeathEffect::~StdDeathEffect() {
}

bool StdDeathEffect::update(const BzTime time) {
  // see if it's time to die
  // if not update all those fun times
  if (BasicEffect::update(time)) {
    return true;
  }

  radius += deltaTime * 25;
  return false;
}

void StdDeathEffect::draw(const SceneRenderer&) {
  glPushMatrix();

  glTranslatef(position.x, position.y, position.z);
  glRotatef(180.0f + (rotation.z * RAD2DEGf), 0.0f, 0.0f, 1.0f);

  ringState.setState();

  color.r = 108.0f / 256.0f;
  color.g = 16.0f / 256.0f;
  color.b = 16.0f / 256.0f;

  fvec3 deltas = 1.0f - color.rgb();

  float ageParam = age / lifeTime;

  float alpha = 1.0f - ageParam;
  if (alpha < 0.0f) {
    alpha = 0.0f;
  }

  color.rgb() += deltas * ageParam;
  color.a = alpha;

  glColor4fv(color);
  glDepthMask(0);

  glPushMatrix();
  glTranslatef(0, 0, 0.5f);
  drawRingXY(radius * 0.75f, 1.5f + (ageParam / 1.0f * 10), 0.5f * age, 0.5f);
  drawRingXY(radius, -0.5f, 0.5f + age, 0.5f);

  glRotatef(5, 0, 1, 0);
  drawRingZ(radius, radius + (radius * 0.45f));
  glRotatef(-5, 0, 1, 0);
  drawRingZ(radius, radius + (radius * 0.25f));
  glRotatef(-5, 0, 1, 0);
  drawRingZ(radius, radius + (radius * 0.25f));
  glPopMatrix();

  glColor4f(1, 1, 1, 1);
  glDepthMask(1);
  glPopMatrix();
}

//******************StdLandEffect****************
StdLandEffect::StdLandEffect() : BasicEffect() {
  texture = TextureManager::instance().getTextureID("dusty_flare", false);
  lifeTime = 1.0f;
  radius = 2.5f;

  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA, (GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  if (texture > -1) {
    gstate.setTexture(texture);
  }

  ringState = gstate.getState();
}

StdLandEffect::~StdLandEffect() {
}

bool StdLandEffect::update(const BzTime time) {
  // see if it's time to die
  // if not update all those fun times
  if (BasicEffect::update(time)) {
    return true;
  }

  radius += deltaTime * 3.5f;
  return false;
}

void StdLandEffect::draw(const SceneRenderer&) {
  glPushMatrix();

  glTranslatef(position.x, position.y, position.z);

  ringState.setState();

  color.r = 1;
  color.g = 1;
  color.b = 1;

  glColor4f(color.r, color.g, color.b, 1.0f - (age / lifeTime));
  glDepthMask(0);

  drawRingXY(radius, 0.5f + age, 0.05f * radius, 0.0f, 0.9f);

  glColor4f(1, 1, 1, 1);
  glDepthMask(1);
  glPopMatrix();
}

//******************StdGMPuffEffect****************
StdGMPuffEffect::StdGMPuffEffect() : BasicEffect() {
  texture = TextureManager::instance().getTextureID("blend_flash", false);
  lifeTime = 6.5f;
  radius = 0.125f;


  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA, (GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  if (texture > -1) {
    gstate.setTexture(texture);
  }

  ringState = gstate.getState();
}

StdGMPuffEffect::~StdGMPuffEffect() {
}

bool StdGMPuffEffect::update(const BzTime time) {
  // see if it's time to die
  // if not update all those fun times
  if (BasicEffect::update(time)) {
    return true;
  }

  radius += deltaTime * 0.5f;
  return false;
}

void StdGMPuffEffect::draw(const SceneRenderer&) {
  glPushMatrix();

  fvec3 pos = position + (age * velocity);

  glTranslatef(pos.x, pos.y, pos.z);
  glRotatef(180.0f + (rotation.z * RAD2DEGf), 0.0f, 0.0f, 1.0f);
  glRotatef((rotation.y * RAD2DEGf), 0.0f, 1.0f, 0.0f);

  ringState.setState();

  color.r = color.g = color.b = 1;

  float alpha = 0.5f - (age / lifeTime);
  if (alpha < 0.000001f) {
    alpha = 0.000001f;
  }

  glColor4f(color.r, color.g, color.b, alpha);
  glDepthMask(0);

  drawRingYZ(radius, -0.25f - (age * 0.125f), 0.5f + age * 0.75f, 0.50f, pos.z);

  glColor4f(1, 1, 1, 1);
  glDepthMask(1);
  glPopMatrix();
}

//******************StdRicoEffect****************
StdRicoEffect::StdRicoEffect() : BasicEffect() {
  texture = TextureManager::instance().getTextureID("blend_flash", false);
  lifeTime = 0.5f;
  radius = 0.25f;

  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA, (GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  if (texture > -1) {
    gstate.setTexture(texture);
  }

  ringState = gstate.getState();
}

StdRicoEffect::~StdRicoEffect() {
}

bool StdRicoEffect::update(const BzTime time) {
  // see if it's time to die
  // if not update all those fun times
  if (BasicEffect::update(time)) {
    return true;
  }

  radius += deltaTime * 6.5f;
  return false;
}

void StdRicoEffect::draw(const SceneRenderer&) {
  glPushMatrix();

  fvec3 pos = position + (age * velocity);

  glTranslatef(pos.x, pos.y, pos.z);
  glRotatef((rotation.z * RAD2DEGf) + 180.0f, 0.0f, 0.0f, 1.0f);
  glRotatef((rotation.y * RAD2DEGf), 0.0f, 1.0f, 0.0f);

  ringState.setState();

  color.r = color.g = color.b = 1;

  float alpha = 0.5f - (age / lifeTime);
  if (alpha < 0.000001f) {
    alpha = 0.000001f;
  }

  glColor4f(color.r, color.g, color.b, alpha);
  glDepthMask(0);

  drawRingYZ(radius, -0.5f, 0.5f, 0.50f, pos.z);

  glColor4f(1, 1, 1, 1);
  glDepthMask(1);
  glPopMatrix();
}

//******************StdShotTeleportEffect****************
StdShotTeleportEffect::StdShotTeleportEffect(float len, const fvec4* cp)
  : BasicEffect()
  , length(len)
  , clipPlane(cp) {
  texture = TextureManager::instance().getTextureID("dusty_flare", false);
  lifeTime = 4.0f;
  radius = 0.25f;

  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA, (GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  if (texture > -1) {
    gstate.setTexture(texture);
  }

  ringState = gstate.getState();
}

StdShotTeleportEffect::~StdShotTeleportEffect() {
}

bool StdShotTeleportEffect::update(const BzTime time) {
  // see if it's time to die
  // if not update all those fun times
  if (BasicEffect::update(time)) {
    return true;
  }

  //radius += deltaTime*6.5f;
  return false;
}

void StdShotTeleportEffect::draw(const SceneRenderer&) {
  const GLenum clipperID = GL_CLIP_PLANE0;
  if (clipPlane) {
    double cp[4] = {
      double(clipPlane->x),
      double(clipPlane->y),
      double(clipPlane->z),
      double(clipPlane->w)
    };
    glClipPlane(clipperID, cp);
    glEnable(clipperID);
  }

  glPushMatrix();

  fvec3 pos = position + (age * velocity);

  glTranslatef(pos.x, pos.y, pos.z);
  glRotatef(rotation.z * RAD2DEGf, 0.0f, 0.0f, 1.0f);
  glRotatef(rotation.y * RAD2DEGf, 0.0f, 1.0f, 0.0f);
  glRotatef(age * 90, 1, 0, 0);

  ringState.setState();

  color.r = color.g = color.b = 1;

  const float fraction = 1.0f - (age / lifeTime);

  const float alpha = 0.25f + (0.75f * fraction);

  glColor4f(color.r, color.g, color.b, alpha);
  glDepthMask(0);

  drawRingYZ(0.0f,
             length * fraction,
             length * fraction * (1.0f - fraction),
             0.00f, +MAXFLOAT, 0.8f, 6);
//             0.00f, pos.z, 0.8f, 6);

  glColor4f(1, 1, 1, 1);
  glDepthMask(1);
  glPopMatrix();

  if (clipPlane) {
    glDisable(clipperID);
  }
}

//******************************** geo utiliys********************************

static void RadialToCartesian(float angle, float rad, fvec3& pos) {
  pos.x = sinf(angle * DEG2RADf) * rad;
  pos.y = cosf(angle * DEG2RADf) * rad;
}

static void drawRingXY(float rad, float z, float topsideOffset, float bottomUV,
                       float topUV, int segments, float uScale) {
  for (int i = 0; i < segments; i ++) {
    float thisAng = 360.0f / segments * i;
    float nextAng = 360.0f / segments * (i + 1);
    if (i + 1 >= segments) {
      nextAng = 0;
    }

    fvec3 thispos(0.0f, 0.0f, 0.0f);
    fvec3 nextPos(0.0f, 0.0f, 0.0f);
    fvec3 thispos2(0.0f, 0.0f, z);
    fvec3 nextPos2(0.0f, 0.0f, z);
    fvec3 thisNormal(0.0f, 0.0f, 0.0f);
    fvec3 nextNormal(0.0f, 0.0f, 0.0f);

    RadialToCartesian(thisAng, rad,  thispos);
    RadialToCartesian(thisAng, 1.0f, thisNormal);
    RadialToCartesian(nextAng, rad,  nextPos);
    RadialToCartesian(nextAng, 1.0f, nextNormal);

    RadialToCartesian(thisAng, rad + topsideOffset, thispos2);
    RadialToCartesian(nextAng, rad + topsideOffset, nextPos2);

    float thisU = thisAng / 360.0f * uScale;
    float nextU = nextAng / 360.0f * uScale;

    glBegin(GL_QUADS);

    // the "inside"
    glNormal3fv(-thisNormal); glTexCoord2f(thisU, bottomUV); glVertex3fv(thispos);
    glNormal3fv(-nextNormal); glTexCoord2f(nextU, bottomUV); glVertex3fv(nextPos);
    glNormal3fv(-nextNormal); glTexCoord2f(nextU, topUV);    glVertex3fv(nextPos2);
    glNormal3fv(-thisNormal); glTexCoord2f(thisU, topUV);    glVertex3fv(thispos2);

    // the "outside"
    glNormal3fv(thisNormal); glTexCoord2f(thisU, topUV);    glVertex3fv(thispos2);
    glNormal3fv(nextNormal); glTexCoord2f(nextU, topUV);    glVertex3fv(nextPos2);
    glNormal3fv(nextNormal); glTexCoord2f(nextU, bottomUV); glVertex3fv(nextPos);
    glNormal3fv(thisNormal); glTexCoord2f(thisU, bottomUV); glVertex3fv(thispos);

    glEnd();

  }
}

static float clampedZ(float z, float offset) {
  if (z + offset > 0.0f) {
    return z;
  }
  return -offset;
}

static void drawRingZ(float innerRad, float outerRad,
                      float innerUV, float outerUV,
                      float ZOffset, int segments, float uScale) {
  for (int i = 0; i < segments; i ++) {
    float thisAng = 360.0f / segments * i;
    float nextAng = 360.0f / segments * (i + 1);
    if (i + 1 >= segments) {
      nextAng = 0;
    }

    fvec3 thisposR1;
    fvec3 thisposR2;
    fvec3 nextPosR1;
    fvec3 nextPosR2;

    RadialToCartesian(thisAng, innerRad, thisposR1);
    RadialToCartesian(thisAng, outerRad, thisposR2);
    RadialToCartesian(nextAng, innerRad, nextPosR1);
    RadialToCartesian(nextAng, outerRad, nextPosR2);


    glBegin(GL_QUADS);
    float thisU = thisAng / 360.0f * uScale;
    float nextU = nextAng / 360.0f * uScale;

    // the "left" Side
    glNormal3f(-1, 0, 0);
    glTexCoord2f(thisU, innerUV);
    glVertex3f(0, thisposR1[1], clampedZ(thisposR1[0], ZOffset));

    glTexCoord2f(nextU, innerUV);
    glVertex3f(0, nextPosR1[1], clampedZ(nextPosR1[0], ZOffset));

    glTexCoord2f(nextU, outerUV);
    glVertex3f(0, nextPosR2[1], clampedZ(nextPosR2[0], ZOffset));

    glTexCoord2f(thisU, outerUV);
    glVertex3f(0, thisposR2[1], clampedZ(thisposR2[0], ZOffset));

    // the "right" side
    glNormal3f(1, 0, 0);
    glTexCoord2f(thisU, innerUV);
    glVertex3f(0, thisposR1[1], clampedZ(thisposR1[0], ZOffset));

    glTexCoord2f(thisU, outerUV);
    glVertex3f(0, thisposR2[1], clampedZ(thisposR2[0], ZOffset));

    glTexCoord2f(nextU, outerUV);
    glVertex3f(0, nextPosR2[1], clampedZ(nextPosR2[0], ZOffset));

    glTexCoord2f(nextU, innerUV);
    glVertex3f(0, nextPosR1[1], clampedZ(nextPosR1[0], ZOffset));
    glEnd();
  }
}

static void drawRingYZ(float rad, float z, float topsideOffset, float bottomUV,
                       float ZOffset, float topUV, int segments, float uScale) {
  for (int i = 0; i < segments; i ++) {
    float thisAng = 360.0f / segments * i;
    float nextAng = 360.0f / segments * (i + 1);
    if (i + 1 >= segments) {
      nextAng = 0;
    }

    fvec3 thispos;
    fvec3 nextPos;
    fvec3 thispos2;
    fvec3 nextPos2;

    fvec3 thisNormal(0.0f, 0.0f, 0.0f);
    fvec3 nextNormal(0.0f, 0.0f, 0.0f);

    RadialToCartesian(thisAng, rad, thispos);
    RadialToCartesian(thisAng, 1, thisNormal);
    RadialToCartesian(nextAng, rad, nextPos);
    RadialToCartesian(nextAng, 1, nextNormal);

    RadialToCartesian(thisAng, rad + topsideOffset, thispos2);
    RadialToCartesian(nextAng, rad + topsideOffset, nextPos2);

    glBegin(GL_QUADS);
    float thisU = thisAng / 360.0f * uScale;
    float nextU = nextAng / 360.0f * uScale;

    // the "inside"
    glNormal3f(-thisNormal[0], -thisNormal[1], -thisNormal[2]);
    glTexCoord2f(thisU, bottomUV);
    glVertex3f(0, thispos[1], clampedZ(thispos[0], ZOffset));

    glNormal3f(-nextNormal[0], -nextNormal[1], -nextNormal[2]);
    glTexCoord2f(nextU, bottomUV);
    glVertex3f(0, nextPos[1], clampedZ(nextPos[0], ZOffset));

    glNormal3f(-nextNormal[0], -nextNormal[1], -nextNormal[2]);
    glTexCoord2f(nextU, topUV);
    glVertex3f(z, nextPos2[1], clampedZ(nextPos2[0], ZOffset));

    glNormal3f(-thisNormal[0], -thisNormal[1], -thisNormal[2]);
    glTexCoord2f(thisU, topUV);
    glVertex3f(z, thispos2[1], clampedZ(thispos2[0], ZOffset));

    // the "outside"
    glNormal3f(thisNormal[0], thisNormal[1], thisNormal[2]);
    glTexCoord2f(thisU, topUV);
    glVertex3f(z, thispos2[1], clampedZ(thispos2[0], ZOffset));

    glNormal3f(nextNormal[0], nextNormal[1], nextNormal[2]);
    glTexCoord2f(nextU, topUV);
    glVertex3f(z, nextPos2[1], clampedZ(nextPos2[0], ZOffset));

    glNormal3f(nextNormal[0], nextNormal[1], nextNormal[2]);
    glTexCoord2f(nextU, bottomUV);
    glVertex3f(0, nextPos[1], clampedZ(nextPos[0], ZOffset));

    glNormal3f(thisNormal[0], thisNormal[1], thisNormal[2]);
    glTexCoord2f(thisU, bottomUV);
    glVertex3f(0, thispos[1], clampedZ(thispos[0], ZOffset));

    glEnd();
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
