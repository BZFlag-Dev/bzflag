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

/*
 * texture loading
 */

#ifndef	BZF_TEXTURE_H
#define	BZF_TEXTURE_H

#include <string>
#include "common.h"
#include "OpenGLTexture.h"
#include "OpenGLTexFont.h"

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

    static OpenGLTexFont getTextureFont(Font, bool required = false);

// sun's compiler is broken: sizeof(fontFileName) fails unless
// fontFileName is public.
#if !defined(sun)
  private:
#endif
    static OpenGLTexFont*	font[];
    static const char*		fontFileName[];
};

#endif /* BZF_TEXTURE_H */

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

