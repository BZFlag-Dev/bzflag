/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
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
#include "effectsRenderer.h"

// common impl headers
#include "TextureManager.h"
#include "StateDatabase.h"
#include "TimeKeeper.h"
#include "Flag.h"
#include "playing.h"



class StdSpawnEffect : public BasicEffect
{
public:
  StdSpawnEffect();
  virtual ~StdSpawnEffect();

  virtual bool update ( float time );
  virtual void draw ( const SceneRenderer& sr );

protected:
  int texture;
  OpenGLGState ringState;

  float radius;
};

class ConeSpawnEffect : public StdSpawnEffect
{
public:
  virtual bool update ( float time );
  virtual void draw ( const SceneRenderer& sr );
};

class RingSpawnEffect : public StdSpawnEffect
{
public:
  RingSpawnEffect();

  virtual bool update ( float time );
  virtual void draw ( const SceneRenderer& sr );

private:
  void drawRing(int n, float coreAlpha);

  float maxZ;
  float ringRange;
};

class StdShotEffect : public BasicEffect
{
public:
  StdShotEffect();
  virtual ~StdShotEffect();

  virtual bool update ( float time );
  virtual void draw ( const SceneRenderer& sr );

protected:
  int texture;
  OpenGLGState ringState;

  float radius;
};

class FlashShotEffect : public StdShotEffect
{
public:
  FlashShotEffect();

  virtual bool update ( float time );
  virtual void draw ( const SceneRenderer& sr );

private:
  float length;
};

class RingsDeathEffect : public DeathEffect
{
public:
  RingsDeathEffect();
  virtual ~RingsDeathEffect();

  virtual bool update ( float time );
  virtual void draw ( const SceneRenderer& sr );

protected:
  int texture;
  OpenGLGState ringState;

  float radius;
};

class SquishDeathEffect : public DeathEffect
{
public:
  SquishDeathEffect();

  virtual bool update ( float time );
  virtual void draw ( const SceneRenderer& sr );
  virtual bool SetDeathRenderParams ( TankDeathOverride::DeathParams &params);
  virtual bool GetDeathVector( fvec3 & v );
  virtual bool ShowExplosion ( void ) { return false; }
protected:
};

class FadeToHeaven : public DeathEffect
{
public:
  FadeToHeaven();

  virtual bool update ( float time );
  virtual void draw ( const SceneRenderer& sr );
  virtual bool SetDeathRenderParams ( TankDeathOverride::DeathParams &params);
  virtual bool GetDeathVector( fvec3 & v );
  virtual bool ShowExplosion ( void ) { return false; }
protected:
  float vertDist;
  bool done;
};

class SpikesDeathEffect : public DeathEffect
{
public:
  SpikesDeathEffect();
  virtual ~SpikesDeathEffect();

  virtual bool update ( float time );
  virtual void draw ( const SceneRenderer& sr );

protected:
  int texture;

  OpenGLGState spikeState;
  OpenGLGState smokeState;

  class Spike
  {
  public:
    float size;
    fvec2 rots;
    float alphaMod;
  };
  std::vector<Spike> Spikes;
  std::vector<fvec3> Puffs;

  float explodeFraction;
};


class StdLandEffect : public BasicEffect
{
public:
  StdLandEffect();
  virtual ~StdLandEffect();

  virtual bool update ( float time );
  virtual void draw ( const SceneRenderer& sr );

protected:
  int texture;
  OpenGLGState ringState;

  float radius;
};

class StdGMPuffEffect : public BasicEffect
{
public:
  StdGMPuffEffect();
  virtual ~StdGMPuffEffect();

  virtual bool update ( float time );
  virtual void draw ( const SceneRenderer& sr );
protected:
  int texture;
  OpenGLGState ringState;

  float radius;
};

class SmokeGMPuffEffect : public BasicEffect
{
public:
  SmokeGMPuffEffect();
  virtual ~SmokeGMPuffEffect();

  virtual bool update ( float time );
  virtual void draw ( const SceneRenderer& sr );

protected:
  int texture;
  OpenGLGState ringState;

  float radius;
  fvec3 jitter;

  float u,v,du,dv;
};

class StdRicoEffect : public BasicEffect
{
public:
  StdRicoEffect();
  virtual ~StdRicoEffect();

  virtual bool update ( float time );
  virtual void draw ( const SceneRenderer& sr );
protected:
  int texture;
  OpenGLGState ringState;

  float radius;
};

class StdShotTeleportEffect : public BasicEffect
{
public:
  StdShotTeleportEffect();
  virtual ~StdShotTeleportEffect();

  virtual bool update ( float time );
  virtual void draw ( const SceneRenderer& sr );
protected:
  int texture;
  OpenGLGState ringState;

  float radius;
};

template <>
EffectsRenderer* Singleton<EffectsRenderer>::_instance = (EffectsRenderer*)0;

// utils for geo
static void drawRingYZ(float rad, float z, float topsideOffset = 0,
  float bottomUV = 0, float ZOffset = 0,
  float topUV = 1.0f, int segments = 32);
static void drawRingXY(float rad, float z, float topsideOffset = 0,
  float bottomUV = 0, float topUV = 1.0f,
  int segments = 32);
static void RadialToCartesian(float angle, float rad, float *pos);

#define deg2Rad 0.017453292519943295769236907684886f


EffectsRenderer::EffectsRenderer()
{
}

EffectsRenderer::~EffectsRenderer()
{
  for ( unsigned int i = 0; i < effectsList.size(); i++ )
    delete(effectsList[i]);

  effectsList.clear();
}

void EffectsRenderer::init(void)
{
  for ( unsigned int i = 0; i < effectsList.size(); i++ )
    delete(effectsList[i]);

  effectsList.clear();
}

void EffectsRenderer::update(void)
{
  tvEffectsList::iterator itr = effectsList.begin();

  float time = (float)TimeKeeper::getCurrent().getSeconds();

  while ( itr != effectsList.end() )
  {
    if ( (*itr)->update(time) )
    {
      delete((*itr));
      itr = effectsList.erase(itr);
    }
    else
      ++itr;
  }
}

