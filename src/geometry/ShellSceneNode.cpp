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
#include "ShellSceneNode.h"

// system headers
#include <math.h>

// common headers
#include "bzfgl.h"
#include "ogl/OpenGLMaterial.h"
#include "common/StateDatabase.h"
#include "bzflag/SceneRenderer.h" // FIXME (SceneRenderer.cpp is in src/bzflag)

#define ShellRadius1_2  ((float)(M_SQRT1_2 * ShellRadius))

const fvec3 ShellSceneNode::shellVertex[9] = {
  fvec3(3.0f * ShellRadius, 0.0f, 0.0f),
  fvec3(0.0f, -ShellRadius, 0.0f),
  fvec3(0.0f, -ShellRadius1_2, -ShellRadius1_2),
  fvec3(0.0f, 0.0f, -ShellRadius),
  fvec3(0.0f, ShellRadius1_2, -ShellRadius1_2),
  fvec3(0.0f, ShellRadius, 0.0f),
  fvec3(0.0f, ShellRadius1_2, ShellRadius1_2),
  fvec3(0.0f, 0.0f, ShellRadius),
  fvec3(0.0f, -ShellRadius1_2, ShellRadius1_2)
};

const fvec3 ShellSceneNode::shellNormal[10] = {
  fvec3(1.0f, 0.0f, 0.0f),
  fvec3(0.0f, -1.0f, 0.0f),
  fvec3(0.0f, (float) - M_SQRT1_2, (float) - M_SQRT1_2),
  fvec3(0.0f, 0.0f, -1.0f),
  fvec3(0.0f, (float)M_SQRT1_2, (float) - M_SQRT1_2),
  fvec3(0.0f, 1.0f, 0.0f),
  fvec3(0.0f, (float)M_SQRT1_2, (float)M_SQRT1_2),
  fvec3(0.0f, 0.0f, 1.0f),
  fvec3(0.0f, (float) - M_SQRT1_2, (float)M_SQRT1_2),
  fvec3(-1.0f, 0.0f, 0.0f)
};


ShellSceneNode::ShellSceneNode(const fvec3& pos, const fvec3& forward) :
  renderNode(this) {
  static const fvec4 specular(1.0f, 1.0f, 1.0f, 1.0f);
  static const fvec4 emissive(0.0f, 0.0f, 0.0f, 1.0f);

  move(pos, forward);
  setRadius(9.0f * ShellRadius * ShellRadius);

  OpenGLGStateBuilder builder(gstate);
  builder.setMaterial(OpenGLMaterial(specular, emissive, 20.0f),
                      RENDERER.useQuality() > _LOW_QUALITY);
  gstate = builder.getState();
}


ShellSceneNode::~ShellSceneNode() {
  // do nothing
}


void ShellSceneNode::move(const fvec3& pos, const fvec3& forward) {
  setCenter(pos);
  azimuth   = (float)(RAD2DEG * atan2f(forward.y, forward.x));
  elevation = (float)(-RAD2DEG * atan2f(forward.z, hypotf(forward.x, forward.y)));
}


void ShellSceneNode::notifyStyleChange() {
  OpenGLGStateBuilder builder(gstate);
  const bool lighting = BZDB.isTrue("lighting");
  builder.enableMaterial(lighting);
  builder.setShading(lighting ? GL_SMOOTH : GL_FLAT);
  renderNode.setLighting(lighting);
  gstate = builder.getState();
}


void ShellSceneNode::addRenderNodes(SceneRenderer& renderer) {
  renderer.addRenderNode(&renderNode, &gstate);
}


void ShellSceneNode::addShadowNodes(SceneRenderer& renderer) {
  renderer.addShadowNode(&renderNode);
}


//
// ShellSceneNode::ShellRenderNode
//

ShellSceneNode::ShellRenderNode::ShellRenderNode(
  const ShellSceneNode* _sceneNode) :
  sceneNode(_sceneNode),
  lighted(false) {
  // do nothing
}


ShellSceneNode::ShellRenderNode::~ShellRenderNode() {
  // do nothing
}


void ShellSceneNode::ShellRenderNode::setLighting(bool _lighted) {
  lighted = _lighted;
}


void ShellSceneNode::ShellRenderNode::render() {
  const fvec4& sphere = sceneNode->getSphere();
  glPushMatrix();
  glTranslatef(sphere.x, sphere.y, sphere.z);
  glRotatef(sceneNode->azimuth, 0.0f, 0.0f, 1.0f);
  glRotatef(sceneNode->elevation, 0.0f, 1.0f, 0.0f);

  myColor3f(0.7f, 0.7f, 0.7f);
  glBegin(GL_TRIANGLE_FAN);
  if (lighted) {
    glNormal3fv(shellNormal[0]);
    glVertex3fv(shellVertex[0]);
    glNormal3fv(shellNormal[1]);
    glVertex3fv(shellVertex[1]);
    glNormal3fv(shellNormal[2]);
    glVertex3fv(shellVertex[2]);
    glNormal3fv(shellNormal[3]);
    glVertex3fv(shellVertex[3]);
    glNormal3fv(shellNormal[4]);
    glVertex3fv(shellVertex[4]);
    glNormal3fv(shellNormal[5]);
    glVertex3fv(shellVertex[5]);
    glNormal3fv(shellNormal[6]);
    glVertex3fv(shellVertex[6]);
    glNormal3fv(shellNormal[7]);
    glVertex3fv(shellVertex[7]);
    glNormal3fv(shellNormal[8]);
    glVertex3fv(shellVertex[8]);
    glNormal3fv(shellNormal[1]);
    glVertex3fv(shellVertex[1]);
  }
  else {
    glVertex3fv(shellVertex[0]);
    glVertex3fv(shellVertex[1]);
    glVertex3fv(shellVertex[2]);
    glVertex3fv(shellVertex[3]);
    glVertex3fv(shellVertex[4]);
    glVertex3fv(shellVertex[5]);
    glVertex3fv(shellVertex[6]);
    glVertex3fv(shellVertex[7]);
    glVertex3fv(shellVertex[8]);
    glVertex3fv(shellVertex[1]);
  }
  glEnd();

  glPopMatrix();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
