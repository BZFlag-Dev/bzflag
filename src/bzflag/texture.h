/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * texture loading
 */

#ifndef	BZF_TEXTURE_H
#define	BZF_TEXTURE_H

#include "common.h"
#include "BzfString.h"
#include "OpenGLTexture.h"
#include "OpenGLTexFont.h"

unsigned char*		getTextureImage(const BzfString& file,
				int& width, int& height, int& depth);
unsigned char*		getTextImage(const BzfString& file,
				int& width, int& height);
OpenGLTexture		getTexture(const BzfString& file,
				int* width = NULL, int* height = NULL,
				OpenGLTexture::Filter = OpenGLTexture::Max,
				boolean repeat = True,
				boolean noError = False);
OpenGLTexture		getTexture(const BzfString& file,
				OpenGLTexture::Filter = OpenGLTexture::Max,
				boolean repeat = True,
				boolean noError = False);

class TextureFont {
  public:
    enum Font {
			TimesBold,
			TimesBoldItalic,
			HelveticaBold,
			HelveticaBoldItalic,
			Fixed,
			FixedBold
    };

    static OpenGLTexFont getTextureFont(Font, boolean required = False);

  private:
    static OpenGLTexFont*	font[];
    static const char*		fontFileName[];
};

#endif /* BZF_TEXTURE_H */
