/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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
#include <math.h>

// common implementation headers
#include "StateDatabase.h"
#include "BZDBCache.h"

// local implementation headers
#include "ViewFrustum.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

BoltSceneNode::BoltSceneNode(const GLfloat pos[3], const GLfloat vel[3]) :
				drawFlares(false),
				texturing(false),
				colorblind(false),
				size(1.0f),
				renderNode(this),
				length(1.0f),
				azimuth(0),
				elevation(0),
				phasingShot(false)
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

void			BoltSceneNode::setFlares(bool on)
{
  drawFlares = on;
}

void			BoltSceneNode::setSize(float radius)
{
  size = radius;
  setRadius(size * size);
}
void			BoltSceneNode::setTextureColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = a;
  light.setColor(1.5f * r, 1.5f * g, 1.5f * b);
  renderNode.setTextureColor(color);
}

void			BoltSceneNode::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = a;
  light.setColor(1.5f * r, 1.5f * g, 1.5f * b);
  renderNode.setColor(color);
}

void			BoltSceneNode::setColor(const GLfloat* rgb)
{
  setColor(rgb[0], rgb[1], rgb[2]);
}

bool			BoltSceneNode::getColorblind() const
{
  return colorblind;
}

void			BoltSceneNode::setColorblind(bool _colorblind)
{
  colorblind = _colorblind;
}

void			BoltSceneNode::setTexture(const int texture)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(texture);
  builder.enableTexture(texture>=0);
  gstate = builder.getState();
}

void			BoltSceneNode::setTextureAnimation(int cu, int cv)
{
  renderNode.setAnimation(cu, cv);
}

void			BoltSceneNode::move(const GLfloat pos[3], const GLfloat vel[3] )
{
  setCenter(pos);
  light.setPosition(pos);

  if (vel)
  {
	  float vec[3];
	  length = sqrt(vel[0]*vel[0]+vel[1]*vel[1]+vel[2]*vel[2]);
	  vec[0] = vel[0]/length;
	  vec[1] = vel[1]/length;
	  vec[2] = vel[2]/length;

	  azimuth = (float)(180.0 / M_PI*atan2f(vec[1], vec[0]));
	  elevation = (float)(-180.0 / M_PI*atan2f(vec[2], hypotf(vec[0],vec[1])));
  }
}

void			BoltSceneNode::addLight(
				SceneRenderer& renderer)
{
  renderer.addLight(light);
}

void			BoltSceneNode::notifyStyleChange()
{
  texturing = BZDBCache::texture && BZDBCache::blend;
  OpenGLGStateBuilder builder(gstate);
  builder.enableTexture(texturing);
  if (BZDBCache::blend) {
    if (RENDERER.useQuality() >=_EXPEREMENTAL_QUALITY )
      builder.setBlending(GL_SRC_ALPHA, GL_ONE);
    else
      builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

void			BoltSceneNode::addRenderNodes(
				SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, &gstate);
}

//
// BoltSceneNode::BoltRenderNode
//

const GLfloat		BoltSceneNode::BoltRenderNode::CoreFraction = 0.4f;
const GLfloat		BoltSceneNode::BoltRenderNode::FlareSize = 1.0f;
const GLfloat		BoltSceneNode::BoltRenderNode::FlareSpread = 0.08f;
GLfloat			BoltSceneNode::BoltRenderNode::core[9][2];
GLfloat			BoltSceneNode::BoltRenderNode::corona[8][2];
const GLfloat		BoltSceneNode::BoltRenderNode::ring[8][2] = {
				{ 1.0f, 0.0f },
				{ (float)M_SQRT1_2, (float)M_SQRT1_2 },
				{ 0.0f, 1.0f },
				{ (float)-M_SQRT1_2, (float)M_SQRT1_2 },
				{ -1.0f, 0.0f },
				{ (float)-M_SQRT1_2, (float)-M_SQRT1_2 },
				{ 0.0f, -1.0f },
				{ (float)M_SQRT1_2, (float)-M_SQRT1_2 }
			};

BoltSceneNode::BoltRenderNode::BoltRenderNode(
				const BoltSceneNode* _sceneNode) :
				sceneNode(_sceneNode),
				numFlares(0)
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

  textureColor[0] = 1.0f;
  textureColor[1] = 1.0f;
  textureColor[2] = 1.0f;
  textureColor[3] = 1.0f;

  setAnimation(1, 1);
}

BoltSceneNode::BoltRenderNode::~BoltRenderNode()
{
  // do nothing
}

