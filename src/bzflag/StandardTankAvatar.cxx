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

// interface header
#include "StandardTankAvatar.h"
#include "BZDBCache.h"
#include "SceneRenderer.h"
#include "TextureManager.h"
#include "Team.h"
#include "OpenGLMaterial.h"


StandardTankAvatar::StandardTankAvatar ( int /* playerID */, const fvec3& pos, const fvec3& forward )
{
  tankNode = new TankSceneNode(pos,forward);
  IDLNode = new TankIDLSceneNode(tankNode);

  const float sphereRad = (1.5f * BZDBCache::tankRadius);
  if (RENDERER.useQuality() >= _HIGH_QUALITY) {
    pausedSphere = new SphereLodSceneNode(pos, sphereRad);
  } else {
    pausedSphere = new SphereBspSceneNode(pos, sphereRad);
  }
  pausedSphere->setColor(0.0f, 0.0f, 0.0f, 0.5f);
  lastVisualTeam = NoTeam;
  tankTexture = -1;
}


StandardTankAvatar::~StandardTankAvatar ( void )
{
  delete IDLNode;
  delete tankNode;
  delete pausedSphere;
}



void StandardTankAvatar::move ( const fvec3& pos, const fvec3& forward )
{
  tankNode->move(pos,forward);
}


void StandardTankAvatar::moveIDL ( const fvec4& plane )
{
  IDLNode->move(plane);
}


void StandardTankAvatar::movePause ( const fvec3& pos, float rad )
{
  pausedSphere->move(pos,rad);
}


void StandardTankAvatar::setTurnOffsets ( const float left, const float right )
{
  tankNode->addTreadOffsets(left,right);
}


void StandardTankAvatar::setScale ( const fvec3& scale)
{
  tankNode->setDimensions(scale);
}


void StandardTankAvatar::setScale ( teAvatarScaleModes mode )
{
  switch (mode) {
    case eNormal: { tankNode->setNormal(); break; }
    case eFat:    { tankNode->setObese();  break; }
    case eTiny:   { tankNode->setTiny();   break; }
    case eThin:   { tankNode->setNarrow(); break; }
    case eThief:  { tankNode->setThief();  break; }
  }
}


void StandardTankAvatar::explode ( void )
{
  tankNode->rebuildExplosion();
}


void StandardTankAvatar::setVisualTeam (TeamColor visualTeam, const fvec4& color )
{
  tankNode->setColor(color);

  // only do all this junk when the effective team color actually changes
  if (visualTeam == lastVisualTeam)
    return;
  lastVisualTeam = visualTeam;

  static const fvec4	tankSpecular(0.1f, 0.1f, 0.1f, 1.0f);
  static const fvec4	tankEmissive(0.0f, 0.0f, 0.0f, 1.0f);
  static float		tankShininess = 20.0f;
  static const fvec4	rabbitEmissive(0.0f, 0.0f, 0.0f, 1.0f);
  static float		rabbitShininess = 100.0f;

  const fvec4* emissive;
  GLfloat shininess;

  if (visualTeam == RabbitTeam) {
    emissive = &rabbitEmissive;
    shininess = rabbitShininess;
  } else {
    emissive = &tankEmissive;
    shininess = tankShininess;
  }

  TextureManager &tm = TextureManager::instance();
  std::string texName;
  texName = Team::getImagePrefix(visualTeam);

  texName += BZDB.get("tankTexture");

  // now after we did all that, see if they have a user texture
  tankTexture = tm.getTextureID(texName.c_str(),false);

  tankNode->setMaterial(OpenGLMaterial(tankSpecular, *emissive, shininess));
  tankNode->setTexture(tankTexture);

  int jumpJetsTexture = tm.getTextureID("jumpjets", false);
  tankNode->setJumpJetsTexture(jumpJetsTexture);
}


void StandardTankAvatar::setColor ( const fvec4& color )
{
  tankNode->setColor(color);
}


void StandardTankAvatar::setVisualMode ( bool inCockpit, bool showTreads )
{
  // setup the visibility properties
  if (inCockpit && !showTreads)
    tankNode->setOnlyShadows(true);
  else
    tankNode->setOnlyShadows(false);

  tankNode->setInTheCockpit(inCockpit);

  // reset the clipping plane
  tankNode->setClipPlane(NULL);

  tankNode->setJumpJets(0.0f);
}


void StandardTankAvatar::setAnimationValues ( float explodeParam, float jumpParam )
{
  tankNode->setExplodeFraction(explodeParam);
  tankNode->setJumpJets(jumpParam);
}


void StandardTankAvatar::setClippingPlane (const fvec4* plane)
{
  tankNode->setClipPlane(plane);
}


std::vector<SceneNode*> StandardTankAvatar::getSceneNodes ( void )
{
  std::vector<SceneNode*> l;
  l.push_back(tankNode);
  return l;
}


std::vector<SceneNode*> StandardTankAvatar::getIDLSceneNodes ( void )
{
  std::vector<SceneNode*> l;
  l.push_back(IDLNode);
  return l;
}


std::vector<SceneNode*> StandardTankAvatar::getPauseSceneNodes ( void )
{
  std::vector<SceneNode*> l;
  l.push_back(pausedSphere);
  return l;
}


void StandardTankAvatar::renderRadar ( void )
{
  tankNode->renderRadar();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
