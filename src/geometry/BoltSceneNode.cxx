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

// bzflag common header
#include "common.h"

// interface header
#include "BoltSceneNode.h"

// system headers
#include <stdlib.h>
#include <string.h>
#include <math.h>

// common headers
#include "bzfgl.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "SceneRenderer.h" // FIXME (SceneRenderer.cxx is in src/bzflag)
#include "TextureManager.h"

// local headers
#include "ViewFrustum.h"


BoltSceneNode::BoltSceneNode(const fvec3& pos, const fvec3& vel)
: phasingShot(false)
, drawFlares(false)
, invisible(false)
, texturing(false)
, colorblind(false)
, size(1.0f)
, renderNode(this)
, azimuth(0)
, elevation(0)
, length(1.0f)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setBlending();
  builder.setAlphaFunc();
  //builder.setTextureEnvMode(GL_DECAL);
  gstate = builder.getState();

  // prepare light
  light.setAttenuation(0, 0.05f);
  light.setAttenuation(1, 0.0f);
  light.setAttenuation(2, 0.03f);

  // prepare geometry
  move(pos, vel);
  setSize(size);
  setColor(1.0f, 1.0f, 1.0f);
}


BoltSceneNode::~BoltSceneNode()
{
  // do nothing
}


void BoltSceneNode::setFlares(bool on)
{
  drawFlares = on;
}


void BoltSceneNode::setSize(float radius)
{
  size = radius;
  setRadius(size * size);
}


void BoltSceneNode::setTextureColor(float r, float g, float b, float a)
{
  color = fvec4(r, g, b, a);
  light.setColor(1.5f * r, 1.5f * g, 1.5f * b);
  renderNode.setTextureColor(color);
}


void BoltSceneNode::setColor(float r, float g, float b, float a)
{
  color = fvec4(r, g, b, a);
  light.setColor(1.5f * r, 1.5f * g, 1.5f * b);
  renderNode.setColor(color);
}


void BoltSceneNode::setColor(const fvec4& rgba)
{
  setColor(rgba.r, rgba.g, rgba.b, rgba.a);
}


bool BoltSceneNode::getColorblind() const
{
  return colorblind;
}


void BoltSceneNode::setColorblind(bool _colorblind)
{
  colorblind = _colorblind;
}


bool BoltSceneNode::getInvisible() const
{
  return invisible;
}


void BoltSceneNode::setInvisible(bool _invisible)
{
  invisible = _invisible;
}


void BoltSceneNode::setTexture(const int texture)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(texture);
  builder.enableTexture(texture>=0);
  gstate = builder.getState();
}


void BoltSceneNode::setTextureAnimation(int cu, int cv)
{
  renderNode.setAnimation(cu, cv);
}


void BoltSceneNode::move(const fvec3& pos, const fvec3& vel)
{
  setCenter(pos);
  light.setPosition(pos);

  velocity = vel;
  length = vel.length();

  azimuth   = (float)(+RAD2DEG * atan2f(vel.y, vel.x));
  elevation = (float)(-RAD2DEG * atan2f(vel.z, vel.xy().length()));
}


void BoltSceneNode::addLight(SceneRenderer& renderer)
{
  renderer.addLight(light);
}


void BoltSceneNode::notifyStyleChange()
{
  texturing = BZDBCache::texture && BZDBCache::blend;
  OpenGLGStateBuilder builder(gstate);
  builder.enableTexture(texturing);
  if (BZDBCache::blend) {
    const int shotLength = (int)(BZDBCache::shotLength * 3.0f);
    if ((shotLength > 0) || (RENDERER.useQuality() >= _EXPERIMENTAL_QUALITY)) {
      builder.setBlending(GL_SRC_ALPHA, GL_ONE);
    } else {
      builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    builder.setStipple(1.0f);
    builder.setAlphaFunc();
    if (!texturing) builder.setShading(GL_SMOOTH);
    else builder.setShading(GL_FLAT);
  }
  else {
    builder.resetBlending();
    builder.resetAlphaFunc();
    builder.setStipple(0.5f);
    builder.setShading(GL_FLAT);
  }
  gstate = builder.getState();
}


void BoltSceneNode::addRenderNodes(SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, &gstate);
}


