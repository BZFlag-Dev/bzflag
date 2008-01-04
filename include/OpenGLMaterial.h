/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* OpenGLMaterial:
 *	Encapsulates an OpenGL material.
 *
 * All materials are for front and back faces.
 * OpenGLMaterial reference counts so copying or assigning a
 * material keeps the same display list until the object is
 * changed.
 *
 * operator==() returns true iff the two objects refer to the
 * same display list.  operator!=() returns true iff the two
 * objects do not refer to the same display list.  Materials
 * that don't refer to the same display list but have exactly
 * the same colors and shininess compare not-equal.
 *
 * <, <=, >, >= define an arbitrary ordering of materials.
 */

#ifndef	BZF_OPENGL_MATERIAL_H
#define	BZF_OPENGL_MATERIAL_H

#include "common.h"
#include "bzfgl.h"

class OpenGLMaterial {
  public:
			OpenGLMaterial();
			OpenGLMaterial(const GLfloat* specularRGB,
					const GLfloat* emissiveRGB,
					GLfloat shininess = 0.0f);
			OpenGLMaterial(const OpenGLMaterial&);
			~OpenGLMaterial();
    OpenGLMaterial&	operator=(const OpenGLMaterial&);

    bool		operator==(const OpenGLMaterial&) const;
    bool		operator!=(const OpenGLMaterial&) const;
    bool		operator<(const OpenGLMaterial&) const;

    const GLfloat*	getSpecularColor() const;
    const GLfloat*	getEmissiveColor() const;
    GLfloat		getShininess() const;

    void		setQuality(bool highQuality);

    bool		isValid() const;
    void		execute() const;

  private:
    class Rep {
      public:
			~Rep();
	void		ref();
	void		unref();
	void		execute();
	static Rep*	getRep(const GLfloat* specular,
				const GLfloat* emissive,
				GLfloat shininess);
      private:
			Rep(const GLfloat* specular,
				const GLfloat* emissive,
				GLfloat shininess);
	static void	freeContext(void*);
	static void	initContext(void*);
      public:
	int		refCount;
	Rep*		prev;
	Rep*		next;
	GLuint		list;
	GLfloat		specular[4];
	GLfloat		emissive[4];
	GLfloat		shininess;
	static Rep*	head;
        bool		highQuality;
    };
    Rep*		rep;
};

//
// OpenGLMaterial
//

inline const GLfloat*	OpenGLMaterial::getSpecularColor() const
{
  return rep->specular;
}

inline const GLfloat*	OpenGLMaterial::getEmissiveColor() const
{
  return rep->emissive;
}

inline GLfloat		OpenGLMaterial::getShininess() const
{
  return rep->shininess;
}

inline void		OpenGLMaterial::setQuality(bool highQuality)
{
  rep->highQuality = highQuality;
}


#endif // BZF_OPENGL_MATERIAL_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