void			BoltSceneNode::BoltRenderNode::setAnimation(
				int _cu, int _cv)
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
void			BoltSceneNode::BoltRenderNode::setTextureColor(const GLfloat* rgba)
{
  textureColor[0] = rgba[0];
  textureColor[1] = rgba[1];
  textureColor[2] = rgba[2];
  textureColor[3] = rgba[3];
}


void			BoltSceneNode::BoltRenderNode::setColor(
				const GLfloat* rgba)
{
  mainColor[0] = rgba[0];
  mainColor[1] = rgba[1];
  mainColor[2] = rgba[2];
  mainColor[3] = rgba[3];

  innerColor[0] = mainColor[0] + 0.5f * (1.0f - mainColor[0]);
  innerColor[1] = mainColor[1] + 0.5f * (1.0f - mainColor[1]);
  innerColor[2] = mainColor[2] + 0.5f * (1.0f - mainColor[2]);
  innerColor[3] = rgba[3];

  outerColor[0] = mainColor[0];
  outerColor[1] = mainColor[1];
  outerColor[2] = mainColor[2];
  outerColor[3] = (rgba[3] == 1.0f )? 0.1f: rgba[3];

  coronaColor[0] = mainColor[0];
  coronaColor[1] = mainColor[1];
  coronaColor[2] = mainColor[2];
  coronaColor[3] = (rgba[3] == 1.0f )? 0.5f : rgba[3];

  flareColor[0] = mainColor[0];
  flareColor[1] = mainColor[1];
  flareColor[2] = mainColor[2];
  flareColor[3] = (rgba[3] == 1.0f )? 0.667f : rgba[3];
}

void		BoltSceneNode::BoltRenderNode::renderGeoPill( float radius, float length, int segments )
{
	float lenMinusRads = length - radius*2;
	glPushMatrix();

	// 4 parts of the first hemesphere
	gluCylinder(gluNewQuadric(),0,radius*0.43589,radius*0.1f,segments,1);
	addTriangleCount(segments);
	glTranslatef(0,0,radius*0.1f);

	gluCylinder(gluNewQuadric(),radius*0.43589,radius*0.66144,radius*0.15f,segments,1);
	addTriangleCount(segments);
	glTranslatef(0,0,radius*0.15f);

	gluCylinder(gluNewQuadric(),radius*0.66144f,radius*0.86603f,radius*0.25f,segments,1);
	addTriangleCount(segments);
	glTranslatef(0,0,radius*0.25f);

	gluCylinder(gluNewQuadric(),radius*0.86603,radius,radius*0.5f,segments,1);
	addTriangleCount(segments);
	glTranslatef(0,0,radius*0.5f);

	// the "shaft"
	if (lenMinusRads > 0)
	{
		gluCylinder(gluNewQuadric(),radius,radius,lenMinusRads,segments,1);
		addTriangleCount(segments);
		glTranslatef(0,0,lenMinusRads);
	}

	// 4 parts of the last hemesphere
	gluCylinder(gluNewQuadric(),radius,radius*0.86603,radius*0.5f,segments,1);
	addTriangleCount(segments);
	glTranslatef(0,0,radius*0.5f);

	gluCylinder(gluNewQuadric(),radius*0.86603f,radius*0.66144f,radius*0.25f,segments,1);
	addTriangleCount(segments);
	glTranslatef(0,0,radius*0.25f);

	gluCylinder(gluNewQuadric(),radius*0.66144,radius*0.43589,radius*0.15f,segments,1);
	addTriangleCount(segments);
	glTranslatef(0,0,radius*0.15f);

	gluCylinder(gluNewQuadric(),radius*0.43589,0,radius*0.1f,segments,1);
	addTriangleCount(segments);
	glTranslatef(0,0,radius*0.1f);

	glPopMatrix();
}

