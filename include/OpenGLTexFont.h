/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
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
 * Font implemented using an OpenGL texture map
 */

#ifndef BZF_OPENGLTEXFONT_H
#define BZF_OPENGLTEXFONT_H

#include "common.h"
#include "OpenGLTexture.h"
#include "OpenGLGState.h"

class BzfString;

class OpenGLTexFont {
public:
	OpenGLTexFont();
	OpenGLTexFont(const BzfString& name);
	OpenGLTexFont(int width, int height, const unsigned char* pixels);
	OpenGLTexFont(const OpenGLTexFont&);
	~OpenGLTexFont();
	OpenGLTexFont&		operator=(const OpenGLTexFont&);

	static void			mapFont(const BzfString& name,
							const BzfString& filename);
	static bool			isAnyMappedFontLoaded();

	void				setSize(float width, float height);

	bool				isValid() const;
	float				getAscent() const;
	float				getDescent() const;
	float				getWidth() const;
	float				getHeight() const;
	float				getWidth(const BzfString&) const;
	float				getWidth(const char* string) const;
	float				getWidth(const char* string, int length) const;
	int					getLengthInWidth(float w, const BzfString&) const;
	int					getLengthInWidth(float w, const char* string) const;
	int					getLengthInWidth(float w,
							const char* string, int length) const;
	float				getSpacing() const;
	float				getBaselineFromCenter() const;
	void				draw(const BzfString&,
							float x, float y, float z = 0.0f) const;
	void				draw(const char*,
							float x, float y, float z = 0.0f) const;
	void				draw(const char* string, int length,
							float x, float y, float z = 0.0f) const;

	class Glyph {
	public:
		float			u, v;
		float			du, dv;
		float			su, sv;
		float			width, height;
		float			advance;

		int				x, y;
		int				dx, dy;
		int				sx, sy;
		int				iAdvance;
	};

	class Rep {
	public:
		Rep();
		Rep(int width, int height, const unsigned char* pixels);
		~Rep();
		void			ref();
		void			unref();

		const unsigned char*	getRow(int row) const;
		const Glyph*	getGlyphs() const;

		private:
		static int		getValue(const unsigned char* data,
							int width, int index, int offset);

	public:
		int				refCount;
		OpenGLTexture	texture;
		OpenGLGState	gstate;
		float			ascent;
		float			descent;
		float			height;
		float			spacing;
		Glyph			glyph[96];
		unsigned char*	data;
		int				width;
	};

	class BitmapRep {
	public:
		static BitmapRep* getBitmapRep(Rep*, int width, int height);
		static BitmapRep* getBitmapRepIfExists(Rep*, int width, int height);
		void			ref();
		void			unref();

		void			draw(const char*, int length,
							float x, float y, float z);

#if !defined(__linux__)		// shut off a brain dead warning in gcc/egcs
	private:
#endif
		BitmapRep(Rep*, int width, int height);
		~BitmapRep();

    private:
		BitmapRep(const BitmapRep&);
		BitmapRep&		operator=(const BitmapRep&);

		void			createGlyph(int index);

		class Glyph {
		public:
		    int			width;
		    int			height;
		    float		xorig;
		    float		yorig;
		    float		xmove;
		    float		ymove;
		    unsigned char*	bitmap;
		    unsigned char*	origBitmap;
		};

      private:
		Rep*			rep;
		int				refCount;
		int				width;
		int				height;
		Glyph*			glyph;
		BitmapRep*		next;
		static BitmapRep*	first;
		static OpenGLGState	gstate;
    };

private:
	Rep*				rep;
	BitmapRep*			bitmapRep;
	float				width, height;
};

#endif // BZF_OPENGLTEXFONT_H