//
// BoltSceneNode::BoltRenderNode
//

const float BoltSceneNode::BoltRenderNode::CoreFraction = 0.4f;
const float BoltSceneNode::BoltRenderNode::FlareSize    = 1.0f;
const float BoltSceneNode::BoltRenderNode::FlareSpread  = 0.08f;
float       BoltSceneNode::BoltRenderNode::core[9][2];
float       BoltSceneNode::BoltRenderNode::corona[8][2];
const float BoltSceneNode::BoltRenderNode::ring[8][2] = {
  { 1.0f, 0.0f },
  { (float)M_SQRT1_2, (float)M_SQRT1_2 },
  { 0.0f, 1.0f },
  { (float)-M_SQRT1_2, (float)M_SQRT1_2 },
  { -1.0f, 0.0f },
  { (float)-M_SQRT1_2, (float)-M_SQRT1_2 },
  { 0.0f, -1.0f },
  { (float)M_SQRT1_2, (float)-M_SQRT1_2 }
};


BoltSceneNode::BoltRenderNode::BoltRenderNode(const BoltSceneNode* _sceneNode)
: sceneNode(_sceneNode)
, numFlares(0)
{
  // initialize core and corona if not already done
  static bool init = false;
  if (!init) {
    init = true;
    core[0][0] = 0.0f;
    core[0][1] = 0.0f;
    for (int i = 0; i < 8; i++) {
      core[i+1][0] = CoreFraction * ring[i][0];
      core[i+1][1] = CoreFraction * ring[i][1];
      corona[i][0] = ring[i][0];
      corona[i][1] = ring[i][1];
    }
  }

  textureColor = fvec4(1.0f, 1.0f, 1.0f, 1.0f);

  setAnimation(1, 1);
}


BoltSceneNode::BoltRenderNode::~BoltRenderNode()
{
  // do nothing
}


void BoltSceneNode::BoltRenderNode::setAnimation(int _cu, int _cv)
{
  cu = _cu;
  cv = _cv;
  du = 1.0f / (float)cu;
  dv = 1.0f / (float)cv;

  // pick a random start frame
  const int index = (int)((float)cu * (float)cv * bzfrand());
  u = index % cu;
  v = index / cu;
  if (v >= cv) v = 0;
}


void BoltSceneNode::BoltRenderNode::setTextureColor(const fvec4& rgba)
{
  textureColor = rgba;
}


void BoltSceneNode::BoltRenderNode::setColor(const fvec4& rgba)
{
  mainColor = rgba;

  innerColor.rgb() = 0.5f * (rgba.rgb() + 1.0f);
  innerColor.a = rgba.a;

  outerColor.rgb()  = rgba.rgb();
  flareColor.rgb()  = rgba.rgb();
  coronaColor.rgb() = rgba.rgb();

  outerColor.a  = (rgba.a == 1.0f)  ? 0.1f   : rgba.a;
  flareColor.a  = (rgba.a == 1.0f ) ? 0.667f : rgba.a;
  coronaColor.a = (rgba.a == 1.0f ) ? 0.5f   : rgba.a;
}