void EffectsRenderer::draw(const SceneRenderer& sr)
{
  // really should check here for only the things that are VISIBILE!!!

  for ( unsigned int i = 0; i < effectsList.size(); i++ )
    effectsList[i]->draw(sr);
}

void EffectsRenderer::freeContext(void)
{
  for ( unsigned int i = 0; i < effectsList.size(); i++ )
    effectsList[i]->freeContext();
}

void EffectsRenderer::rebuildContext(void)
{
  for ( unsigned int i = 0; i < effectsList.size(); i++ )
    effectsList[i]->rebuildContext();
}

void EffectsRenderer::addSpawnEffect ( const float* rgb, const float* pos )
{
  if (!BZDB.isTrue("useFancyEffects"))
    return;

  int flashType = static_cast<int>(BZDB.eval("spawnEffect"));

  if (flashType == 0)
    return;

  BasicEffect *effect = NULL;
  switch (flashType)
  {
  case 1:
    effect = new StdSpawnEffect;
    break;

  case 2:
    effect = new ConeSpawnEffect;
    break;

  case 3:
    effect = new RingSpawnEffect;
    break;
  }

  if (effect)
  {
    effect->setPos(pos,NULL);
    effect->setStartTime((float)TimeKeeper::getCurrent().getSeconds());
    effect->setColor(rgb);
    effectsList.push_back(effect);
  }
}

std::vector<std::string> EffectsRenderer::getSpawnEffectTypes ( void )
{
  std::vector<std::string> ret;
  ret.push_back(std::string("Off"));
  ret.push_back(std::string("Blossom"));
  ret.push_back(std::string("Cone"));
  ret.push_back(std::string("Rings"));

  return ret;
}

void EffectsRenderer::addShotEffect (  const float* rgb, const float* pos, float rot, const float *vel, int _type)
{
  if (!BZDB.isTrue("useFancyEffects"))
    return;

  int flashType = _type;
  if (flashType < 0)
    flashType = static_cast<int>(BZDB.eval("shotEffect"));

  if (flashType == 0)
    return;

  float rots[3] = {0};
  rots[2] = rot;

  BasicEffect *effect = NULL;
  switch (flashType)
  {
  case 1:
    effect = new StdShotEffect;
    break;
  case 2:
    effect = new FlashShotEffect;
    break;
  case 3:
    // composite effect
    addShotEffect(rgb, pos, rot, vel,1);
    addShotEffect(rgb, pos, rot, vel,2);
    break;
  }

  if (effect)
  {
    effect->setPos(pos,rots);
    effect->setStartTime((float)TimeKeeper::getCurrent().getSeconds());
    if (BZDB.isTrue("useVelOnShotEffects"))
      effect->setVel(vel);
    effect->setColor(rgb);

    effectsList.push_back(effect);
  }
}

std::vector<std::string> EffectsRenderer::getShotEffectTypes ( void )
{
  std::vector<std::string> ret;
  ret.push_back(std::string("Off"));
  ret.push_back(std::string("Smoke Rings"));
  ret.push_back(std::string("Muzzle Flash"));
  ret.push_back(std::string("Smoke and Flash"));

  return ret;
}

void EffectsRenderer::addGMPuffEffect ( const float* pos, float rot[2], const float* vel)
{
  if (!BZDB.isTrue("useFancyEffects"))
    return;

  int flashType = static_cast<int>(BZDB.eval("gmPuffEffect"));

  if (flashType == 0)
    return;

  float rots[3] = {0};
  rots[2] = rot[0];
  rots[1] = rot[1];

  BasicEffect *effect = NULL;
  switch (flashType)
  {
  case 1:
    // handled outside this manager in the "old" code
    break;

  case 2:
    effect = new StdGMPuffEffect;
    break;

  case 3:
    effect = new SmokeGMPuffEffect;
    break;
  }

  if (effect)
  {
    effect->setPos(pos,rots);
    effect->setStartTime((float)TimeKeeper::getCurrent().getSeconds());
    if (BZDB.isTrue("useVelOnShotEffects"))
      effect->setVel(vel);
    effectsList.push_back(effect);
  }
}

std::vector<std::string> EffectsRenderer::getGMPuffEffectTypes ( void )
{
  std::vector<std::string> ret;
  ret.push_back(std::string("Off"));
  ret.push_back(std::string("Classic Puff"));
  ret.push_back(std::string("Shock Cone"));
  ret.push_back(std::string("Smoke"));

  return ret;
}

DeathEffect* EffectsRenderer::addDeathEffect (const float *rgb, const float *pos, float rot,
  int, Player* player, FlagType *)
{
  if (!BZDB.isTrue("useFancyEffects"))
    return NULL;

  int effectType = static_cast<int>(BZDB.eval("deathEffect"));

  if (effectType == 0)
    return NULL;

  DeathEffect *effect = NULL;

  float rots[3] = {0};
  rots[2] = rot;

  // if (reason == GotKilledMsg)
  //   effect = new FadeToHeaven();
  // else if (reason == GotRunOver || flag == Flags::Steamroller)
  //   effect = new SquishDeathEffect;
  // else if (flag == Flags::GuidedMissile)
  //   effect = new SpikesDeathEffect;
  // else
  effect = new RingsDeathEffect;

  if (effect)
  {
    effect->setPlayer(player);
    effect->setPos(pos,rots);
    effect->setStartTime((float)TimeKeeper::getCurrent().getSeconds());
    effect->setColor(rgb);
    effectsList.push_back(effect);
  }
  return effect;
}

std::vector<std::string> EffectsRenderer::getDeathEffectTypes ( void )
{
  std::vector<std::string> ret;
  ret.push_back(std::string("Off"));
  ret.push_back(std::string("Fancy Deaths"));

  return ret;
}

