/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
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
 * Font implemented using an OpenGL texture map
 */

#ifndef	BZF_OPENGLTEXFONT_H
#define	BZF_OPENGLTEXFONT_H

#include "common.h"
#include "OpenGLTexture.h"
#include "OpenGLGState.h"
#include <string>

#define ESC_CHAR	((char) 0x1B)
#define FONT_CODES	12

typedef enum ColorCodes {
  // the first 5 codes line up with the TeamColor enum from global.h
  RogueColor		= 0,	// team (yellow)
  RedColor		= 1,	// team
  GreenColor		= 2,	// team
  BlueColor		= 3,	// team
  PurpleColor		= 4,	// team

  WhiteColor		= 5,
  GreyColor		= 6,
  CyanColor		= 7,

  ResetColor		= 8,
  FinalResetColor       = 11,
  BlinkColor		= 9,
  UnderlineColor	= 10,

  YellowColor		= 0,
  DefaultColor		= 6	// default to grey
};

extern const char * ColorStrings[FONT_CODES];

class OpenGLTexFont {
  public:
			OpenGLTexFont();
			OpenGLTexFont(int width, int height,
				const unsigned char* pixels);
			OpenGLTexFont(const OpenGLTexFont&);
			~OpenGLTexFont();
    OpenGLTexFont&	operator=(const OpenGLTexFont&);

    void		setSize(float width, float height);

    bool		isValid() const;
    float		getAscent() const;
    float		getDescent() const;
    float		getWidth() const;
    float		getHeight() const;
    float		getWidth(const std::string&) const;
    float		getWidth(const char* string) const;
    float		getWidth(const char* string, int length) const;
    float		getSpacing() const;
    float		getBaselineFromCenter() const;
    void		draw(const std::string&,
				float x, float y, float z = 0.0f) const;
    void		draw(const char*,
				float x, float y, float z = 0.0f) const;
    void		draw(const char* string, int length,
				float x, float y, float z = 0.0f) const;

    /**
     * strips the ansi codes from an input string for
     * up to length characters.  the new length of the
     * string is returned.  the input string is
     * destructively modified (and clipped with a zero
     * char).
     */
    static int		stripAnsiCodes (char* string, int length);

    /**
     * returns the length of a string. ansi codes that
     * may be present are taken into account.  only 
     * printable characters are included in the string
     * length (i.e. the ansi code chars are not counted)
     */
    static int		rawStrlen (const char* string, int length);

    static void		setUnderlineColor (int code);
    static std::string	getUnderlineColor();

    class Glyph {
      public:
	float		u, v;
	float		du, dv;
	float		su, sv;
	float		width, height;
	float		advance;

	int		x, y;
	int		dx, dy;
	int		sx, sy;
	int		iAdvance;
    };

    class Rep {
      public:
			Rep();
			Rep(int width, int height, const unsigned char* pixels);
			~Rep();
	void		ref();
	void		unref();

	const unsigned char*	getRow(int row) const;
	const Glyph*		getGlyphs() const;

      private:
	static int	getValue(const unsigned char* data,
				int width, int index, int offset);

      public:
	int		refCount;
	OpenGLTexture	texture;
	OpenGLGState	gstate;
	float		ascent;
	float		descent;
	float		height;
	float		spacing;
	Glyph		glyph[96];
	unsigned char*	data;
	int		width;
    };

    class BitmapRep {
      public:
	static BitmapRep* getBitmapRep(Rep*, int width, int height);
	static BitmapRep* getBitmapRepIfExists(Rep*, int width, int height);
	static void	setState();
	void		ref();
	void		unref();

	void		draw(const char*, int length,
				float x, float y, float z);
	float		drawChar(const char);

#if !defined(__linux__)	// shut off a brain dead warning in gcc/egcs
      private:
#endif
			BitmapRep(Rep*, int width, int height);
			~BitmapRep();
      private:
			BitmapRep(const BitmapRep&);
	BitmapRep&	operator=(const BitmapRep&);

	void		createGlyph(int index);

	class Glyph {
	  public:
	    GLsizei	width;
	    GLsizei	height;
	    GLfloat	xorig;
	    GLfloat	yorig;
	    GLfloat	xmove;
	    GLfloat	ymove;
	    GLubyte*	bitmap;
	    GLubyte*	origBitmap;
	};

      private:
	Rep*		rep;
	int		refCount;
	int		width;
	int		height;
	Glyph*		glyph;
	BitmapRep*	next;
	static BitmapRep*	first;
	static OpenGLGState	gstate;
    };

  private:
    Rep*		rep;
    BitmapRep*		bitmapRep;
    float		width, height;
    static int		underlineColor;
};

#endif // BZF_OPENGLTEXFONT_H
// ex: shiftwidth=2 tabstop=8