void BoltSceneNode::BoltRenderNode::renderGeoPill(float radius, float length,
                                                  int segments, float endRad)
{
  glPushMatrix();

  float assRadius = radius;
  if (endRad >= 0)
    assRadius = endRad;

  float lenMinusRads = length - (radius+assRadius);

  GLUquadric *q = gluNewQuadric();
  if (assRadius > 0)
  {
    // 4 parts of the first hemisphere
    gluCylinder(q,0,assRadius*0.43589,assRadius*0.1f,segments,1);
    addTriangleCount(segments);
    glTranslatef(0,0,assRadius*0.1f);

    gluCylinder(q,assRadius*0.43589,assRadius*0.66144,assRadius*0.15f,segments,1);
    addTriangleCount(segments);
    glTranslatef(0,0,assRadius*0.15f);

    gluCylinder(q,assRadius*0.66144f,assRadius*0.86603f,assRadius*0.25f,segments,1);
    addTriangleCount(segments);
    glTranslatef(0,0,assRadius*0.25f);

    gluCylinder(q,assRadius*0.86603,assRadius,assRadius*0.5f,segments,1);
    addTriangleCount(segments);
    glTranslatef(0,0,assRadius*0.5f);
  }

  // the "shaft"
  if (lenMinusRads > 0)
  {
    gluCylinder(q,assRadius,radius,lenMinusRads,segments,1);
    addTriangleCount(segments);
    glTranslatef(0,0,lenMinusRads);
  }

  if (radius > 0)
  {
    // 4 parts of the last hemisphere
    gluCylinder(q,radius,radius*0.86603,radius*0.5f,segments,1);
    addTriangleCount(segments);
    glTranslatef(0,0,radius*0.5f);

    gluCylinder(q,radius*0.86603f,radius*0.66144f,radius*0.25f,segments,1);
    addTriangleCount(segments);
    glTranslatef(0,0,radius*0.25f);

    gluCylinder(q,radius*0.66144,radius*0.43589,radius*0.15f,segments,1);
    addTriangleCount(segments);
    glTranslatef(0,0,radius*0.15f);

    gluCylinder(q,radius*0.43589,0,radius*0.1f,segments,1);
    addTriangleCount(segments);
    glTranslatef(0,0,radius*0.1f);
  }

  gluDeleteQuadric(q);
  glPopMatrix();
}


void BoltSceneNode::BoltRenderNode::renderGeoGMBolt()
{
  // bzdb these 2? they control the shot size
  float lenMod = 0.025f;
  float baseRadius = 0.2f;

  float len = sceneNode->length * lenMod;
  glPushMatrix();
  glRotatef(sceneNode->azimuth, 0.0f, 0.0f, 1.0f);
  glRotatef(sceneNode->elevation, 0.0f, 1.0f, 0.0f);
  glRotatef(90, 0.0f, 1.0f, 0.0f);

  float alphaMod = 1.0f;
  if (sceneNode->phasingShot)
    alphaMod = 0.85f;

  glDisable(GL_TEXTURE_2D);

  float coreBleed = 4.5f;
  float minimumChannelVal = 0.45f;
  fvec3 coreColor = sceneNode->color.rgb() * coreBleed;
  if (coreColor.r < minimumChannelVal) { coreColor.r = minimumChannelVal; }
  if (coreColor.g < minimumChannelVal) { coreColor.g = minimumChannelVal; }
  if (coreColor.b < minimumChannelVal) { coreColor.b = minimumChannelVal; }

  glPushMatrix();
  myColor4f(1, 1, 1, 0.85f * alphaMod);
  glTranslatef(0, 0, len - baseRadius);

  GLUquadric* q = gluNewQuadric();
  gluSphere(q, baseRadius * 0.75f, 6, 6);
  addTriangleCount(6 * 6);
  glPopMatrix();

  myColor4fv(fvec4(coreColor,  0.85f * alphaMod));
  renderGeoPill(baseRadius, len, 16, baseRadius * 0.25f);

  float radInc = 1.5f * baseRadius - baseRadius;
  glPushMatrix();
  glTranslatef(0, 0, -radInc * 0.5f);
  myColor4fv(fvec4(sceneNode->color.rgb(), 0.5f));
  renderGeoPill(1.5f * baseRadius, len + radInc, 25, 1.5f * baseRadius * 0.25f);
  glPopMatrix();

  glPushMatrix();
  myColor4f(1, 1, 1, 0.125f);
  glTranslatef(0, 0, len*0.125f);
  gluCylinder(q, 3.0f * baseRadius, 1.75f * baseRadius, len * 0.35f, 16, 1);
  addTriangleCount(16);

  glTranslatef(0, 0, len * 0.5f);
  gluCylinder(q, 2.5f * baseRadius, 1.5f * baseRadius, len * 0.25f, 16, 1);
  addTriangleCount(16);

  glPopMatrix();

  gluDeleteQuadric(q);

  glEnable(GL_TEXTURE_2D);

  glPopMatrix();
}