// landing effects
void EffectsRenderer::addLandEffect ( const float* rgb, const float* pos, float rot )
{
  if (!BZDB.isTrue("useFancyEffects"))
    return;

  int effectType = static_cast<int>(BZDB.eval("landEffect"));

  if (effectType == 0)
    return;

  BasicEffect *effect = NULL;

  float rots[3] = {0};
  rots[2] = rot;

  switch (effectType)
  {
  case 1:
    effect = new StdLandEffect;
    break;
  }

  if (effect)
  {
    effect->setPos(pos,rots);
    effect->setStartTime((float)TimeKeeper::getCurrent().getSeconds());
    effect->setColor(rgb);
    effectsList.push_back(effect);
  }
}

std::vector<std::string> EffectsRenderer::getLandEffectTypes ( void )
{
  std::vector<std::string> ret;
  ret.push_back(std::string("Off"));
  ret.push_back(std::string("Dirt Flash"));

  return ret;
}

void EffectsRenderer::addRicoEffect ( const float* pos, float rot[2], const float* vel)
{
  if (!BZDB.isTrue("useFancyEffects"))
    return;

  int flashType = static_cast<int>(BZDB.eval("ricoEffect"));

  if (flashType == 0)
    return;

  float rots[3] = {0};
  rots[2] = rot[0];
  rots[1] = rot[1];

  BasicEffect *effect = NULL;
  switch (flashType)
  {
  case 1:
    effect = new StdRicoEffect;
    break;
  }

  if (effect)
  {
    effect->setPos(pos,rots);
    effect->setStartTime((float)TimeKeeper::getCurrent().getSeconds());
    if (BZDB.isTrue("useVelOnShotEffects"))
      effect->setVel(vel);

    effectsList.push_back(effect);
  }
}

std::vector<std::string> EffectsRenderer::getRicoEffectTypes ( void )
{
  std::vector<std::string> ret;
  ret.push_back(std::string("Off"));
  ret.push_back(std::string("Ring"));
  // ret.push_back(std::string("Sparks"));

  return ret;
}

void EffectsRenderer::addShotTeleportEffect ( const float* pos, float rot[2], const float* vel)
{
  if (!BZDB.isTrue("useFancyEffects"))
    return;

  int flashType = static_cast<int>(BZDB.eval("tpEffect"));

  if (flashType == 0)
    return;

  float rots[3] = {0};
  rots[2] = rot[0];
  rots[1] = rot[1];

  BasicEffect *effect = NULL;
  switch (flashType)
  {
  case 1:
    effect = new StdShotTeleportEffect;
    break;
  }

  if (effect)
  {
    effect->setPos(pos,rots);
    effect->setStartTime((float)TimeKeeper::getCurrent().getSeconds());
    if (BZDB.isTrue("useVelOnShotEffects"))
      effect->setVel(vel);
    effectsList.push_back(effect);
  }
}

std::vector<std::string> EffectsRenderer::getShotTeleportEffectTypes ( void )
{
  std::vector<std::string> ret;
  ret.push_back(std::string("None"));
  ret.push_back(std::string("IDL"));
  // ret.push_back(std::string("Sparks"));

  return ret;
}



//****************** effects base class*******************************
BasicEffect::BasicEffect(): age()
{
  position[0] = position[1] = position[2] = 0.0f;
  rotation[0] = rotation[1] = rotation[2] = 0.0f;
  velocity[0] = velocity[1] = velocity[2] = 0.0f;
  color[0] = color[1] = color[2] = 0.0f;
  startTime = (float)TimeKeeper::getCurrent().getSeconds();

  lifetime = 0;
  lastTime = startTime;
  deltaTime = 0;
  lifeParam = 1.0;
}

void BasicEffect::setPos ( const float *pos, const float *rot )
{
  if (pos)
  {
    position[0] = pos[0];
    position[1] = pos[1];
    position[2] = pos[2];
  }

  if (rot)
  {
    rotation[0] = rot[0];
    rotation[1] = rot[1];
    rotation[2] = rot[2];
  }
}

void BasicEffect::setVel ( const float *vel )
{
  if (vel)
  {
    velocity[0] = vel[0];
    velocity[1] = vel[1];
    velocity[2] = vel[2];
  }
}

void BasicEffect::setColor ( const float *rgb )
{
  color[0] = rgb[0];
  color[1] = rgb[1];
  color[2] = rgb[2];
}

void BasicEffect::setStartTime ( float time )
{
  startTime = time;
  lastTime = time;
  deltaTime = 0;
}

bool BasicEffect::update( float time )
{
  age = time - startTime;

  if ( age >= lifetime)
    return true;

  deltaTime = time - lastTime;
  lastTime = time;

  if (lifetime != 0 && age != 0)
    lifeParam = age/lifetime;

  return false;
}