void		BoltSceneNode::BoltRenderNode::renderGeoBolt()
{
	// bzdb these 2? they controll the shot size
	float lenMod = 0.025f;
	float baseRadius = 0.225f;

	GLfloat len = sceneNode->length * lenMod;
	const GLfloat* sphere = sceneNode->getSphere();
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
	float coreColor[3];

	coreColor[0] = sceneNode->color[0]*coreBleed;
	if (coreColor[0] < minimumChannelVal)
		coreColor[0] = minimumChannelVal;

	coreColor[1] = sceneNode->color[1]*coreBleed;
	if (coreColor[1] < minimumChannelVal)
		coreColor[1] = minimumChannelVal;

	coreColor[2] = sceneNode->color[2]*coreBleed;
	if (coreColor[2] < minimumChannelVal)
		coreColor[2] = minimumChannelVal;

	myColor4f(coreColor[0], coreColor[1], coreColor[2], 0.85f*alphaMod);
	renderGeoPill(baseRadius,len,16);

	float radInc = 1.5f*baseRadius - baseRadius;
	glPushMatrix();
	glTranslatef(0, 0, -radInc*0.5f);
	myColor4f(sceneNode->color[0], sceneNode->color[1], sceneNode->color[2], 0.5f);
	renderGeoPill(1.5f*baseRadius,len+radInc,25);
	glPopMatrix();

	radInc = 2.7f*baseRadius - baseRadius;
	glPushMatrix();
	glTranslatef(0, 0, -radInc*0.5f);
	myColor4f(sceneNode->color[0], sceneNode->color[1], sceneNode->color[2], 0.25f);
	renderGeoPill(2.7f*baseRadius,len+radInc,32);
	glPopMatrix();

	radInc = 3.8f*baseRadius - baseRadius;
	glPushMatrix();
	glTranslatef(0, 0,-radInc*0.5f);
	myColor4f(sceneNode->color[0], sceneNode->color[1], sceneNode->color[2], 0.125f);
	renderGeoPill(3.8f*baseRadius,len+radInc,48);
	glPopMatrix();

	glEnable(GL_TEXTURE_2D);

	glPopMatrix();
}

void			BoltSceneNode::BoltRenderNode::render()
{
  const float u0 = (float)u * du;
  const float v0 = (float)v * dv;

  const GLfloat* sphere = sceneNode->getSphere();
  glPushMatrix();
    glTranslatef(sphere[0], sphere[1], sphere[2]);
	if (!sceneNode->drawFlares && RENDERER.useQuality() >= _EXPEREMENTAL_QUALITY)
		renderGeoBolt();
	else
	{
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
		if (!BZDBCache::blend) myStipple(flareColor[3]);
		glBegin(GL_QUADS);
		for (int i = 0; i < numFlares; i++) {
		// pick random direction in 3-space.  picking a random theta with
		// a uniform distribution is fine, but doing so with phi biases
		// the directions toward the poles.  my correction doesn't remove
		// the bias completely, but moves it towards the equator, which is
		// really where i want it anyway cos the flares are more noticeable
		// there.
		const GLfloat c = FlareSize * GLfloat(cosf(phi[i]));
		const GLfloat s = FlareSize * GLfloat(sinf(phi[i]));
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
		glTexCoord2f(   u0,    v0);
		glVertex2f  (-1.0f, -1.0f);
		glTexCoord2f(du+u0,    v0);
		glVertex2f  ( 1.0f, -1.0f);
		glTexCoord2f(du+u0, dv+v0);
		glVertex2f  ( 1.0f,  1.0f);
		glTexCoord2f(   u0, dv+v0);
		glVertex2f  (-1.0f,  1.0f);
		glEnd();
		addTriangleCount(2);
		}

		else if (BZDBCache::blend) {
		// draw corona
		glBegin(GL_QUAD_STRIP);
		myColor4fv(mainColor);
		glVertex2fv(core[1]);
		myColor4fv(outerColor);
		glVertex2fv(corona[0]);
		myColor4fv(mainColor);
		glVertex2fv(core[2]);
		myColor4fv(outerColor);
		glVertex2fv(corona[1]);
		myColor4fv(mainColor);
		glVertex2fv(core[3]);
		myColor4fv(outerColor);
		glVertex2fv(corona[2]);
		myColor4fv(mainColor);
		glVertex2fv(core[4]);
		myColor4fv(outerColor);
		glVertex2fv(corona[3]);
		myColor4fv(mainColor);
		glVertex2fv(core[5]);
		myColor4fv(outerColor);
		glVertex2fv(corona[4]);
		myColor4fv(mainColor);
		glVertex2fv(core[6]);
		myColor4fv(outerColor);
		glVertex2fv(corona[5]);
		myColor4fv(mainColor);
		glVertex2fv(core[7]);
		myColor4fv(outerColor);
		glVertex2fv(corona[6]);
		myColor4fv(mainColor);
		glVertex2fv(core[8]);
		myColor4fv(outerColor);
		glVertex2fv(corona[7]);
		myColor4fv(mainColor);
		glVertex2fv(core[1]);
		myColor4fv(outerColor);
		glVertex2fv(corona[0]);
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
      if (++v == cv) v = 0;
    }
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

