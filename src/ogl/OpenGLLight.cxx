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

#include "common.h"

#include <math.h>

#include "OpenGLLight.h"
#include "OpenGLGState.h"
#include "ViewFrustum.h"


GLint OpenGLLight::maxLights = 0;


OpenGLLight::OpenGLLight()
: pos(0.0f, 0.0f, 1.0f, 0.0f)
, color(0.8f, 0.8f, 0.8f, 1.0f)
, atten(1.0f, 0.0f, 0.0f)
, maxDist(50.0f)
, onlyReal(false)
, onlyGround(false)
{
  //
  // Here's the equation for maxDist:
  //
  //   c - the cutoff value (ex: 0.02 = 2%)
  //   d - falloff distance where percentage is less then 1/r
  //
  //   (d^2 * atten[2]) + (d * atten[1]) + (atten[0] - (1/c)) = 0
  //
  // Most of the lights used in BZFlag seem to be under 50, so
  // it doesn't seem to be worth the bother or CPU time to actually
  // use this equation. Grep for 'Attenuation' to find the values.
  //
  makeLists();
  return;
}


OpenGLLight::OpenGLLight(const OpenGLLight& l)
{
  pos = l.pos;
  color = l.color;
  atten = l.atten;
  maxDist = l.maxDist;
  onlyReal = l.onlyReal;
  onlyGround = l.onlyGround;
  makeLists();
  return;
}


OpenGLLight& OpenGLLight::operator=(const OpenGLLight& l)
{
  if (this != &l) {
    freeLists();
    pos = l.pos;
    color = l.color;
    atten = l.atten;
    maxDist = l.maxDist;
    onlyReal = l.onlyReal;
    onlyGround = l.onlyGround;
  }
  return *this;
}


void OpenGLLight::makeLists()
{
  const int numLights = getMaxLights();

  // invalidate the lists
  lists = new GLuint[numLights];
  for (int i = 0; i < numLights; i++) {
    lists[i] = INVALID_GL_LIST_ID;
  }

  OpenGLGState::registerContextInitializer(freeContext,
					   initContext, (void*)this);
  return;
}


OpenGLLight::~OpenGLLight()
{
  OpenGLGState::unregisterContextInitializer(freeContext,
					     initContext, (void*)this);
  freeLists();
  delete[] lists;
  return;
}


void OpenGLLight::setDirection(const fvec3& _pos)
{
  freeLists();
  pos = fvec4(_pos, 0.0f);
}


void OpenGLLight::setPosition(const fvec3& _pos)
{
  freeLists();
  pos = fvec4(_pos, 1.0f);
}


void OpenGLLight::setColor(GLfloat r, GLfloat g, GLfloat b)
{
  freeLists();
  color = fvec4(r, g, b, 1.0f);
}


void OpenGLLight::setColor(const fvec4& rgba)
{
  freeLists();
  color = rgba;
}


void OpenGLLight::setAttenuation(const fvec3& _atten)
{
  freeLists();
  atten = _atten;
}


void OpenGLLight::setAttenuation(int index, GLfloat value)
{
  freeLists();
  atten[index] = value;
}


void OpenGLLight::setOnlyReal(bool value)
{
  freeLists();
  onlyReal = value;
  return;
}


void OpenGLLight::setOnlyGround(bool value)
{
  freeLists();
  onlyGround = value;
  return;
}


void OpenGLLight::calculateImportance(const ViewFrustum& frustum)
{
  // for away lights count the most
  // (shouldn't happen for dynamic lights?)
  if (pos[3] == 0.0f) {
    importance = MAXFLOAT;
    return;
  }

  // This is not an exact test, the real culling shape should
  // be a frustum with extended in all directions by maxDist.
  // The hard edges on the frustum are bogus zones.

  // check if the light is in front of the front viewing plane
  bool sphereCull = true;
  const GLfloat* p = frustum.getDirection();
  const float fd = (p[0] * pos[0]) +
		   (p[1] * pos[1]) +
		   (p[2] * pos[2]) + p[3];

  // cull against the frustum planes
  if (fd > 0.0f) {
    sphereCull = false; // don't need a sphere cull
    const int planeCount = frustum.getPlaneCount();
    for (int i = 1; i < planeCount; i++) {
      const float* plane = frustum.getSide(i);
      const float len = (plane[0] * pos[0]) +
			(plane[1] * pos[1]) +
			(plane[2] * pos[2]) + plane[3];
      if (len < -maxDist) {
	importance = -1.0f;
	return;
      }
    }
  }

  // calculate the distance
  const GLfloat* eye = frustum.getEye();
  const float v[3] = {
    (eye[0] - pos[0]),
    (eye[1] - pos[1]),
    (eye[2] - pos[2]),
  };
  float dist = (v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]);
  dist = sqrtf(dist);

  // do a sphere cull if requested
  if (sphereCull && (dist > maxDist)) {
    importance = -1.0f;
    return;
  }

  // compute the 'importance' factor
  if (dist == 0.0f) {
    importance = 0.5f * MAXFLOAT;
  } else {
    importance = 1.0f / dist;
  }

  return;
}


void OpenGLLight::enableLight(int index, bool on) // const
{
  const GLenum light = (GL_LIGHT0 + index);
  if (on) {
    glEnable(light);
  } else {
    glDisable(light);
  }
  return;
}


void OpenGLLight::execute(int index, bool useList) const
{
  if (!useList) {
    genLight((GLenum)(GL_LIGHT0 + index));
    return;
  }

  // setup the light parameters (buffered in
  // a display list), but do not turn it on.
  if (lists[index] != INVALID_GL_LIST_ID) {
    glCallList(lists[index]);
  }
  else {
    lists[index] = glGenLists(1);
    glNewList(lists[index], GL_COMPILE_AND_EXECUTE);
    genLight((GLenum)(GL_LIGHT0 + index));
    glEndList();
  }
  return;
}


void OpenGLLight::genLight(GLenum light) const
{
  glLightfv(light, GL_POSITION, pos);
  glLightfv(light, GL_DIFFUSE, color);
  glLightfv(light, GL_SPECULAR, color);
  glLighti(light, GL_SPOT_EXPONENT, 0);
  glLightf(light, GL_CONSTANT_ATTENUATION,  atten[0]);
  glLightf(light, GL_LINEAR_ATTENUATION,    atten[1]);
  glLightf(light, GL_QUADRATIC_ATTENUATION, atten[2]);
  return;
}


void OpenGLLight::freeLists()
{
  const int numLights = getMaxLights();
  for (int i = 0; i < numLights; i++) {
    if (lists[i] != INVALID_GL_LIST_ID) {
      glDeleteLists(lists[i], 1);
      lists[i] = INVALID_GL_LIST_ID;
    }
  }
  return;
}


GLint OpenGLLight::getMaxLights()
{
  if (maxLights == 0) {
    glGetIntegerv(GL_MAX_LIGHTS, &maxLights);
  }
  return maxLights;
}


void OpenGLLight::freeContext(void* self)
{
  ((OpenGLLight*)self)->freeLists();
  return;
}


void OpenGLLight::initContext(void* /*self*/)
{
  // execute() will rebuild the lists
  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
