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

#include <string.h>
#include "SceneNode.h"
#include "SceneRenderer.h"

void			(__stdcall *SceneNode::color3f)(GLfloat, GLfloat, GLfloat);
void			(__stdcall *SceneNode::color4f)(GLfloat, GLfloat, GLfloat, GLfloat);
void			(__stdcall *SceneNode::color3fv)(const GLfloat*);
void			(__stdcall *SceneNode::color4fv)(const GLfloat*);
void			(*SceneNode::stipple)(GLfloat);

SceneNode::SceneNode() : styleMailbox(0)
{
  static boolean init = False;
  if (!init) {
    init = True;
    setColorOverride(False);
  }

  setCenter(0.0f, 0.0f, 0.0f);
  setRadius(0.0f);
}

SceneNode::~SceneNode()
{
  // do nothing
}

#if defined(sun)
static void __stdcall	oglColor3f(GLfloat r, GLfloat g, GLfloat b)
				{ glColor3f(r, g, b); }
static void __stdcall	oglColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
				{ glColor4f(r, g, b, a); }
static void __stdcall	oglColor3fv(const GLfloat* v)
				{ glColor3fv(v); }
static void __stdcall	oglColor4fv(const GLfloat* v)
				{ glColor4fv(v); }
#endif

void __stdcall		SceneNode::noColor3f(GLfloat, GLfloat, GLfloat) { }
void __stdcall		SceneNode::noColor4f(
				GLfloat, GLfloat, GLfloat, GLfloat) { }
void __stdcall		SceneNode::noColor3fv(const GLfloat*) { }
void __stdcall		SceneNode::noColor4fv(const GLfloat*) { }
void			SceneNode::noStipple(GLfloat) { }

void			SceneNode::setColorOverride(boolean on)
{
  if (on) {
    color3f  = &noColor3f;
    color4f  = &noColor4f;
    color3fv = &noColor3fv;
    color4fv = &noColor4fv;
    stipple  = &noStipple;
  }
  else {
#if defined(sun)
    color3f  = &oglColor3f;
    color4f  = &oglColor4f;
    color3fv = &oglColor3fv;
    color4fv = &oglColor4fv;
#else
    color3f  = &::glColor3f;
    color4f  = &::glColor4f;
    color3fv = &::glColor3fv;
    color4fv = &::glColor4fv;
#endif
    stipple  = &OpenGLGState::setStipple;
  }
}

const GLfloat*		SceneNode::getSphere() const
{
  return sphere;
}

void			SceneNode::setRadius(GLfloat radiusSquared)
{
  sphere[3] = radiusSquared;
}

void			SceneNode::setCenter(const GLfloat center[3])
{
  sphere[0] = center[0];
  sphere[1] = center[1];
  sphere[2] = center[2];
}

void			SceneNode::setCenter(GLfloat x, GLfloat y, GLfloat z)
{
  sphere[0] = x;
  sphere[1] = y;
  sphere[2] = z;
}

void			SceneNode::setSphere(const GLfloat _sphere[4])
{
  sphere[0] = _sphere[0];
  sphere[1] = _sphere[1];
  sphere[2] = _sphere[2];
  sphere[3] = _sphere[3];
}

void			SceneNode::getRenderNodes(SceneRenderer& renderer)
{
  addShadowNodes(renderer);
  if (!cull(renderer.getViewFrustum())) {
    if (!renderer.testAndSetStyle(styleMailbox)) notifyStyleChange(renderer);
    addRenderNodes(renderer);
  }
}

void			SceneNode::forceNotifyStyleChange()
{
  styleMailbox--;
}

void			SceneNode::notifyStyleChange(const SceneRenderer&)
{
  // do nothing
}

void			SceneNode::addRenderNodes(SceneRenderer&)
{
  // do nothing
}

void			SceneNode::addShadowNodes(SceneRenderer&)
{
  // do nothing
}

const GLfloat*		SceneNode::getPlane() const
{
  return NULL;
}

void			SceneNode::addLight(SceneRenderer&)
{
  // do nothing
}

GLfloat			SceneNode::getDistance(const GLfloat* eye) const
{
  return (eye[0] - sphere[0]) * (eye[0] - sphere[0]) +
	 (eye[1] - sphere[1]) * (eye[1] - sphere[1]) +
	 (eye[2] - sphere[2]) * (eye[2] - sphere[2]);
}

int			SceneNode::split(const float*,
					SceneNode*&, SceneNode*&) const
{
  // can't split me
  return 1;
}

boolean			SceneNode::cull(const ViewFrustum& view) const
{
  // if center of object is outside view frustum and distance is
  // greater than radius of object then cull.
  for (int i = 0; i < 5; i++) {
    const GLfloat* norm = view.getSide(i);
    const GLfloat d = sphere[0] * norm[0] +
		      sphere[1] * norm[1] +
		      sphere[2] * norm[2] + norm[3];
    if (d < 0.0f && d * d > sphere[3]) return True;
  }
  return False;
}

//
// GLfloat2Array
//

GLfloat2Array::GLfloat2Array(const GLfloat2Array& a) :
				size(a.size)
{
  data = new GLfloat2[size];
  ::memcpy(data, a.data, size * sizeof(GLfloat2));
}

GLfloat2Array&		GLfloat2Array::operator=(const GLfloat2Array& a)
{
  if (this != &a) {
    delete[] data;
    size = a.size;
    data = new GLfloat2[size];
    ::memcpy(data, a.data, size * sizeof(GLfloat2));
  }
  return *this;
}

//
// GLfloat3Array
//

GLfloat3Array::GLfloat3Array(const GLfloat3Array& a) :
				size(a.size)
{
  data = new GLfloat3[size];
  ::memcpy(data, a.data, size * sizeof(GLfloat3));
}

GLfloat3Array&		GLfloat3Array::operator=(const GLfloat3Array& a)
{
  if (this != &a) {
    delete[] data;
    size = a.size;
    data = new GLfloat3[size];
    ::memcpy(data, a.data, size * sizeof(GLfloat3));
  }
  return *this;
}
// ex: shiftwidth=2 tabstop=8