//******************StdSpawnEffect****************
StdSpawnEffect::StdSpawnEffect() : BasicEffect()
{
  texture = TextureManager::instance().getTextureID("blend_flash",false);
  lifetime = 2.0f;
  radius = 1.75f;

  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA,(GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  if (texture >-1)
    gstate.setTexture(texture);

  ringState = gstate.getState();
}

StdSpawnEffect::~StdSpawnEffect()
{
}

bool StdSpawnEffect::update ( float time )
{
  // see if it's time to die
  // if not update all those fun times
  if ( BasicEffect::update(time))
    return true;

  // nope it's not.
  // we live another day
  // do stuff that maybe need to be done every time to animage

  radius += deltaTime*5;
  return false;
}

void StdSpawnEffect::draw(const SceneRenderer &)
{
  glPushMatrix();

  glTranslatef(position[0],position[1],position[2]+0.1f);

  ringState.setState();

  float ageParam = age/lifetime;

  glColor4f(color[0],color[1],color[2],1.0f-(age/lifetime));
  glDepthMask(0);

  drawRingXY(radius*0.1f,2.5f+(age*2));
  drawRingXY(radius*0.5f,1.5f + (ageParam/1.0f * 2),0.5f,0.5f);
  drawRingXY(radius,2);

  glColor4f(1,1,1,1);
  glDepthMask(1);
  glPopMatrix();
}

//******************ConeSpawnEffect****************
bool ConeSpawnEffect::update ( float time )
{
  // see if it's time to die
  // if not update all those fun times
  if ( BasicEffect::update(time))
    return true;

  // nope it's not.
  // we live another day
  // do stuff that maybe need to be done every time to animage

  radius += deltaTime*5;
  return false;
}

void ConeSpawnEffect::draw(const SceneRenderer &)
{
  glPushMatrix();

  glTranslatef(position[0],position[1],position[2]+0.1f);

  ringState.setState();

  glColor4f(color[0],color[1],color[2],1.0f-(age/lifetime));
  glDepthMask(0);

  drawRingXY(radius*0.5f,1.25f);

  glTranslatef(0,0,2);
  drawRingXY(radius*0.6f,1.5f);

  glTranslatef(0,0,2);
  drawRingXY(radius*0.75f,1.75f);

  glTranslatef(0,0,2);
  drawRingXY(radius*0.85f,1.89f);

  glTranslatef(0,0,2);
  drawRingXY(radius,2.0f);

  glColor4f(1,1,1,1);
  glDepthMask(1);
  glPopMatrix();
}


//******************RingSpawnEffect****************
RingSpawnEffect::RingSpawnEffect(): ringRange()
{
  radius = 4.0f;
  maxZ = 10.0f;
}

bool RingSpawnEffect::update ( float time )
{
  // see if it's time to die
  // if not update all those fun times
  if ( BasicEffect::update(time))
    return true;

  // nope it's not.
  // we live another day
  // do stuff that maybe need to be done every time to animage
  return false;
}

void RingSpawnEffect::draw(const SceneRenderer &)
{
  glPushMatrix();

  glTranslatef(position[0],position[1],position[2]);

  ringState.setState();

  glDepthMask(0);

  ringRange = lifetime / 4.0f;  // first 3/4ths of the life are rings, last is fade
  ringRange = (ringRange * 3) / 4.0f; // of the ring section there are 4 ring segments

  const float bigRange = ringRange * 3;

  float coreAlpha = 1;
  if (age >= bigRange)
    coreAlpha = 1.0f - ((age - bigRange) / (lifetime - bigRange));

  for (int n = 0; n < 4; ++n)
    drawRing(n, coreAlpha);

  glColor4f(1,1,1,1);
  glDepthMask(1);
  glPopMatrix();
}

void RingSpawnEffect::drawRing(int n, float coreAlpha)
{
  float posZ;
  float alpha;

  if (age <= (ringRange * (n-1)))  // this ring in?
    return;

  if (age < ringRange * n) { // the ring is still coming in
    posZ = maxZ - ((age - ringRange * (n-1)) / ringRange) * (maxZ - n * 2.5f);
    alpha = (age - ringRange) / (ringRange * n);
    } else {
    posZ = n * 2.5f;
    alpha = coreAlpha;
  }

  glPushMatrix();
  glTranslatef(0, 0, posZ);
  glColor4f(color[0], color[1], color[2], alpha);
  drawRingXY(radius, 2.5f * n);
  glPopMatrix();
}

//******************StdShotEffect****************
StdShotEffect::StdShotEffect() : BasicEffect()
{
  texture = TextureManager::instance().getTextureID("blend_flash",false);
  lifetime = 1.5f;
  radius = 0.125f;


  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA,(GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  if (texture >-1)
    gstate.setTexture(texture);

  ringState = gstate.getState();
}

StdShotEffect::~StdShotEffect()
{
}

bool StdShotEffect::update ( float time )
{
  // see if it's time to die
  // if not update all those fun times
  if ( BasicEffect::update(time))
    return true;

  // nope it's not.
  // we live another day
  // do stuff that maybe need to be done every time to animage

  radius += deltaTime*6;
  return false;
}

void StdShotEffect::draw(const SceneRenderer &)
{
  glPushMatrix();

  float pos[3];

  pos[0] = position[0] + velocity[0] * age;
  pos[1] = position[1] + velocity[1] * age;
  pos[2] = position[2] + velocity[2] * age;

  glTranslatef(pos[0],pos[1],pos[2]);
  glRotatef(180+rotation[2]/deg2Rad,0,0,1);

  //TODO: _muzzleFront and _muzzleHeight (4.42 and 1.57) should be
  // the same as the tank model's muzzle (4.94 and 1.53).
  // FlashShot is also affected by this todo.
  glTranslatef(-0.52f, 0.0f, -0.04f);

  ringState.setState();

  color[0] = color[1] = color[2] = 1;

  float alpha = 0.5f-(age/lifetime);
  if (alpha < 0.001f)
    alpha = 0.001f;

  glColor4f(color[0],color[1],color[2],alpha);
  glDepthMask(0);

  drawRingYZ(radius,0.5f /*+ (age * 0.125f)*/,1.0f+age*5,0.65f,pos[2]);

  glColor4f(1,1,1,1);
  glDepthMask(1);
  glPopMatrix();
}

//******************FlashShotEffect****************
FlashShotEffect::FlashShotEffect() : StdShotEffect(), length()
{
  // we use the jump jet texture upside-down to get a decent muzzle flare effect
  texture = TextureManager::instance().getTextureID("jumpjets",false);
  lifetime = 0.75f;
  radius = 0.5f;

  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA,(GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  if (texture >-1)
    gstate.setTexture(texture);

  ringState = gstate.getState();
}

bool FlashShotEffect::update ( float time )
{
  // see if it's time to die
  // if not update all those fun times
  if (BasicEffect::update(time))
    return true;

  // nope it's not.
  // we live another day
  // do stuff that maybe need to be done every time to animage
  if (age < lifetime / 2)
    length = 6 * (age / lifetime);
  else
    length = 6 * (1 - (age / lifetime));

  return false;
}

void FlashShotEffect::draw(const SceneRenderer &)
{
  if (!LocalPlayer::getMyTank()) {
    //just left the game
    return;
  }

  glPushMatrix();

  float pos[3];

  pos[0] = position[0] + velocity[0] * age;
  pos[1] = position[1] + velocity[1] * age;
  pos[2] = position[2] + velocity[2] * age;

  glTranslatef(pos[0],pos[1],pos[2]);
  glRotatef(270+rotation[2]/deg2Rad,0,0,1);
  glTranslatef(0.0f, 0.52f, -0.04f);

  //barrel roll to camera
  const float *playerpos = LocalPlayer::getMyTank()->getPosition();
  float camerapos[3] = {
    playerpos[0] - pos[0],
    playerpos[1] - pos[1],
    playerpos[2] - pos[2]
  };
  //camerapos[0] = camerapos[0] * cos(-rotation[2])
  //		 - camerapos[1] * sin(-rotation[2]);
  camerapos[1] = camerapos[1] * cos(-rotation[2])
    + camerapos[0] * sin(-rotation[2]);
  glRotatef(270 - atan(camerapos[1] / camerapos[2]) / deg2Rad +
    (camerapos[2] >= 0 ? 180 : 0), //for a single-sided face
    0,1,0);

  ringState.setState();

  color[0] = color[1] = color[2] = 1;

  float alpha = 0.8f-(age/lifetime);
  if (alpha < 0.001f)
    alpha = 0.001f;

  glColor4f(color[0],color[1],color[2],alpha);
  glDepthMask(0);

  // draw me here
  glBegin(GL_QUADS);

  glTexCoord2f(0,1);
  glVertex3f(0,0,radius);

  glTexCoord2f(0,0);
  glVertex3f(0,length,radius);

  glTexCoord2f(1,0);
  glVertex3f(0,length,-radius);

  glTexCoord2f(1,1);
  glVertex3f(0,0,-radius);

  glEnd();

  glColor4f(1,1,1,1);
  glDepthMask(1);
  glPopMatrix();
}

//******************SquishDeathEffect****************
SquishDeathEffect::SquishDeathEffect() : DeathEffect()
{
  lifetime = 10.0f;
}

bool SquishDeathEffect::update ( float time )
{
  // see if it's time to die
  // if not update all those fun times
  if ( BasicEffect::update(time))
    return true;

  return false;
}
void SquishDeathEffect::draw ( const SceneRenderer& UNUSED(sr) )
{}

bool SquishDeathEffect::SetDeathRenderParams (TankDeathOverride::DeathParams &params)
{
  params.scale = fvec3(1,1,0);
  params.pos = fvec3(0,0,0.1f);
  return true;
}

bool SquishDeathEffect::GetDeathVector( fvec3 & vel )
{
  if (!player)
    return false;

  const float *v = player->getVelocity();
  vel = fvec3(v[0],v[1],v[2]);
  return true;
}


//******************FadeToHeaven****************
FadeToHeaven::FadeToHeaven() : DeathEffect()
{
  lifetime = BZDB.eval(StateDatabase::BZDB_EXPLODETIME);
  vertDist = 5.0;
  done = false;
}

bool FadeToHeaven::update ( float time )
{
  if (!done)
    return true;

  // see if it's time to die
  // if not update all those fun times
  if ( BasicEffect::update(time))
    return true;

  return false;
}

void FadeToHeaven::draw ( const SceneRenderer& UNUSED(sr) )
{
}

bool FadeToHeaven::SetDeathRenderParams (TankDeathOverride::DeathParams &params)
{
  done = params.explodeParam  < 0.0001f;

  params.color[3] = params.explodeParam;
  params.pos = fvec3(0,0,vertDist*params.explodeParam);

  return true;
}

bool FadeToHeaven::GetDeathVector( fvec3 & vel )
{
  if (!player)
    return false;
  vel = fvec3(0,0,0);
  return true;
}


//******************RingsDeathEffect****************
RingsDeathEffect::RingsDeathEffect() : DeathEffect()
{
  texture = TextureManager::instance().getTextureID("blend_flash",false);
  lifetime = 1.5f;
  radius = 2.0f;

  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA,(GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  if (texture >-1)
    gstate.setTexture(texture);

  ringState = gstate.getState();
}

RingsDeathEffect::~RingsDeathEffect()
{
}

bool RingsDeathEffect::update ( float time )
{
  // see if it's time to die
  // if not update all those fun times
  if ( BasicEffect::update(time))
    return true;

  // nope it's not.
  // we live another day
  // do stuff that maybe need to be done every time to animage

  radius += deltaTime*20;
  return false;
}

void RingsDeathEffect::draw(const SceneRenderer &)
{
  glPushMatrix();

  glTranslatef(position[0],position[1],position[2]);
  glRotatef(180+rotation[2]/deg2Rad,0,0,1);

  ringState.setState();

  color[0] = 108.0f/256.0f;
  color[1] = 16.0f/256.0f;
  color[2] = 16.0f/256.0f;

  float deltas[3];

  deltas[0] = 1.0f - color[0];
  deltas[1] = 1.0f - color[1];
  deltas[2] = 1.0f - color[2];

  float ageParam = age/lifetime;

  float alpha = 1.0f-(ageParam*0.5f);
  if (alpha < 0.005f)
    alpha = 0.005f;

  color[0] += deltas[0] *ageParam;
  color[1] += deltas[1] *ageParam;
  color[2] += deltas[2] *ageParam;

  glColor4f(color[0],color[1],color[2],alpha);
  glDepthMask(0);

  glPushMatrix();
  glTranslatef(0,0,0.5f);
  drawRingXY(radius*0.75f,1.5f + (ageParam/1.0f * 10),0.5f*age,0.5f);
  drawRingXY(radius,-0.5f,0.5f+ age,0.5f);

  glTranslatef(-1.5,0,0);
  glRotatef(90,0,0,1);
  drawRingYZ(radius,3,0,0,position[2]+0.5f);
  glPopMatrix();

  glColor4f(1,1,1,1);
  glDepthMask(1);
  glPopMatrix();
}


//******************SpikesDeathEffect****************
SpikesDeathEffect::SpikesDeathEffect() : DeathEffect()
{
  texture = TextureManager::instance().getTextureID("puff",false);
  lifetime = 4.5f;
  explodeFraction = 0.5f;

  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA,(GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  spikeState = gstate.getState();

  if (texture >-1)
    gstate.setTexture(texture);

  smokeState = gstate.getState();

  int spikes = (int)((bzfrand() * 25)) + 10;

  for ( int i = 0; i < spikes; i++)
  {
    Spike s;
    s.alphaMod = ((float)bzfrand() * 0.5f) + 0.5f;
    s.size = ((float)bzfrand() * 1.5f) + 0.5f;
    s.rots = fvec2((float)bzfrand()* 360.0f,(float)bzfrand() * 180.0f);
    Spikes.push_back(s);
  }

  int puffs = (int)((bzfrand() * 25)) + 10;
  for ( int i = 0; i < puffs; i++)
    Puffs.push_back(fvec3((float)bzfrand()*4-2,(float)bzfrand()*4-2,(float)bzfrand()*2));
}

SpikesDeathEffect::~SpikesDeathEffect()
{
}

bool SpikesDeathEffect::update ( float time )
{
  // see if it's time to die
  // if not update all those fun times
  if ( BasicEffect::update(time))
    return true;

  // nope it's not.
  // we live another day
  // do stuff that maybe need to be done every time to animage
  return false;
}

void SpikesDeathEffect::draw(const SceneRenderer &)
{
  glPushMatrix();

  glTranslatef(position[0],position[1],position[2]);
  glRotatef(180+rotation[2]/deg2Rad,0,0,1);

  color[0] = 108.0f/256.0f;
  color[1] = 16.0f/256.0f;
  color[2] = 16.0f/256.0f;

/*float deltas[3];

  deltas[0] = 1.0f - color[0];
  deltas[1] = 1.0f - color[1];
  deltas[2] = 1.0f - color[2];

  float ageParam = age/lifetime;

  float alpha = 1.0f-(ageParam*0.5f);
  if (alpha < 0.005f)
  alpha = 0.005f;

  color[0] += deltas[0] *ageParam;
  color[1] += deltas[1] *ageParam;
  color[2] += deltas[2] *ageParam;

  glColor4f(color[0],color[1],color[2],alpha);
  glDepthMask(0);

  glPushMatrix();
  glTranslatef(0,0,0.5f);
  drawRingXY(radius*0.75f,1.5f + (ageParam/1.0f * 10),0.5f*age,0.5f);
  drawRingXY(radius,-0.5f,0.5f+ age,0.5f);

  glTranslatef(-1.5,0,0);
  glRotatef(90,0,0,1);
  drawRingYZ(radius,3,0,0,position[2]+0.5f);
  glPopMatrix();

  glColor4f(1,1,1,1);
  glDepthMask(1);*/
  glPopMatrix();
}

//******************StdLandEffect****************
StdLandEffect::StdLandEffect() : BasicEffect()
{
  texture = TextureManager::instance().getTextureID("dusty_flare",false);
  lifetime = 1.0f;
  radius = 2.5f;

  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA,(GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  if (texture >-1)
    gstate.setTexture(texture);

  ringState = gstate.getState();
}

StdLandEffect::~StdLandEffect()
{
}

bool StdLandEffect::update ( float time )
{
  // see if it's time to die
  // if not update all those fun times
  if ( BasicEffect::update(time))
    return true;

  // nope it's not.
  // we live another day
  // do stuff that maybe need to be done every time to animage

  radius += deltaTime * 3.5f;
  return false;
}

void StdLandEffect::draw(const SceneRenderer &)
{
  glPushMatrix();

  glTranslatef(position[0],position[1],position[2]);

  ringState.setState();

  color[0] = 1;
  color[1] = 1;
  color[2] = 1;

  glColor4f(color[0],color[1],color[2],1.0f-(age/lifetime));
  glDepthMask(0);

  drawRingXY(radius,0.5f + age,0.05f*radius,0.0f,0.9f);

  glColor4f(1,1,1,1);
  glDepthMask(1);
  glPopMatrix();
}

//******************StdGMPuffEffect****************
StdGMPuffEffect::StdGMPuffEffect() : BasicEffect()
{
  texture = TextureManager::instance().getTextureID("blend_flash",false);
  lifetime = 6.5f;

  radius = 0.125f;
  if (RENDERER.useQuality() >= 3)
    radius = 0.001f;


  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA,(GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  if (texture >-1)
    gstate.setTexture(texture);

  ringState = gstate.getState();
}

StdGMPuffEffect::~StdGMPuffEffect()
{
}

bool StdGMPuffEffect::update ( float time )
{
  // see if it's time to die
  // if not update all those fun times
  if ( BasicEffect::update(time))
    return true;

  // nope it's not.
  // we live another day
  // do stuff that maybe need to be done every time to animage

  radius += deltaTime*0.5f;
  return false;
}

void StdGMPuffEffect::draw(const SceneRenderer &)
{
  glPushMatrix();

  float pos[3];

  pos[0] = position[0] + velocity[0] * age;
  pos[1] = position[1] + velocity[1] * age;
  pos[2] = position[2] + velocity[2] * age;

  glTranslatef(pos[0],pos[1],pos[2]);
  glRotatef(180+rotation[2]/deg2Rad,0,0,1);
  glRotatef(rotation[1]/deg2Rad,0,1,0);

  ringState.setState();

  color[0] = color[1] = color[2] = 1;

  float alpha = 0.5f-(age/lifetime);
  if (alpha < 0.000001f)
    alpha = 0.000001f;

  glColor4f(color[0],color[1],color[2],alpha);
  glDepthMask(0);

  drawRingYZ(radius,-0.25f -(age * 0.125f),0.5f+age*0.75f,0.50f,pos[2]);

  glColor4f(1,1,1,1);
  glDepthMask(1);
  glPopMatrix();
}


//******************StdGMPuffEffect****************
SmokeGMPuffEffect::SmokeGMPuffEffect() : BasicEffect()
{
  texture = TextureManager::instance().getTextureID("puffs",false);
  lifetime = 3.5f;

  radius = 0.125f;
  if (RENDERER.useQuality() >= 3)
    radius = 0.001f;

  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA,(GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  if (texture >-1)
    gstate.setTexture(texture);

  ringState = gstate.getState();

  float randMod = 0.5f;

  jitter.x = ((float)bzfrand() * (randMod*2)) - randMod;
  jitter.y = ((float)bzfrand() * (randMod*2)) - randMod;
  jitter.z = ((float)bzfrand() * (randMod*2)) - randMod;

  du = dv = 0.5f;

  if (bzfrand() < 0.5)
    u = 0;
  else
    u = 0.5f;

  if (bzfrand() < 0.5)
    v = 0;
  else
    v = 0.5f;

}

SmokeGMPuffEffect::~SmokeGMPuffEffect()
{
}

bool SmokeGMPuffEffect::update ( float time )
{
  // see if it's time to die
  // if not update all those fun times
  if ( BasicEffect::update(time))
    return true;

  // nope it's not.
  // we live another day
  // do stuff that maybe need to be done every time to animage

  radius += deltaTime*0.5f;
  return false;
}

void QuadGuts ( float u0, float v0, float u1, float v1, float h, float v)
{
  glTexCoord2f(u0, v0); glVertex2f(-h, -v);
  glTexCoord2f(u1, v0); glVertex2f(+h, -v);
  glTexCoord2f(u1, v1); glVertex2f(+h, +v);
  glTexCoord2f(u0, v1); glVertex2f(-h, +v);
}

void DrawTextureQuad ( float u0, float v0, float u1, float v1, float h, float v)
{
  glBegin(GL_QUADS);
  QuadGuts(u0,v0,u1,v1,h,v);
  glEnd();
}

void SmokeGMPuffEffect::draw(const SceneRenderer &)
{
  glPushMatrix();

  float pos[3];

  float vertDrift = 1.5f * age;

  pos[0] = position[0] + velocity[0] * age;
  pos[1] = position[1] + velocity[1] * age;
  pos[2] = position[2] + velocity[2] * age;

  glTranslatef(pos[0]+jitter.x,pos[1]+jitter.y,pos[2]+jitter.z+vertDrift);

  glPushMatrix();
  RENDERER.getViewFrustum().executeBillboard();
  glRotatef(age*180,0,0,1);

  ringState.setState();
  glColor4f(1,1,1,1);

  color[0] = color[1] = color[2] = 1;

  float alpha = 0.5f-(age/lifetime);
  if (alpha < 0.000001f)
    alpha = 0.000001f;

  glColor4f(1,1,1,alpha);
  glDepthMask(0);

  float size = 0.5f + (age * 1.25f);

  DrawTextureQuad ( (float)u, (float)v, (float)u + du, (float)v + dv, size, size);

  glPopMatrix();
  glDepthMask(1);
  glPopMatrix();
}

//******************StdRicoEffect****************
StdRicoEffect::StdRicoEffect() : BasicEffect()
{
  texture = TextureManager::instance().getTextureID("blend_flash",false);
  lifetime = 0.5f;
  radius = 0.25f;

  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA,(GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  if (texture >-1)
    gstate.setTexture(texture);

  ringState = gstate.getState();
}

StdRicoEffect::~StdRicoEffect()
{
}

bool StdRicoEffect::update ( float time )
{
  // see if it's time to die
  // if not update all those fun times
  if ( BasicEffect::update(time))
    return true;

  // nope it's not.
  // we live another day
  // do stuff that maybe need to be done every time to animage

  radius += deltaTime*6.5f;
  return false;
}

void StdRicoEffect::draw(const SceneRenderer &)
{
  glPushMatrix();

  float pos[3];

  pos[0] = position[0] + velocity[0] * age;
  pos[1] = position[1] + velocity[1] * age;
  pos[2] = position[2] + velocity[2] * age;

  glTranslatef(pos[0],pos[1],pos[2]);
  glRotatef((rotation[2]/deg2Rad)+180,0,0,1);
  glRotatef(rotation[1]/deg2Rad,0,1,0);

  ringState.setState();

  color[0] = color[1] = color[2] = 1;

  float alpha = 0.5f-(age/lifetime);
  if (alpha < 0.000001f)
    alpha = 0.000001f;

  glColor4f(color[0],color[1],color[2],alpha);
  glDepthMask(0);

  drawRingYZ(radius,-0.5f,0.5f,0.50f,pos[2]);

  glColor4f(1,1,1,1);
  glDepthMask(1);
  glPopMatrix();
}

//******************StdShotTeleportEffect****************
StdShotTeleportEffect::StdShotTeleportEffect() : BasicEffect()
{
  texture = TextureManager::instance().getTextureID("dusty_flare",false);
  lifetime = 4.0f;
  radius = 0.25f;


  OpenGLGStateBuilder gstate;
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum) GL_SRC_ALPHA,(GLenum) GL_ONE_MINUS_SRC_ALPHA);
  gstate.setAlphaFunc();

  if (texture >-1)
    gstate.setTexture(texture);

  ringState = gstate.getState();
}

StdShotTeleportEffect::~StdShotTeleportEffect()
{
}

bool StdShotTeleportEffect::update ( float time )
{
  // see if it's time to die
  // if not update all those fun times
  if ( BasicEffect::update(time))
    return true;

  // nope it's not.
  // we live another day
  // do stuff that maybe need to be done every time to animage

  //radius += deltaTime*6.5f;
  return false;
}

void StdShotTeleportEffect::draw(const SceneRenderer &)
{
  glPushMatrix();

  float pos[3];

  pos[0] = position[0] + velocity[0] * age;
  pos[1] = position[1] + velocity[1] * age;
  pos[2] = position[2] + velocity[2] * age;

  glTranslatef(pos[0],pos[1],pos[2]);
  glRotatef((rotation[2]/deg2Rad),0,0,1);
  glRotatef(rotation[1]/deg2Rad,0,1,0);
  glRotatef(age*90,1,0,0);

  ringState.setState();

  color[0] = color[1] = color[2] = 1;

  float alpha = 1.0f;

  glColor4f(color[0],color[1],color[2],alpha);
  glDepthMask(0);

  float mod = age-(int)age;
  mod -= 0.5f;

  drawRingYZ(radius,0.5f + mod*0.5f,0.125f,0.00f,pos[2],0.8f,6);

  glColor4f(1,1,1,1);
  glDepthMask(1);
  glPopMatrix();
}

//******************************** geo utiliys********************************

static void RadialToCartesian(float angle, float rad, float *pos)
{
  pos[0] = sinf(angle*deg2Rad)*rad;
  pos[1] = cosf(angle*deg2Rad)*rad;
}

static void drawRingXY(float rad, float z, float topsideOffset, float bottomUV,
  float topUV, int segments )
{
  for ( int i = 0; i < segments; i ++)
  {
    float thisAng = 360.0f/segments * i;
    float nextAng = 360.0f/segments * (i+1);
    if ( i+1 >= segments )
      nextAng = 0;

    float thispos[2];
    float nextPos[2];

    float thispos2[2];
    float nextPos2[2];

    float thisNormal[3] = {0};
    float nextNormal[3] = {0};

    RadialToCartesian(thisAng,rad,thispos);
    RadialToCartesian(thisAng,1,thisNormal);
    RadialToCartesian(nextAng,rad,nextPos);
    RadialToCartesian(nextAng,1,nextNormal);

    RadialToCartesian(thisAng,rad+topsideOffset,thispos2);
    RadialToCartesian(nextAng,rad+topsideOffset,nextPos2);

    glBegin(GL_QUADS);

    // the "inside"
    glNormal3f(-thisNormal[0],-thisNormal[1],-thisNormal[2]);
    glTexCoord2f(0,bottomUV);
    glVertex3f(thispos[0],thispos[1],0);

    glNormal3f(-nextNormal[0],-nextNormal[1],-nextNormal[2]);
    glTexCoord2f(1,bottomUV);
    glVertex3f(nextPos[0],nextPos[1],0);

    glNormal3f(-nextNormal[0],-nextNormal[1],-nextNormal[2]);
    glTexCoord2f(1,topUV);
    glVertex3f(nextPos2[0],nextPos2[1],z);

    glNormal3f(-thisNormal[0],-thisNormal[1],-thisNormal[2]);
    glTexCoord2f(0,topUV);
    glVertex3f(thispos2[0],thispos2[1],z);

    // the "outside"

    glNormal3f(thisNormal[0],thisNormal[1],thisNormal[2]);
    glTexCoord2f(0,topUV);
    glVertex3f(thispos2[0],thispos2[1],z);

    glNormal3f(nextNormal[0],nextNormal[1],nextNormal[2]);
    glTexCoord2f(1,topUV);
    glVertex3f(nextPos2[0],nextPos2[1],z);

    glNormal3f(nextNormal[0],nextNormal[1],nextNormal[2]);
    glTexCoord2f(1,bottomUV);
    glVertex3f(nextPos[0],nextPos[1],0);

    glNormal3f(thisNormal[0],thisNormal[1],thisNormal[2]);
    glTexCoord2f(0,bottomUV);
    glVertex3f(thispos[0],thispos[1],0);

    glEnd();

  }
}

static float clampedZ(float z, float offset)
{
  if ( z +offset > 0.0f)
    return z;
  return -offset;
}

static void drawRingYZ(float rad, float z, float topsideOffset, float bottomUV,
  float ZOffset, float topUV, int segments)
{
  for ( int i = 0; i < segments; i ++)
  {
    float thisAng = 360.0f/segments * i;
    float nextAng = 360.0f/segments * (i+1);
    if ( i+1 >= segments )
      nextAng = 0;

    float thispos[2];
    float nextPos[2];

    float thispos2[2];
    float nextPos2[2];

    float thisNormal[3] = {0};
    float nextNormal[3] = {0};

    RadialToCartesian(thisAng,rad,thispos);
    RadialToCartesian(thisAng,1,thisNormal);
    RadialToCartesian(nextAng,rad,nextPos);
    RadialToCartesian(nextAng,1,nextNormal);

    RadialToCartesian(thisAng,rad+topsideOffset,thispos2);
    RadialToCartesian(nextAng,rad+topsideOffset,nextPos2);

    glBegin(GL_QUADS);

    // the "inside"
    glNormal3f(-thisNormal[0],-thisNormal[1],-thisNormal[2]);
    glTexCoord2f(0,bottomUV);
    glVertex3f(0,thispos[1],clampedZ(thispos[0],ZOffset));

    glNormal3f(-nextNormal[0],-nextNormal[1],-nextNormal[2]);
    glTexCoord2f(1,bottomUV);
    glVertex3f(0,nextPos[1],clampedZ(nextPos[0],ZOffset));

    glNormal3f(-nextNormal[0],-nextNormal[1],-nextNormal[2]);
    glTexCoord2f(1,topUV);
    glVertex3f(z,nextPos2[1],clampedZ(nextPos2[0],ZOffset));

    glNormal3f(-thisNormal[0],-thisNormal[1],-thisNormal[2]);
    glTexCoord2f(0,topUV);
    glVertex3f(z,thispos2[1],clampedZ(thispos2[0],ZOffset));

    // the "outside"

    glNormal3f(thisNormal[0],thisNormal[1],thisNormal[2]);
    glTexCoord2f(0,topUV);
    glVertex3f(z,thispos2[1],clampedZ(thispos2[0],ZOffset));

    glNormal3f(nextNormal[0],nextNormal[1],nextNormal[2]);
    glTexCoord2f(1,topUV);
    glVertex3f(z,nextPos2[1],clampedZ(nextPos2[0],ZOffset));

    glNormal3f(nextNormal[0],nextNormal[1],nextNormal[2]);
    glTexCoord2f(1,bottomUV);
    glVertex3f(0,nextPos[1],clampedZ(nextPos[0],ZOffset));

    glNormal3f(thisNormal[0],thisNormal[1],thisNormal[2]);
    glTexCoord2f(0,bottomUV);
    glVertex3f(0,thispos[1],clampedZ(thispos[0],ZOffset));

    glEnd();
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
