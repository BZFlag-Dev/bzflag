/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <math.h>
#include "common.h"
#include "OpenGLLight.h"
#include "OpenGLGState.h"
#include "ViewFrustum.h"

static class OpenGLLightCleanup {
  public:
			OpenGLLightCleanup() { }
			~OpenGLLightCleanup() { OpenGLLight::cleanup(); }
} cleanup;

GLint			OpenGLLight::maxLights = 0;
OpenGLLightDLStack	OpenGLLight::oldLists;


OpenGLLight::OpenGLLight() : mailbox(0)
{
  pos[0] = 0.0f;
  pos[1] = 0.0f;
  pos[2] = 1.0f;
  pos[3] = 0.0f;
  color[0] = 0.8f;
  color[1] = 0.8f;
  color[2] = 0.8f;
  color[3] = 1.0f;
  atten[0] = 1.0f;
  atten[1] = 0.0f;
  atten[2] = 0.0f;
  maxDist = 50.0f;
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
}


OpenGLLight::OpenGLLight(const OpenGLLight& l) : mailbox(0)
{
  pos[0] = l.pos[0];
  pos[1] = l.pos[1];
  pos[2] = l.pos[2];
  pos[3] = l.pos[3];
  color[0] = l.color[0];
  color[1] = l.color[1];
  color[2] = l.color[2];
  color[3] = l.color[3];
  atten[0] = l.atten[0];
  atten[1] = l.atten[1];
  atten[2] = l.atten[2];
  makeLists();
}


OpenGLLight::~OpenGLLight()
{
  OpenGLGState::unregisterContextInitializer(initContext, (void*)this);

  // put display lists on oldLists list
  oldLists.push_back(listBase);
  delete[] list;
}


OpenGLLight& OpenGLLight::operator=(const OpenGLLight& l)
{
  if (this != &l) {
    freeLists();
    pos[0] = l.pos[0];
    pos[1] = l.pos[1];
    pos[2] = l.pos[2];
    pos[3] = l.pos[3];
    color[0] = l.color[0];
    color[1] = l.color[1];
    color[2] = l.color[2];
    color[3] = l.color[3];
    atten[0] = l.atten[0];
    atten[1] = l.atten[1];
    atten[2] = l.atten[2];
  }
  return *this;
}


void OpenGLLight::setDirection(const GLfloat* _pos)
{
  freeLists();
  pos[0] = _pos[0];
  pos[1] = _pos[1];
  pos[2] = _pos[2];
  pos[3] = 0.0f;
}


void OpenGLLight::setPosition(const GLfloat* _pos)
{
  freeLists();
  pos[0] = _pos[0];
  pos[1] = _pos[1];
  pos[2] = _pos[2];
  pos[3] = 1.0f;
}


void OpenGLLight::setColor(GLfloat r, GLfloat g, GLfloat b)
{
  freeLists();
  color[0] = r;
  color[1] = g;
  color[2] = b;
}


void OpenGLLight::setColor(const GLfloat* rgb)
{
  freeLists();
  color[0] = rgb[0];
  color[1] = rgb[1];
  color[2] = rgb[2];
}


void OpenGLLight::setAttenuation(const GLfloat* _atten)
{
  freeLists();
  atten[0] = _atten[0];
  atten[1] = _atten[1];
  atten[2] = _atten[2];
}


void OpenGLLight::setAttenuation(int index, GLfloat value)
{
  freeLists();
  atten[index] = value;
}


void OpenGLLight::setImportance(const ViewFrustum& frustum)
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
  // (right, left, up, and down)
  if (fd > 0.0f) {
    sphereCull = false; // don't need a sphere cull
    for (int i = 1; i < 5; i++) {
      const float* p = frustum.getSide(i);
      const float len = (p[0] * pos[0]) +
                        (p[1] * pos[1]) +
                        (p[2] * pos[2]) + p[3];
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


void OpenGLLight::execute(int index) const
{
  if (list[index] != mailbox) {
    list[index] = mailbox;
    glNewList(listBase + index, GL_COMPILE);
    genLight((GLenum)(GL_LIGHT0 + index));
    glEndList();
  }
  glCallList(listBase + index);
}


void OpenGLLight::genLight(GLenum light) const
{
  glLightfv(light, GL_POSITION, pos);
  glLightfv(light, GL_DIFFUSE, color);
  glLightfv(light, GL_SPECULAR, color);
  glLighti(light, GL_SPOT_EXPONENT, 0);
  glLightf(light, GL_CONSTANT_ATTENUATION, atten[0]);
  glLightf(light, GL_LINEAR_ATTENUATION, atten[1]);
  glLightf(light, GL_QUADRATIC_ATTENUATION, atten[2]);
}


void OpenGLLight::makeLists()
{
  // make all lights dirty
  const int numLights = getMaxLights();
  list = new GLuint[numLights];
  for (int i = 0; i < numLights; i++)
    list[i] = mailbox;
  freeLists();

  // make display lists
  const int count = oldLists.size();
  if (count != 0) {
    listBase = oldLists[count - 1];
    oldLists.pop_back();
  }
  else {
    listBase = glGenLists(numLights);
  }

  // watch for context recreation
  OpenGLGState::registerContextInitializer(initContext, (void*)this);
}


void OpenGLLight::freeLists()
{
  mailbox++;
}


GLint OpenGLLight::getMaxLights()
{
  if (maxLights == 0)
    glGetIntegerv(GL_MAX_LIGHTS, &maxLights);
  return maxLights;
}


void OpenGLLight::enableLight(int index,
						bool on) // const
{
  if (on) glEnable((GLenum)(GL_LIGHT0 + index));
  else glDisable((GLenum)(GL_LIGHT0 + index));
}


void OpenGLLight::cleanup()
{
  // free all display lists on oldLists list
  const int count = oldLists.size();
  for (int i = 0; i < count; i++) {
    // FIXME -- can't call safely since context is probably gone
    // glDeleteLists(oldLists[i], getMaxLights());
  }
}


void OpenGLLight::initContext(void* self)
{
  // cause display list to be recreated on next execute()
  ((OpenGLLight*)self)->freeLists();
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

