/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "global.h"
#include "TankSceneNode.h"
#include "ViewFrustum.h"
#include "SceneRenderer.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

// parts: body, turret, barrel, left tread, right tread

const int		TankSceneNode::numLOD = 3;
int			TankSceneNode::maxLevel = numLOD;

TankSceneNode::TankSceneNode(const GLfloat pos[3], const GLfloat forward[3]) :
				colorblind(False),
				hidden(False),
				invisible(False),
				clip(False),
				style(TankRenderNode::Normal),
				lowRenderNode(this),
				medRenderNode(this),
				highRenderNode(this),
				shadowRenderNode(this)
{
  // prepare geometry
  move(pos, forward);
  baseRadius = 0.25f * (TankLength * TankLength +
			TankWidth * TankWidth + TankHeight * TankHeight);
  setRadius(baseRadius);

  color[3] = 1.0f;
  setColor(1.0f, 1.0f, 1.0f);
  setExplodeFraction(0.0f);

  shadowRenderNode.setShadow();
}

TankSceneNode::~TankSceneNode()
{
  // do nothing
}

void			TankSceneNode::setColor(
				GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  const boolean oldTransparent = (color[3] != 1.0f);
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = a;
  transparent = (color[3] != 1.0f);
  if (transparent != oldTransparent) forceNotifyStyleChange();
}

void			TankSceneNode::setColor(const GLfloat* rgba)
{
  const boolean oldTransparent = (color[3] != 1.0f);
  color[0] = rgba[0];
  color[1] = rgba[1];
  color[2] = rgba[2];
  color[3] = rgba[3];
  transparent = (color[3] != 1.0f);
  if (transparent != oldTransparent) forceNotifyStyleChange();
}

void			TankSceneNode::setMaterial(const OpenGLMaterial& mat)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setMaterial(mat);
  gstate = builder.getState();
  forceNotifyStyleChange();
}

void			TankSceneNode::setTexture(const OpenGLTexture& texture)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(texture);
  gstate = builder.getState();
  forceNotifyStyleChange();
}

void			TankSceneNode::move(const GLfloat pos[3],
					const GLfloat forward[3])
{
  azimuth = 180.0f / M_PI*atan2f(forward[1], forward[0]);
  elevation = -180.0f / M_PI*atan2f(forward[2], hypotf(forward[0],forward[1]));
  setCenter(pos);
}

void			TankSceneNode::notifyStyleChange(
				const SceneRenderer& renderer)
{
  sort = !renderer.useZBuffer();
  blending = renderer.useBlending();
  OpenGLGStateBuilder builder(gstate);
  builder.enableTexture(renderer.useTexture());
  builder.enableMaterial(renderer.useLighting());
  builder.setSmoothing(renderer.useSmoothing());
  if (blending && transparent) {
    builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    builder.setStipple(1.0f);
  }
  else {
    builder.resetBlending();
    builder.setStipple(transparent ? 0.5f : 1.0f);
  }
  gstate = builder.getState();

  OpenGLGStateBuilder builder2(lightsGState);
  if (renderer.useSmoothing()) {
    builder2.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    builder2.setSmoothing();
  }
  else {
    builder2.resetBlending();
    builder2.setSmoothing(False);
  }
  lightsGState = builder2.getState();
}

void			TankSceneNode::addRenderNodes(
				SceneRenderer& renderer)
{
  // don't draw hidden tanks.  this is mainly to avoid drawing player's
  // tank when player is using view from tank.  can't simply not include
  // player, though, cos then we wouldn't get the tank's shadow.
  if (hidden || invisible) return;

  // pick level of detail
  TankRenderNode* node;
  const GLfloat* sphere = getSphere();
  const ViewFrustum& view = renderer.getViewFrustum();
  const float size = sphere[3] * view.getAreaFactor() /
					getDistance(view.getEye());
  if (maxLevel > 2 && size > 55.0f) node = &highRenderNode;
  else if (maxLevel > 1 && size > 25.0f) node = &medRenderNode;
  else node = &lowRenderNode;

  // if drawing in sorted order then decide which order
  if (sort || transparent) {
    const GLfloat* eye = view.getEye();
    GLfloat dx = eye[0] - sphere[0];
    GLfloat dy = eye[1] - sphere[1];
    const GLfloat d = dx * dx + dy * dy;
    dx += cosf(azimuth * M_PI / 180.0f);
    dy += sinf(azimuth * M_PI / 180.0f);
    const boolean towards = d < (dx * dx + dy * dy);
    // note: above test should take elevation into account
    const boolean above = sphere[2] > eye[2];
    node->sortOrder(above, towards);
  }

  renderer.addRenderNode(node, &gstate);
}

