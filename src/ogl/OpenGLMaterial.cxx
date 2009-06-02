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

// interface header
#include "OpenGLMaterial.h"

// common headers
#include "bzfgl.h"
#include "OpenGLGState.h"

//
// OpenGLMaterial::Rep
//

OpenGLMaterial::Rep* OpenGLMaterial::Rep::head = NULL;

OpenGLMaterial::Rep* OpenGLMaterial::Rep::getRep(
				const fvec4& specular,
				const fvec4& emissive,
				float shininess)
{
  // see if we've already got an identical material
  for (Rep* scan = head; scan; scan = scan->next) {
    if (shininess != scan->shininess) {
      continue;
    }
    if (specular.rgb() != scan->specular.rgb()) {
      continue;
    }
    if (emissive.rgb() != scan->emissive.rgb()) {
      continue;
    }
    scan->ref();
    return scan;
  }

  // nope, make a new one
  return new Rep(specular, emissive, shininess);
}

OpenGLMaterial::Rep::Rep(const fvec4& _specular,
			 const fvec4& _emissive,
			 float _shininess)
: refCount(1)
, shininess(_shininess)
{
  list = INVALID_GL_LIST_ID;

  prev = NULL;
  next = head;
  head = this;
  if (next) next->prev = this;

  specular.rgb() = _specular.rgb();
  specular.a = 1.0f;
  emissive.rgb() = _emissive.rgb();
  emissive.a = 1.0f;

  OpenGLGState::registerContextInitializer(freeContext,
					   initContext, (void*)this);
}

OpenGLMaterial::Rep::~Rep()
{
  OpenGLGState::unregisterContextInitializer(freeContext,
					     initContext, (void*)this);

  // free OpenGL display list
  if (list != INVALID_GL_LIST_ID) {
    glDeleteLists(list, 1);
    list = INVALID_GL_LIST_ID;
  }

  // remove me from material list
  if (next != NULL) next->prev = prev;
  if (prev != NULL) prev->next = next;
  else head = next;
}

void			OpenGLMaterial::Rep::ref()
{
  refCount++;
}

void			OpenGLMaterial::Rep::unref()
{
  if (--refCount == 0) delete this;
}

void			OpenGLMaterial::Rep::execute()
{
  if (list != INVALID_GL_LIST_ID) {
    glCallList(list);
  }
  else {
    list = glGenLists(1);
    glNewList(list, GL_COMPILE_AND_EXECUTE);
    {
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
      glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissive);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
      if (highQuality) {
	if  ((specular.r > 0.0f) ||
	     (specular.g > 0.0f) ||
	     (specular.b > 0.0f)) {
	  // accurate specular highlighting  (more GPU intensive)
	  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	} else {
	  // speed up the lighting calcs by simplifying
	  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
	}
      }
    }
    glEndList();
  }
  return;
}


void OpenGLMaterial::Rep::freeContext(void* self)
{
  unsigned int& list = ((Rep*)self)->list;
  if (list != INVALID_GL_LIST_ID) {
    glDeleteLists(list, 1);
    list = INVALID_GL_LIST_ID;
  }
  return;
}


void OpenGLMaterial::Rep::initContext(void* /*self*/)
{
  // the next execute() call will rebuild the list
  return;
}


//
// OpenGLMaterial
//

OpenGLMaterial::OpenGLMaterial()
{
  rep = NULL;
}

OpenGLMaterial::OpenGLMaterial(const fvec4& specular,
                               const fvec4& emissive,
                               float shininess)
{
  rep = Rep::getRep(specular, emissive, shininess);
}

OpenGLMaterial::OpenGLMaterial(const OpenGLMaterial& m)
{
  rep = m.rep;
  if (rep) {
    rep->highQuality = false;
    rep->ref();
  }

}

OpenGLMaterial::~OpenGLMaterial()
{
  if (rep) rep->unref();
}

OpenGLMaterial&		OpenGLMaterial::operator=(const OpenGLMaterial& m)
{
  if (rep != m.rep) {
    if (rep) rep->unref();
    rep = m.rep;
    if (rep) rep->ref();
  }
  return *this;
}

bool			OpenGLMaterial::operator==(const OpenGLMaterial& m) const
{
  return rep == m.rep;
}

bool			OpenGLMaterial::operator!=(const OpenGLMaterial& m) const
{
  return rep != m.rep;
}

bool			OpenGLMaterial::operator<(const OpenGLMaterial& m) const
{
  if (rep == m.rep) return false;
  if (!m.rep) return false;
  if (!rep) return true;
  return (rep->list < m.rep->list);
}

bool			OpenGLMaterial::isValid() const
{
  return (rep != NULL);
}

void			OpenGLMaterial::execute() const
{
  if (rep) rep->execute();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
