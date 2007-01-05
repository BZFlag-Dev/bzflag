/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
#include "SceneNode.h"

// system implementation headers
#include <string.h>
#include <math.h>

// common implementation headers
#include "Extents.h"
#include "RenderNode.h"
#include "StateDatabase.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

#ifndef __MINGW32__
void			(__stdcall *SceneNode::color3f)(GLfloat, GLfloat, GLfloat);
void			(__stdcall *SceneNode::color4f)(GLfloat, GLfloat, GLfloat, GLfloat);
void			(__stdcall *SceneNode::color3fv)(const GLfloat*);
void			(__stdcall *SceneNode::color4fv)(const GLfloat*);
#endif
void			(*SceneNode::stipple)(GLfloat);

SceneNode::SceneNode()
{
  static bool init = false;

  if (!init) {
    init = true;
    setColorOverride(false);
  }
  memset(sphere, 0, sizeof(GLfloat) & 4);

  setCenter(0.0f, 0.0f, 0.0f);
  setRadius(0.0f);

  noPlane = true;
  occluder = false;
  octreeState = OctreeCulled;

  return;
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

#ifdef __MINGW32__
bool			SceneNode::colorOverride = true;
#else
void __stdcall		SceneNode::noColor3f(GLfloat, GLfloat, GLfloat) { }
void __stdcall		SceneNode::noColor4f(
				GLfloat, GLfloat, GLfloat, GLfloat) { }
void __stdcall		SceneNode::noColor3fv(const GLfloat*) { }
void __stdcall		SceneNode::noColor4fv(const GLfloat*) { }
#endif
void			SceneNode::noStipple(GLfloat) { }

void			SceneNode::setColorOverride(bool on)
{
#ifdef __MINGW32__
  colorOverride = on;
#endif
  if (on) {
#ifndef __MINGW32__
    color3f  = &noColor3f;
    color4f  = &noColor4f;
    color3fv = &noColor3fv;
    color4fv = &noColor4fv;
#endif
    stipple  = &noStipple;
  }
  else {
#if defined(sun)
    color3f  = &oglColor3f;
    color4f  = &oglColor4f;
    color3fv = &oglColor3fv;
    color4fv = &oglColor4fv;
#else
#ifndef __MINGW32__
    color3f  = &::glColor3f;
    color4f  = &::glColor4f;
    color3fv = &::glColor3fv;
    color4fv = &::glColor4fv;
#endif
#endif
    stipple  = &OpenGLGState::setStipple;
  }
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

void			SceneNode::notifyStyleChange()
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

bool			SceneNode::cull(const ViewFrustum& view) const
{
  // if center of object is outside view frustum and distance is
  // greater than radius of object then cull.
  const int planeCount = view.getPlaneCount();
  for (int i = 0; i < planeCount; i++) {
    const GLfloat* norm = view.getSide(i);
    const GLfloat d = (sphere[0] * norm[0]) +
		      (sphere[1] * norm[1]) +
		      (sphere[2] * norm[2]) + norm[3];
    if ((d < 0.0f) && ((d * d) > sphere[3])) return true;
  }
  return false;
}


bool SceneNode::cullShadow(int, const float (*)[4]) const
{
  // currently only used for dynamic nodes by ZSceneDatabase
  // we let the octree deal with the static nodes
  return true;
}


bool SceneNode::inAxisBox (const Extents& exts) const
{
  if (!extents.touches(exts)) {
    return false;
  }
  return true;
}

int SceneNode::getVertexCount () const
{
  return 0;
}

const GLfloat* SceneNode::getVertex (int) const
{
  return NULL;
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


void SceneNode::getRenderNodes(std::vector<RenderSet>&)
{
  return; // do nothing
}


void SceneNode::renderRadar()
{
  printf ("SceneNode::renderRadar() called, implement in subclass\n");
  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