void			TankSceneNode::addShadowNodes(
				SceneRenderer& renderer)
{
  if (invisible) return;
  renderer.addShadowNode(&shadowRenderNode);
}

void			TankSceneNode::setColorblind(boolean on)
{
  colorblind = on;
}

void			TankSceneNode::setNormal()
{
  style = TankRenderNode::Normal;
  setRadius(baseRadius);
}

void			TankSceneNode::setObese()
{
  style = TankRenderNode::Obese;
  setRadius(ObeseFactor * ObeseFactor * baseRadius);
}

void			TankSceneNode::setTiny()
{
  style = TankRenderNode::Tiny;
  setRadius(TinyFactor * TinyFactor * baseRadius);
}

void			TankSceneNode::setNarrow()
{
  style = TankRenderNode::Narrow;
  setRadius(baseRadius);
}

void			TankSceneNode::setExplodeFraction(float t)
{
  explodeFraction = t;
}

void			TankSceneNode::setClipPlane(const GLfloat* plane)
{
  if (!plane) {
    clip = False;
  }
  else {
    clip = True;
    clipPlane[0] = GLdouble(plane[0]);
    clipPlane[1] = GLdouble(plane[1]);
    clipPlane[2] = GLdouble(plane[2]);
    clipPlane[3] = GLdouble(plane[3]);
  }
}

void			TankSceneNode::setHidden(boolean _hidden)
{
  hidden = _hidden;
  invisible = False;
}

void			TankSceneNode::setInvisible(boolean _invisible)
{
  invisible = _invisible;
  hidden = False;
}

void			TankSceneNode::setMaxLOD(int _maxLevel)
{
  maxLevel = _maxLevel;
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
  setRadius(4.0f * TankLength * TankLength);

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

void			TankIDLSceneNode::move(const GLfloat _plane[4])
{
  plane[0] = _plane[0];
  plane[1] = _plane[1];
  plane[2] = _plane[2];
  plane[3] = _plane[3];

  // compute new sphere
  const GLfloat* s = tank->getSphere();
  setCenter(s[0] + 1.5f * TankLength * plane[0],
	    s[1] + 1.5f * TankLength * plane[1],
	    s[2] + 1.5f * TankLength * plane[2]);
}

void			TankIDLSceneNode::notifyStyleChange(
				const SceneRenderer& renderer)
{
  OpenGLGStateBuilder builder(gstate);
  if (renderer.useBlending()) {
    builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    builder.setStipple(1.0f);
  }
  else {
    builder.resetBlending();
    builder.setStipple(0.5f);
  }
  gstate = builder.getState();
}

void			TankIDLSceneNode::addRenderNodes(
				SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, &gstate);
}

//
// TankIDLSceneNode::IDLRenderNode
//