void BoltSceneNode::BoltRenderNode::renderGeoBolt()
{
  // bzdb these 2? they control the shot size
  float lenMod = 0.025f;
  float baseRadius = 0.225f;

  float len = sceneNode->length * lenMod;
  glPushMatrix();
  glRotatef(sceneNode->azimuth, 0.0f, 0.0f, 1.0f);
  glRotatef(sceneNode->elevation, 0.0f, 1.0f, 0.0f);
  glRotatef(90, 0.0f, 1.0f, 0.0f);

  float alphaMod = 1.0f;
  if (sceneNode->phasingShot)
    alphaMod = 0.85f;

  glDisable(GL_TEXTURE_2D);

  float coreBleed = 4.5f;
  float minimumChannelVal = 0.45f;

  fvec3 coreColor =  sceneNode->color.rgb() * coreBleed;
  if (coreColor.r < minimumChannelVal) { coreColor.r = minimumChannelVal; }
  if (coreColor.g < minimumChannelVal) { coreColor.g = minimumChannelVal; }
  if (coreColor.b < minimumChannelVal) { coreColor.b = minimumChannelVal; }

  myColor4fv(fvec4(coreColor, 0.85f * alphaMod));
  renderGeoPill(baseRadius,len,16);

  float radInc = 1.5f * baseRadius - baseRadius;
  glPushMatrix();
  glTranslatef(0, 0, -radInc * 0.5f);
  myColor4fv(fvec4(sceneNode->color.rgb(), 0.5f));
  renderGeoPill(1.5f * baseRadius, len + radInc, 25);
  glPopMatrix();

  radInc = 2.7f * baseRadius - baseRadius;
  glPushMatrix();
  glTranslatef(0, 0, -radInc*0.5f);
  myColor4fv(fvec4(sceneNode->color.rgb(), 0.25f));
  renderGeoPill(2.7f * baseRadius, len + radInc, 32);
  glPopMatrix();

  radInc = 3.8f * baseRadius - baseRadius;
  glPushMatrix();
  glTranslatef(0, 0,-radInc*0.5f);
  myColor4fv(fvec4(sceneNode->color.rgb(), 0.125f));
  renderGeoPill(3.8f * baseRadius, len + radInc, 48);
  glPopMatrix();

  glEnable(GL_TEXTURE_2D);

  glPopMatrix();
}




