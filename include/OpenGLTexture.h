/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* OpenGLTexture:
 *	Encapsulates an OpenGL texture.
 *
 * Data must be in GL_RGBA format with type GL_UNSIGNED_BYTE.
 * If, for all pixels, the RGB components are equal, then
 * the data will be transformed to GL_LUMINANCE_ALPHA.  If,
 * for all pixels, the alpha component is opaque, then the
 * data will be transformed to GL_LUMINANCE or GL_RGB.
 * hasAlpha() will return true iff the alpha component was
 * kept.
 *
 * OpenGLTexture reference counts so copying or assigning a
 * texture keeps the same display list until the object is
 * changed.
 *
 * A default constructed texture is invalid.  The only way to
 * make such an object valid is to assign a valid texture to it.
 * Drawing an invalid texture has no effect.
 *
 * operator==() returns true iff the two objects refer to the
 * same display list.  operator!=() returns true iff the two
 * objects do not refer to the same display list.  Textures
 * that don't refer to the same display list but have exactly
 * the same texture map compare not-equal.
 *
 * <, <=, >, >= define an arbitrary ordering of textures.
 */

#ifndef	BZF_OPENGL_TEXTURE_H
#define	BZF_OPENGL_TEXTURE_H

#include "bzfgl.h"
#include "common.h"

class OpenGLTexture {
  public:
    enum Filter {
			Off,
			Nearest,
			Linear,
			NearestMipmapNearest,
			LinearMipmapNearest,
			NearestMipmapLinear,
			LinearMipmapLinear,
			Max = LinearMipmapLinear
    };

			OpenGLTexture();
			OpenGLTexture(int width, int height,
					const GLvoid* pixels,
					Filter maxFilter = Linear,
					bool repeat = true,
					int internalFormat = 0);
			OpenGLTexture(const OpenGLTexture&);
			~OpenGLTexture();
    OpenGLTexture&	operator=(const OpenGLTexture&);

    bool		operator==(const OpenGLTexture&) const;
    bool		operator!=(const OpenGLTexture&) const;
    bool		operator<(const OpenGLTexture&) const;
    bool		isValid() const;
    bool		hasAlpha() const;
    GLuint		getList() const;

    void		execute() const;

    static Filter	getFilter();
    static void		setFilter(Filter);

  private:
    class Rep {
      public:
			Rep(int width, int height,
					const GLvoid* pixels,
					int maxFilter,
					bool repeat,
					int internalFormat);
			~Rep();
	void		setFilter(int filter);

      public:
	int		refCount;
	GLuint		list;
	bool		alpha;
	Rep*		next;
	static Rep*	first;

      private:
	void		doInitContext();
	static void	initContext(void*);
	static int	getBestFormat(int width, int height,
					const GLvoid* pixels);

      private:
	const int	width;
	const int	height;
	GLubyte*	image;
	bool		repeat;
	int		internalFormat;

	int			maxFilter;
	static const GLenum	minifyFilter[];
	static const GLenum	magnifyFilter[];
    };

    void		ref();
    bool		unref();
    static void		bind(Rep*);

  private:
    Rep*		rep;
    static Filter	filter;
    static Rep*		lastRep;
};

//
// OpenGLTexture
//

inline bool		OpenGLTexture::isValid() const
{
  return rep != NULL;
}

inline bool		OpenGLTexture::hasAlpha() const
{
  return rep != NULL && rep->alpha;
}

#endif // BZF_OPENGL_TEXTURE_H
// ex: shiftwidth=2 tabstop=8