const int		TankIDLSceneNode::IDLRenderNode::idlFaces[][5] = {
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
				// barrrel
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
				// barrrel
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

void			TankIDLSceneNode::IDLRenderNode::render()
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

  // compute projection point -- one TankLength in from plane
  const GLfloat pd = -1.0f * TankLength - plane[3];
  GLfloat origin[3];
  origin[0] = pd * plane[0];
  origin[1] = pd * plane[1];
  origin[2] = pd * plane[2];

  glPushMatrix();
    glTranslatef(sphere[0], sphere[1], sphere[2]);
    glRotatef(azimuth, 0.0f, 0.0f, 1.0f);

    glBegin(GL_QUADS);
    const int numFaces = sizeof(idlFaces) / sizeof(idlFaces[0]);
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

const GLfloat		TankSceneNode::TankRenderNode::centerOfGravity[][3] = {
				{ 0.000f,  0.0f, 1.5f * 0.68f },// body
				{ 3.252f,  0.0f, 1.532f },	// barrel
				{ 0.125f,  0.0f, 2.5f * 0.68f },// turret
				{ 0.000f,  0.7f, 0.5f * 0.68f },// left tread
				{ 0.000f, -0.7f, 0.5f * 0.68f }	// right tread
			};
GLfloat			TankSceneNode::TankRenderNode::vertexScale[3];
GLfloat			TankSceneNode::TankRenderNode::normalScale[3];

TankSceneNode::TankRenderNode::TankRenderNode(
				const TankSceneNode* _sceneNode) :
				sceneNode(_sceneNode),
				isShadow(False),
				above(False),
				towards(False)
{
  // prepare explosion rotations and translations
  for (int i = 0; i < 5; i++) {
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

  // watch for context recreation
  OpenGLGState::registerContextInitializer(initContext, (void*)this);
}

TankSceneNode::TankRenderNode::~TankRenderNode()
{
  OpenGLGState::unregisterContextInitializer(initContext, (void*)this);
}

void			TankSceneNode::TankRenderNode::setShadow()
{
  isShadow = True;
}

void			TankSceneNode::TankRenderNode::sortOrder(
				boolean _above, boolean _towards)
{
  above = _above;
  towards = _towards;
}

void			TankSceneNode::TankRenderNode::render()
{
  static const GLfloat colorblindColor[3] = { 0.25f, 0.25f, 0.25f };

  base = getParts(sceneNode->style);
  explodeFraction = sceneNode->explodeFraction;
  isExploding = (explodeFraction != 0.0f);
  color = sceneNode->colorblind ? colorblindColor : sceneNode->color;
  alpha = sceneNode->color[3];

  if (!sceneNode->blending && sceneNode->transparent) myStipple(alpha);
  if (sceneNode->clip) {
    glClipPlane(GL_CLIP_PLANE0, sceneNode->clipPlane);
    glEnable(GL_CLIP_PLANE0);
  }

  const GLfloat* sphere = sceneNode->getSphere();
  glPushMatrix();
    glTranslatef(sphere[0], sphere[1], sphere[2]);
    glRotatef(sceneNode->azimuth, 0.0f, 0.0f, 1.0f);
    glRotatef(sceneNode->elevation, 0.0f, 1.0f, 0.0f);

    if (!isShadow && (sceneNode->sort || sceneNode->transparent)) {
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
    else {
      // any old order is fine.  if exploding then draw both sides.
      if (isExploding) glDisable(GL_CULL_FACE);
      renderPart(LeftTread);
      renderPart(RightTread);
      renderPart(Body);
      renderPart(Turret);
      renderPart(Barrel);
      if (isExploding) glEnable(GL_CULL_FACE);
    }

  glPopMatrix();

  if (!isExploding && !isShadow) {
    // FIXME -- add flare lights using addFlareLight().  pass
    // light position in world space.
  }

  glShadeModel(GL_FLAT);
  if (!sceneNode->blending && sceneNode->transparent) myStipple(0.5f);
  if (sceneNode->clip) glDisable(GL_CLIP_PLANE0);
}

void			TankSceneNode::TankRenderNode::renderParts()
{
  // draw parts in back to front order
  if (!above) {
    renderPart(LeftTread);
    renderPart(RightTread);
    renderPart(Body);
    if (towards) {
      renderPart(Turret);
      renderPart(Barrel);
    }
    else {
      renderPart(Barrel);
      renderPart(Turret);
    }
  }
  else {
    if (towards) {
      renderPart(Turret);
      renderPart(Barrel);
    }
    else {
      renderPart(Barrel);
      renderPart(Turret);
    }
    renderPart(Body);
    renderPart(LeftTread);
    renderPart(RightTread);
  }
}

void			TankSceneNode::TankRenderNode::renderPart(Part part)
{
  // apply explosion transform
  if (isExploding) {
    glPushMatrix();
    glTranslatef(centerOfGravity[part][0] + explodeFraction * vel[part][0],
		 centerOfGravity[part][1] + explodeFraction * vel[part][1],
		 centerOfGravity[part][2]);
    glRotatef(spin[part][3] * explodeFraction,
		 spin[part][0], spin[part][1], spin[part][2]);
    glTranslatef(-centerOfGravity[part][0],
		 -centerOfGravity[part][1],
		 -centerOfGravity[part][2]);
  }

  // set color
  switch (part) {
    case Body:
      myColor4f(color[0], color[1], color[2], alpha);
      break;

    case Barrel:
      myColor4f(0.25f, 0.25f, 0.25f, alpha);
      break;

    case Turret:
      myColor4f(0.9f * color[0], 0.9f * color[1], 0.9f * color[2], alpha);
      break;

    case LeftTread:
    case RightTread:
      myColor4f(0.5f * color[0], 0.5f * color[1], 0.5f * color[2], alpha);
      break;
  }

  // draw part
  glCallList(base + part);

  if (part == Turret && !isExploding && !isShadow) {
    renderLights();
  }

  // restore transform
  if (isExploding) glPopMatrix();
}

void			TankSceneNode::TankRenderNode::prepStyle(Style style)
{
  static const GLfloat styleFactors[4][3] = {
				{ 1.0f, 1.0f, 1.0f },
				{ ObeseFactor, ObeseFactor, 1.0f },
				{ TinyFactor, TinyFactor, 1.0f },
				{ 1.0f, 0.0f, 1.0f }
			};
  vertexScale[0] = styleFactors[style][0];
  vertexScale[1] = styleFactors[style][1];
  vertexScale[2] = styleFactors[style][2];
  normalScale[0] = (vertexScale[0] == 0.0f ? 0.0f : 1.0f / vertexScale[0]);
  normalScale[1] = (vertexScale[1] == 0.0f ? 0.0f : 1.0f / vertexScale[1]);
  normalScale[2] = (vertexScale[2] == 0.0f ? 0.0f : 1.0f / vertexScale[2]);
}

void			TankSceneNode::TankRenderNode::
				doVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
  glVertex3f(x * vertexScale[0], y * vertexScale[1], z * vertexScale[2]);
}

void			TankSceneNode::TankRenderNode::
				doNormal3f(GLfloat x, GLfloat y, GLfloat z)
{
  const GLfloat d = hypotf(x * normalScale[0],
			hypotf(y * normalScale[1], z * normalScale[2]));
  if (d > 1.0e-5f) {
    x *= normalScale[0] / d;
    y *= normalScale[1] / d;
    z *= normalScale[2] / d;
  }
  glNormal3f(x, y, z);
}

void			TankSceneNode::TankRenderNode::doInitContext()
{
  freeParts();
}

void			TankSceneNode::TankRenderNode::initContext(void* self)
{
  ((TankSceneNode::TankRenderNode*)self)->doInitContext();
}

//
// TankSceneNode::LowTankRenderNode
//

GLuint			TankSceneNode::LowTankRenderNode::parts[4] =
				{ 0, 0, 0, 0 };

TankSceneNode::LowTankRenderNode::LowTankRenderNode(
				const TankSceneNode* sceneNode) :
				TankRenderNode(sceneNode)
{
  // do nothing
}

TankSceneNode::LowTankRenderNode::~LowTankRenderNode()
{
  // do nothing
}

GLuint			TankSceneNode::LowTankRenderNode::
				getParts(Style style)
{
  if (parts[style] == 0) {
    prepStyle(style);

    parts[style] = glGenLists(5);
    glNewList(parts[style] + Body, GL_COMPILE);
      makeBody();
    glEndList();

    glNewList(parts[style] + Barrel, GL_COMPILE);
      makeBarrel();
    glEndList();

    glNewList(parts[style] + Turret, GL_COMPILE);
      makeTurret();
    glEndList();

    glNewList(parts[style] + LeftTread, GL_COMPILE);
      makeLeftTread();
    glEndList();

    glNewList(parts[style] + RightTread, GL_COMPILE);
      makeRightTread();
    glEndList();
  }

  return parts[style];
}

void			TankSceneNode::LowTankRenderNode::freeParts()
{
  // forget about old parts
  for (unsigned int i = 0; i < sizeof(parts) / sizeof(parts[0]); i++)
    parts[i] = 0;
}

//
// TankSceneNode::MedTankRenderNode
//

GLuint			TankSceneNode::MedTankRenderNode::parts[4] =
				{ 0, 0, 0, 0 };

TankSceneNode::MedTankRenderNode::MedTankRenderNode(
				const TankSceneNode* sceneNode) :
				TankRenderNode(sceneNode)
{
  // do nothing
}

TankSceneNode::MedTankRenderNode::~MedTankRenderNode()
{
  // do nothing
}

GLuint			TankSceneNode::MedTankRenderNode::
				getParts(Style style)
{
  if (parts[style] == 0) {
    prepStyle(style);

    parts[style] = glGenLists(5);
    glNewList(parts[style] + Body, GL_COMPILE);
      makeBody();
    glEndList();

    glNewList(parts[style] + Barrel, GL_COMPILE);
      makeBarrel();
    glEndList();

    glNewList(parts[style] + Turret, GL_COMPILE);
      makeTurret();
    glEndList();

    glNewList(parts[style] + LeftTread, GL_COMPILE);
      makeLeftTread();
    glEndList();

    glNewList(parts[style] + RightTread, GL_COMPILE);
      makeRightTread();
    glEndList();
  }

  return parts[style];
}

void			TankSceneNode::MedTankRenderNode::freeParts()
{
  // forget about old parts
  for (unsigned int i = 0; i < sizeof(parts) / sizeof(parts[0]); i++)
    parts[i] = 0;
}

//
// TankSceneNode::HighTankRenderNode
//

GLuint			TankSceneNode::HighTankRenderNode::parts[4] =
				{ 0, 0, 0, 0 };

TankSceneNode::HighTankRenderNode::HighTankRenderNode(
				const TankSceneNode* sceneNode) :
				TankRenderNode(sceneNode)
{
  // do nothing
}

TankSceneNode::HighTankRenderNode::~HighTankRenderNode()
{
  // do nothing
}

GLuint			TankSceneNode::HighTankRenderNode::
				getParts(Style style)
{
  if (parts[style] == 0) {
    prepStyle(style);

    parts[style] = glGenLists(5);
    glNewList(parts[style] + Body, GL_COMPILE);
      makeBody();
    glEndList();

    glNewList(parts[style] + Barrel, GL_COMPILE);
      makeBarrel();
    glEndList();

    glNewList(parts[style] + Turret, GL_COMPILE);
      makeTurret();
    glEndList();

    glNewList(parts[style] + LeftTread, GL_COMPILE);
      makeLeftTread();
    glEndList();

    glNewList(parts[style] + RightTread, GL_COMPILE);
      makeRightTread();
    glEndList();
  }

  return parts[style];
}

void			TankSceneNode::HighTankRenderNode::freeParts()
{
  // forget about old parts
  for (unsigned int i = 0; i < sizeof(parts) / sizeof(parts[0]); i++)
    parts[i] = 0;
}

//
// geometry rendering methods
//

#define	glVertex3f	doVertex3f
#define	glNormal3f	doNormal3f

void			TankSceneNode::TankRenderNode::renderLights()
{
  static const GLfloat	lights[3][6] = {
				{ 1.0f, 1.0f, 1.0f, -1.53f,  0.00f, 2.1f },
				{ 1.0f, 0.0f, 0.0f,  0.10f,  0.75f, 2.1f },
				{ 0.0f, 1.0f, 0.0f,  0.10f, -0.75f, 2.1f }
			};

  sceneNode->lightsGState.setState();
  glPointSize(2.0f);
  glBegin(GL_POINTS);
    myColor3fv(lights[0]);
    glVertex3f(lights[0][3], lights[0][4], lights[0][5]);
    myColor3fv(lights[1]);
    glVertex3f(lights[1][3], lights[1][4], lights[1][5]);
    myColor3fv(lights[2]);
    glVertex3f(lights[2][3], lights[2][4], lights[2][5]);
  glEnd();
  glPointSize(1.0f);
  sceneNode->gstate.setState();
}

//
// TankSceneNode::LowTankRenderNode
//

void			TankSceneNode::LowTankRenderNode::makeBody()
{
#include "models/lowtank/body.c"
}

void			TankSceneNode::LowTankRenderNode::makeBarrel()
{
#include "models/lowtank/barrel.c"
}

void			TankSceneNode::LowTankRenderNode::makeTurret()
{
#include "models/lowtank/turret.c"
}

void			TankSceneNode::LowTankRenderNode::makeLeftTread()
{
#include "models/lowtank/ltread.c"
}

void			TankSceneNode::LowTankRenderNode::makeRightTread()
{
#include "models/lowtank/rtread.c"
}

//
// TankSceneNode::MedTankRenderNode
//

void			TankSceneNode::MedTankRenderNode::makeBody()
{
#include "models/medtank/body.c"
}

void			TankSceneNode::MedTankRenderNode::makeBarrel()
{
#include "models/medtank/barrel.c"
}

void			TankSceneNode::MedTankRenderNode::makeTurret()
{
#include "models/medtank/turret.c"
}

void			TankSceneNode::MedTankRenderNode::makeLeftTread()
{
#include "models/medtank/ltread.c"
}

void			TankSceneNode::MedTankRenderNode::makeRightTread()
{
#include "models/medtank/rtread.c"
}

//
// TankSceneNode::HighTankRenderNode
//

void			TankSceneNode::HighTankRenderNode::makeBody()
{
#include "models/hitank/body.c"
}

void			TankSceneNode::HighTankRenderNode::makeBarrel()
{
#include "models/hitank/barrel.c"
}

void			TankSceneNode::HighTankRenderNode::makeTurret()
{
#include "models/hitank/turret.c"
}

void			TankSceneNode::HighTankRenderNode::makeLeftTread()
{
#include "models/hitank/ltread.c"
}

void			TankSceneNode::HighTankRenderNode::makeRightTread()
{
#include "models/hitank/rtread.c"
}
// ex: shiftwidth=2 tabstop=8
