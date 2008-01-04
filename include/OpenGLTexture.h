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

#include "common.h"
#include <string>
#include "bzfgl.h"

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
			Max = LinearMipmapLinear,
			Default = Max
    };

			OpenGLTexture(int width, int height,
					const GLvoid* pixels,
					Filter maxFilter = Linear,
					bool repeat = true,
					int internalFormat = 0);
			~OpenGLTexture();
    bool		hasAlpha() const;

    void		execute();

    float		getAspectRatio() const;
    int			getWidth() const;
    int			getHeight() const;

    Filter		getFilter();
    void		setFilter(Filter);

    bool		getColorAverages(float rgbaRaw[4],
					 bool factorAlpha) const;

    void		freeContext();
    void		initContext();


    static int		getFilterCount();
    static const char*	getFilterName(Filter id);
    static const char**	getFilterNames();

    static Filter	getMaxFilter();
    static void		setMaxFilter(Filter);

  private:
			OpenGLTexture(const OpenGLTexture&);
    OpenGLTexture&	operator=(const OpenGLTexture&);

    bool		operator==(const OpenGLTexture&) const;
    bool		operator!=(const OpenGLTexture&) const;
    bool		operator<(const OpenGLTexture&) const;
    int			getBestFormat( int width, int height,
					const GLvoid* pixels);
    void		bind();
    bool		setupImage(const GLubyte* pixels);

    void* operator new(size_t s) { return ::operator new(s);}
    void  operator delete(void *p) {::operator delete(p);}

    bool	alpha;
    const int	width;
    const int	height;
    GLint	scaledWidth;
    GLint	scaledHeight;
    GLubyte*	image;
    GLubyte*	imageMemory;
    bool	repeat;
    int		internalFormat;
    GLuint	list;
    Filter	filter;

    static Filter	maxFilter;

    static const int	filterCount;
    static const char*	configFilterNames[];

    static const GLenum	minifyFilter[];
    static const GLenum	magnifyFilter[];

    static void		static_freeContext(void *that);
    static void		static_initContext(void *that);

    friend class TextureManager;
};

//
// OpenGLTexture
//

inline bool		OpenGLTexture::hasAlpha() const
{
  return alpha;
}

inline int		OpenGLTexture::getWidth() const
{
  return width;
}
inline int		OpenGLTexture::getHeight() const
{
  return height;
}

#endif // BZF_OPENGL_TEXTURE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
