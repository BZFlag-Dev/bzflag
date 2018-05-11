/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
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
#include "OpenGLMaterial.h"
#include "OpenGLGState.h"
#include "SceneRenderer.h"

//
// OpenGLMaterial::Rep
//

OpenGLMaterial::Rep*	OpenGLMaterial::Rep::head = NULL;

OpenGLMaterial::Rep*	OpenGLMaterial::Rep::getRep(
				const GLfloat* specular,
				const GLfloat* emissive,
				GLfloat shininess)
{
  // see if we've already got an identical material
  for (Rep* scan = head; scan; scan = scan->next) {
    if (shininess != scan->shininess)
      continue;

    const GLfloat* c1 = specular;
    const GLfloat* c2 = scan->specular;
    if (c1[0] != c2[0] || c1[1] != c2[1] || c1[2] != c2[2])
      continue;

    c1 = emissive;
    c2 = scan->emissive;
    if (c1[0] != c2[0] || c1[1] != c2[1] || c1[2] != c2[2])
      continue;

    scan->ref();
    return scan;
  }

  // nope, make a new one
  return new Rep(specular, emissive, shininess);
}

OpenGLMaterial::Rep::Rep(const GLfloat* _specular,
			 const GLfloat* _emissive,
			 GLfloat _shininess)
			 : refCount(1), shininess(_shininess)
{
  list = INVALID_GL_LIST_ID;

  prev = NULL;
  next = head;
  head = this;
  if (next) next->prev = this;

  specular[0] = _specular[0];
  specular[1] = _specular[1];
  specular[2] = _specular[2];
  specular[3] = 1.0f;
  emissive[0] = _emissive[0];
  emissive[1] = _emissive[1];
  emissive[2] = _emissive[2];
  emissive[3] = 1.0f;

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
      if (RENDERER.useQuality() > 0) {
	if  ((specular[0] > 0.0f) ||
	     (specular[1] > 0.0f) ||
	     (specular[2] > 0.0f)) {
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
  GLuint& list = ((Rep*)self)->list;
  if (list != INVALID_GL_LIST_ID) {
    glDeleteLists(list, 1);
    list = INVALID_GL_LIST_ID;
  }
  return;
}


void OpenGLMaterial::Rep::initContext(void* UNUSED(self))
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

OpenGLMaterial::OpenGLMaterial(const GLfloat* specular,
				const GLfloat* emissive,
				GLfloat shininess)
{
  rep = Rep::getRep(specular, emissive, shininess);
}

OpenGLMaterial::OpenGLMaterial(const OpenGLMaterial& m)
{
  rep = m.rep;
  if (rep) rep->ref();
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