void BoltSceneNode::BoltRenderNode::render()
{
  if (sceneNode->invisible) {
    return;
  }
  const float u0 = (float)u * du;
  const float v0 = (float)v * dv;
  const float u1 = u0 + du;
  const float v1 = v0 + dv;

  const fvec4& sphere = sceneNode->getSphere();
  glPushMatrix();
  glTranslatef(sphere.x, sphere.y, sphere.z);
  const int shotLength = (int)(BZDBCache::shotLength * 3.0f);
  if ((shotLength <= 0) &&
      (RENDERER.useQuality() >= _EXPERIMENTAL_QUALITY)) {
    if (!sceneNode->drawFlares) {
      renderGeoBolt();
    } else {
      renderGeoGMBolt();
    }
  }
  else {
    RENDERER.getViewFrustum().executeBillboard();
    glScalef(sceneNode->size, sceneNode->size, sceneNode->size);
    // draw some flares
    if (sceneNode->drawFlares) {
      if (!RENDERER.isSameFrame()) {
	numFlares = 3 + int(3.0f * (float)bzfrand());
	for (int i = 0; i < numFlares; i++) {
	  theta[i] = (float)(2.0 * M_PI * bzfrand());
	  phi[i] = (float)bzfrand() - 0.5f;
	  phi[i] *= (float)(2.0 * M_PI * fabsf(phi[i]));
	}
      }

      if (sceneNode->texturing) glDisable(GL_TEXTURE_2D);
      myColor4fv(flareColor);
      if (!BZDBCache::blend) myStipple(flareColor.a);
      glBegin(GL_QUADS);
      for (int i = 0; i < numFlares; i++) {
	// pick random direction in 3-space.  picking a random theta with
	// a uniform distribution is fine, but doing so with phi biases
	// the directions toward the poles.  my correction doesn't remove
	// the bias completely, but moves it towards the equator, which is
	// really where i want it anyway cos the flares are more noticeable
	// there.
	const float c = FlareSize * float(cosf(phi[i]));
	const float s = FlareSize * float(sinf(phi[i]));
	glVertex3fv(core[0]);
	glVertex3f(c * cosf(theta[i]-FlareSpread), c * sinf(theta[i]-FlareSpread), s);
	glVertex3f(2.0f * c * cosf(theta[i]), 2.0f * c * sinf(theta[i]), 2.0f * s);
	glVertex3f(c * cosf(theta[i]+FlareSpread), c * sinf(theta[i]+FlareSpread), s);
      }
      glEnd();
      if (sceneNode->texturing) glEnable(GL_TEXTURE_2D);

      addTriangleCount(numFlares * 2);
    }

    if (sceneNode->texturing) {
      // draw billboard square
      myColor4fv(textureColor); // 1.0f all
      glBegin(GL_QUADS);
      glTexCoord2f(u0, v0); glVertex2f(-1.0f, -1.0f);
      glTexCoord2f(u1, v0); glVertex2f(+1.0f, -1.0f);
      glTexCoord2f(u1, v1); glVertex2f(+1.0f, +1.0f);
      glTexCoord2f(u0, v1); glVertex2f(-1.0f, +1.0f);
      glEnd();
      addTriangleCount(2);


      if ((shotLength > 0) && (sceneNode->length > 1.0e-6f)) {
        const float startSize  = 0.6f;
        const float startAlpha = 0.8f;

        glPushAttrib(GL_TEXTURE_BIT);
        TextureManager &tm = TextureManager::instance();
        const int texID = tm.getTextureID("missile");
        const ImageInfo& texInfo = tm.getInfo(texID);
        if (texInfo.id >= 0) {
          texInfo.texture->execute();
        }

        const fvec3& vel = sceneNode->velocity;
        const fvec3  dir = vel * (-1.0f / sceneNode->length);

        const float invLenPlusOne = 1.0f / (float)(shotLength + 1);
        const float shiftScale = 90.0f / (150.0f + (float)shotLength);
        float size = sceneNode->size * startSize;
        float alpha = startAlpha;
        const float sizeStep  = size  * invLenPlusOne;
        const float alphaStep = alpha * invLenPlusOne;

        fvec3 pos = sphere.xyz();

        int uvCell = rand() % 16;

        for (int i = 0; i < shotLength; i++) {
          size  -= sizeStep;
          const float s = size * (0.65f + (1.0f * (float)bzfrand()));
          const float shift = s * shiftScale;

          pos += (shift * dir);
          if (pos.z < 0.0f) {
            continue;
          }

          uvCell = (uvCell + 1) % 16;
          const float U0 = (uvCell % 4 ) * 0.25f;
          const float V0 = (uvCell / 4 ) * 0.25f;
          const float U1 = U0 + 0.25f;
          const float V1 = V0 + 0.25f;

          alpha -= alphaStep;
          glColor4f(1.0f, 1.0f, 1.0f, alpha);
          glPopMatrix();
          glPushMatrix();

          glTranslatef(pos.x, pos.y, pos.z);
          RENDERER.getViewFrustum().executeBillboard();
          glScalef(s, s, s);

          glBegin(GL_QUADS);
          glTexCoord2f(U0, V0); glVertex2f(-1.0f, -1.0f);
          glTexCoord2f(U1, V0); glVertex2f(+1.0f, -1.0f);
          glTexCoord2f(U1, V1); glVertex2f(+1.0f, +1.0f);
          glTexCoord2f(U0, V1); glVertex2f(-1.0f, +1.0f);
          glEnd();
        }

        addTriangleCount(shotLength * 2);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glPopAttrib(); // revert the texture
      }
    }
    else if (BZDBCache::blend) {
      // draw corona
      glBegin(GL_QUAD_STRIP);
      myColor4fv(mainColor);  glVertex2fv(core[1]);
      myColor4fv(outerColor); glVertex2fv(corona[0]);
      myColor4fv(mainColor);  glVertex2fv(core[2]);
      myColor4fv(outerColor); glVertex2fv(corona[1]);
      myColor4fv(mainColor);  glVertex2fv(core[3]);
      myColor4fv(outerColor); glVertex2fv(corona[2]);
      myColor4fv(mainColor);  glVertex2fv(core[4]);
      myColor4fv(outerColor); glVertex2fv(corona[3]);
      myColor4fv(mainColor);  glVertex2fv(core[5]);
      myColor4fv(outerColor); glVertex2fv(corona[4]);
      myColor4fv(mainColor);  glVertex2fv(core[6]);
      myColor4fv(outerColor); glVertex2fv(corona[5]);
      myColor4fv(mainColor);  glVertex2fv(core[7]);
      myColor4fv(outerColor); glVertex2fv(corona[6]);
      myColor4fv(mainColor);  glVertex2fv(core[8]);
      myColor4fv(outerColor); glVertex2fv(corona[7]);
      myColor4fv(mainColor);  glVertex2fv(core[1]);
      myColor4fv(outerColor); glVertex2fv(corona[0]);
      glEnd(); // 18 verts -> 16 tris

      // draw core
      glBegin(GL_TRIANGLE_FAN);
      myColor4fv(innerColor);
      glVertex2fv(core[0]);
      myColor4fv(mainColor);
      glVertex2fv(core[1]);
      glVertex2fv(core[2]);
      glVertex2fv(core[3]);
      glVertex2fv(core[4]);
      glVertex2fv(core[5]);
      glVertex2fv(core[6]);
      glVertex2fv(core[7]);
      glVertex2fv(core[8]);
      glVertex2fv(core[1]);
      glEnd(); // 10 verts -> 8 tris

      addTriangleCount(24);
    }
    else {
      // draw corona
      myColor4fv(coronaColor);
      myStipple(coronaColor[3]);
      glBegin(GL_QUAD_STRIP);
      glVertex2fv(core[1]);
      glVertex2fv(corona[0]);
      glVertex2fv(core[2]);
      glVertex2fv(corona[1]);
      glVertex2fv(core[3]);
      glVertex2fv(corona[2]);
      glVertex2fv(core[4]);
      glVertex2fv(corona[3]);
      glVertex2fv(core[5]);
      glVertex2fv(corona[4]);
      glVertex2fv(core[6]);
      glVertex2fv(corona[5]);
      glVertex2fv(core[7]);
      glVertex2fv(corona[6]);
      glVertex2fv(core[8]);
      glVertex2fv(corona[7]);
      glVertex2fv(core[1]);
      glVertex2fv(corona[0]);
      glEnd(); // 18 verts -> 16 tris

      // draw core
      myStipple(1.0f);
      glBegin(GL_TRIANGLE_FAN);
      myColor4fv(innerColor);
      glVertex2fv(core[0]);
      myColor4fv(mainColor);
      glVertex2fv(core[1]);
      glVertex2fv(core[2]);
      glVertex2fv(core[3]);
      glVertex2fv(core[4]);
      glVertex2fv(core[5]);
      glVertex2fv(core[6]);
      glVertex2fv(core[7]);
      glVertex2fv(core[8]);
      glVertex2fv(core[1]);
      glEnd(); // 10 verts -> 8 tris

      myStipple(0.5f);

      addTriangleCount(24);
    }
  }

  glPopMatrix();

  if (RENDERER.isLastFrame()) {
    if (++u == cu) {
      u = 0;
      if (++v == cv) {
        v = 0;
      }
    }
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
