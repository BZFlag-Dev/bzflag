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

#include "OpenGLTexFont.h"
#include "MediaFile.h"
#include "bzfgl.h"
#include <string.h>
#include <math.h>
#include <map>

typedef std::map<std::string, OpenGLTexFont::Rep*> TexFontMap;
static TexFontMap		texFontMap;
static bool				anyFontLoaded = false;

//
// OpenGLTexFont::Rep
//

OpenGLTexFont::Rep::Rep() : refCount(1),
								ascent(0.0f), descent(0.0f),
								height(0.0f), spacing(0.0f),
								data(NULL), width(0)
{
	// do nothing
}

OpenGLTexFont::Rep::Rep(int dx, int dy, const unsigned char* pixels) :
								refCount(1),
								ascent(0.0f), descent(0.0f),
								height(0.0f), spacing(0.0f),
								data(NULL), width(dx)
{
	// pick a good format.  we want to pack it as much as possible.
	static const int format = GL_LUMINANCE4_ALPHA4; // GL_INTENSITY4;

	// copy pixel data
	data = new unsigned char[dx * (dy - 28)];
	for (int j = 0; j < dy - 28; j++)
		for (int i = 0; i < dx; i++)
			data[i + j * dx] = pixels[4 * (i + (j + 28) * dx)];

	// make texture
	texture = OpenGLTexture(dx, dy - 28, pixels + 4 * 28 * dx,
				OpenGLTexture::Linear, true, format);

	// font constants
	const int tmpAscent = getValue(pixels, dx, 0, 0);
	const int tmpDescent = getValue(pixels, dx, 0, 1);
	height = (float)(tmpAscent + tmpDescent);
	spacing = getValue(pixels, dx, 0, 2) / height;
	ascent = (float)tmpAscent / height;
	descent = (float)tmpDescent / height;

	// useful constants
	const float iw = 1.0f / (float)dx;
	const float ih = 1.0f / (float)(dy - 28);
	const float ifh = 1.0f / height;

	// get glyphs
	int i;
	for (i = 0; i < 95; i++) {
		glyph[i].du = iw * (float)(getValue(pixels, dx, i+1, 0));
		glyph[i].dv = ih * (float)(getValue(pixels, dx, i+1, 1));
		glyph[i].u = iw * (float)getValue(pixels, dx, i+1, 2);
		glyph[i].v = ih * (float)getValue(pixels, dx, i+1, 3);
		glyph[i].width = ifh * (float)(getValue(pixels, dx, i+1, 0));
		glyph[i].height = ifh * (float)(getValue(pixels, dx, i+1, 1));
		glyph[i].advance = ifh * (float)getValue(pixels, dx, i+1, 4);
		glyph[i].su = ifh * (float)getValue(pixels, dx, i+1, 5);
		glyph[i].sv = ifh * (float)getValue(pixels, dx, i+1, 6);

		glyph[i].dx = getValue(pixels, dx, i+1, 0);
		glyph[i].dy = getValue(pixels, dx, i+1, 1);
		glyph[i].x = getValue(pixels, dx, i+1, 2);
		glyph[i].y = getValue(pixels, dx, i+1, 3);
		glyph[i].iAdvance = getValue(pixels, dx, i+1, 4);
		glyph[i].sx = getValue(pixels, dx, i+1, 5);
		glyph[i].sy = getValue(pixels, dx, i+1, 6);
	}

	OpenGLGStateBuilder builder(gstate);
	builder.setTexture(texture);
	builder.setBlending(GState::kSrcAlpha, GState::kOneMinusSrcAlpha);
	builder.setAlphaFunc(GState::kGEqual, 0.1f);
	gstate = builder.getState();
}

OpenGLTexFont::Rep::~Rep()
{
	delete[] data;
}

void					OpenGLTexFont::Rep::ref()
{
	refCount++;
}

void					OpenGLTexFont::Rep::unref()
{
	if (--refCount <= 0) delete this;
}

const unsigned char*		OpenGLTexFont::Rep::getRow(int row) const
{
	return data + width * row;
}

const OpenGLTexFont::Glyph* OpenGLTexFont::Rep::getGlyphs() const
{
	return glyph;
}

int						OpenGLTexFont::Rep::getValue(
								const unsigned char* data,
								int width, int index, int offset)
{
	if (index >= 64) {
		index -= 64;
		data += 14 * 4 * width;
	}
	data += 4 * 2 * offset * width;
	return (int)(short)(((unsigned short)data[4 * index] << 8) +
						(unsigned short)data[4 * index + 4 * width]);
}

//
// OpenGLTexFont::BitmapRep
//

OpenGLTexFont::BitmapRep* OpenGLTexFont::BitmapRep::first = NULL;
OpenGLGState			OpenGLTexFont::BitmapRep::gstate;

OpenGLTexFont::BitmapRep::BitmapRep(Rep* _rep, int _width, int _height) :
								rep(_rep), refCount(1),
								width(_width), height(_height)
{
	// hold a reference to the texture rep
	rep->ref();

	// make my glyphs
	glyph = new Glyph[95];
	for (int i = 0; i < 95; i++)
		createGlyph(i);

	// add me to list
	next = first;
	first = this;
}

OpenGLTexFont::BitmapRep::~BitmapRep()
{
	// release glyphs
	for (int i = 0; i < 95; i++)
		delete[] glyph[i].origBitmap;
	delete[] glyph;

	// release texture rep
	rep->unref();

	// remove me from list
	if (this == first) {
		first = next;
	}
	else {
		for (BitmapRep* scan = first; scan; scan = scan->next)
			if (scan->next == this) {
				scan->next = next;
				break;
			}
	}
}

OpenGLTexFont::BitmapRep* OpenGLTexFont::BitmapRep::getBitmapRep(
								Rep* rep, int width, int height)
{
	BitmapRep* newRep = getBitmapRepIfExists(rep, width, height);
	if (!newRep) newRep = new BitmapRep(rep, width, height);
	return newRep;
}

OpenGLTexFont::BitmapRep* OpenGLTexFont::BitmapRep::getBitmapRepIfExists(
								Rep* rep, int width, int height)
{
	for (BitmapRep* scan = first; scan; scan = scan->next)
		if (scan->rep == rep && scan->width == width && scan->height == height) {
			scan->ref();
			return scan;
		}
	return NULL;
}

void					OpenGLTexFont::BitmapRep::ref()
{
	refCount++;
}

void					OpenGLTexFont::BitmapRep::unref()
{
	if (--refCount == 0) delete this;
}

void					OpenGLTexFont::BitmapRep::draw(
								const char* string, int length,
								float x, float y, float z)
{
	float dx = 0.0f;
	gstate.setState();
	glRasterPos3f(x, y, z);
	for (int i = 0; i < length; i++) {
		const unsigned int c = (unsigned int)string[i];
		if (c >= 32 && c < 127) {
			const Glyph& g = glyph[c - 32];
			glBitmap(g.width, g.height, g.xorig, g.yorig, g.xmove, g.ymove, g.bitmap);
			dx += g.xmove;
		}
		else if (c == '\n') {
			glBitmap(0, 0, 0, 0, -dx, height, glyph[0].bitmap);
			dx = 0.0f;
		}
	}
}

void					OpenGLTexFont::BitmapRep::createGlyph(int index)
{
	const OpenGLTexFont::Glyph& srcGlyph = rep->getGlyphs()[index];
	Glyph& dstGlyph = glyph[index];

	// compute scaling factors
	const float xScale = (float)width / rep->height;
	const float yScale = (float)height / rep->height;

	// compute size of resulting bitmap
	int dx = (int)(xScale * srcGlyph.dx);
	if (dx < 1) dx = 1;
	int dy = (int)(yScale * srcGlyph.dy);
	if (dy < 1) dy = 1;
	const int bytesPerRow = ((dx + 31) / 8) & ~3;

	// set glyph info
	dstGlyph.width = dx;
	dstGlyph.height = dy;
	dstGlyph.xorig = -xScale * srcGlyph.sx;
	dstGlyph.yorig = yScale * srcGlyph.sy;
	dstGlyph.xmove = xScale * srcGlyph.iAdvance;
	dstGlyph.ymove = 0;

	// allocate space for bitmap
	dstGlyph.origBitmap = new unsigned char[dy * bytesPerRow + 4];
	dstGlyph.bitmap = (unsigned char*)(((unsigned long)dstGlyph.origBitmap & ~3) + 4);

	// copy bitmap with scaling
	const int IT = 32;	// intensity threshold
	const float xInvScale = (float)srcGlyph.dx / (float)dx;
	const float yInvScale = (float)srcGlyph.dy / (float)dy;
	for (int j = 0; j < dy; j++) {
		const int srcRow = srcGlyph.y + (int)(yInvScale * (float)j + 0.0f);
		const unsigned char* srcData = rep->getRow(srcRow) + srcGlyph.x;
		unsigned char* dstData = dstGlyph.bitmap + j * bytesPerRow;

		int b;
		for (b = 0; b < dx - 7; b += 8) {
			unsigned char data = 0;

			if (srcData[(int)(xInvScale * ((float)b + 0.0f))] >= IT) data |= 0x80u;
			if (srcData[(int)(xInvScale * ((float)b + 1.0f))] >= IT) data |= 0x40u;
			if (srcData[(int)(xInvScale * ((float)b + 2.0f))] >= IT) data |= 0x20u;
			if (srcData[(int)(xInvScale * ((float)b + 3.0f))] >= IT) data |= 0x10u;
			if (srcData[(int)(xInvScale * ((float)b + 4.0f))] >= IT) data |= 0x08u;
			if (srcData[(int)(xInvScale * ((float)b + 5.0f))] >= IT) data |= 0x04u;
			if (srcData[(int)(xInvScale * ((float)b + 6.0f))] >= IT) data |= 0x02u;
			if (srcData[(int)(xInvScale * ((float)b + 7.0f))] >= IT) data |= 0x01u;

			*dstData++ = data;
		}

		unsigned char data = 0;
		switch (dx - b) {
			case 7:
				if (srcData[(int)(xInvScale * ((float)b + 6.0f))] >= IT) data |= 0x02u;
			case 6:
				if (srcData[(int)(xInvScale * ((float)b + 5.0f))] >= IT) data |= 0x04u;
			case 5:
				if (srcData[(int)(xInvScale * ((float)b + 4.0f))] >= IT) data |= 0x08u;
			case 4:
				if (srcData[(int)(xInvScale * ((float)b + 3.0f))] >= IT) data |= 0x10u;
			case 3:
				if (srcData[(int)(xInvScale * ((float)b + 2.0f))] >= IT) data |= 0x20u;
			case 2:
				if (srcData[(int)(xInvScale * ((float)b + 1.0f))] >= IT) data |= 0x40u;
			case 1:
				if (srcData[(int)(xInvScale * ((float)b + 0.0f))] >= IT) data |= 0x80u;
				*dstData++ = data;
		}
	}
}

//
// OpenGLTexFont
//

OpenGLTexFont::OpenGLTexFont() : bitmapRep(NULL), width(1.0f), height(1.0f)
{
	rep = new Rep;
}

OpenGLTexFont::OpenGLTexFont(const std::string& name) :
								bitmapRep(NULL), width(1.0f), height(1.0f)
{
	TexFontMap::iterator index = texFontMap.find(name);
	if (index == texFontMap.end()) {
		// no such name
		rep = new Rep;
	}
	else {
		// found it
		rep = index->second;
		rep->ref();
	}
}

OpenGLTexFont::OpenGLTexFont(int dx, int dy, const unsigned char* pixels) :
								bitmapRep(NULL), width(1.0f), height(1.0f)
{
	rep = new Rep(dx, dy, pixels);
}

OpenGLTexFont::OpenGLTexFont(const OpenGLTexFont& f)
{
	rep = f.rep;
	rep->ref();
	bitmapRep = f.bitmapRep;
	if (bitmapRep) bitmapRep->ref();
	width = f.width;
	height = f.height;
}

OpenGLTexFont::~OpenGLTexFont()
{
	if (bitmapRep) bitmapRep->unref();
	rep->unref();
}

OpenGLTexFont&			OpenGLTexFont::operator=(const OpenGLTexFont& f)
{
	if (this != &f) {
		if (bitmapRep) bitmapRep->unref();
		bitmapRep = f.bitmapRep;
		if (bitmapRep) bitmapRep->ref();
		rep->unref();
		rep = f.rep;
		rep->ref();
		width = f.width;
		height = f.height;
	}
	return *this;
}

void					OpenGLTexFont::mapFont(
								const std::string& name,
								const std::string& filename)
{
	// load file and create rep
	int dx, dy;
	unsigned char* pixels = MediaFile::readImage(filename, &dx, &dy);
	OpenGLTexFont::Rep* rep;
	if (pixels != NULL) {
		rep = new Rep(dx, dy, pixels);
		anyFontLoaded = true;
	}
	else {
		rep = new Rep;
	}
	delete[] pixels;

	// save font
	texFontMap.erase(name);
	texFontMap.insert(std::make_pair(name, rep));
}

bool					OpenGLTexFont::isAnyMappedFontLoaded()
{
	return anyFontLoaded;
}

bool					OpenGLTexFont::isValid() const
{
	return rep->texture.isValid();
}

void					OpenGLTexFont::setSize(float _width, float _height)
{
	float wm = _width / rep->height;
	float hm = _height / rep->height;

/* turn this off for now
	// force small fonts at small size to integer multiples for clarity
	if (rep->height < 14.0f && (wm <= 2.0f || hm <= 2.0f)) {
		wm = floorf(wm);
		hm = floorf(hm);
		if (wm < 1.0f) wm = 1.0f;
		if (hm < 1.0f) hm = 1.0f;
	}
*/
	width = wm * rep->height;
	height = hm * rep->height;

	BitmapRep* newBitmapRep = BitmapRep::getBitmapRepIfExists(rep,
												(int)width, (int)height);
	if (bitmapRep != newBitmapRep && bitmapRep) bitmapRep->unref();
	bitmapRep = newBitmapRep;
}

float					OpenGLTexFont::getAscent() const
{
	return height * rep->ascent;
}

float					OpenGLTexFont::getDescent() const
{
	return height * rep->descent;
}

float					OpenGLTexFont::getWidth() const
{
	return width;
}

float					OpenGLTexFont::getHeight() const
{
	return height;
}

float					OpenGLTexFont::getWidth(const std::string& s) const
{
	return getWidth(s.c_str(), s.size());
}

float					OpenGLTexFont::getWidth(const char* s) const
{
	return getWidth(s, strlen(s));
}

float					OpenGLTexFont::getWidth(const char* s, int length) const
{
	float dx    = 0.0f;
	float dxMax = 0.0f;
	for (int i = 0; i < length; i++) {
		if (s[i] >= 32 && s[i] < 127) {
			dx += rep->glyph[s[i] - 32].advance;
		}
		else if (s[i] == '\n') {
			if (dx > dxMax)
				dxMax = dx;
			dx = 0.0f;
		}
	}
	if (dx > dxMax)
		dxMax = dx;
	return width * dxMax;
}

int						OpenGLTexFont::getLengthInWidth(
								float w,
								const std::string& s) const
{
	return getLengthInWidth(w, s.c_str(), s.size());
}

int						OpenGLTexFont::getLengthInWidth(
								float w,
								const char* s) const
{
	return getLengthInWidth(w, s, strlen(s));
}

int						OpenGLTexFont::getLengthInWidth(
								float w,
								const char* s,
								int length) const
{
	w /= width;
	float dx = 0.0f;
	for (int i = 0; i < length; i++) {
		if (s[i] >= 32 && s[i] < 127) {
			dx += rep->glyph[s[i] - 32].advance;
			if (dx > w)
				return i;
		}
		else if (s[i] == '\n') {
			return i;
		}
	}
	return length;
}

float					OpenGLTexFont::getSpacing() const
{
	return height * rep->spacing;
}

float					OpenGLTexFont::getBaselineFromCenter() const
{
	// offset of baseline from centerline
	return 0.5f * (getDescent() - getAscent());
}

void					OpenGLTexFont::draw(const std::string& s,
								float x, float y, float z) const
{
	draw(s.c_str(), s.size(), x, y, z);
}

void					OpenGLTexFont::draw(const char* s,
								float x, float y, float z) const
{
	draw(s, strlen(s), x, y, z);
}

void					OpenGLTexFont::draw(const char* string, int length,
								float x, float y, float z) const
{
	if (OpenGLTexture::getFilter() == OpenGLTexture::Off) {
		if (!bitmapRep)
			((OpenGLTexFont*)this)->bitmapRep =
						BitmapRep::getBitmapRep(rep, (int)width, (int)height);
		if (bitmapRep) {
			bitmapRep->draw(string, length, x, y, z);
			return;
		}
	}

	float x0 = x;
	rep->gstate.setState();
	glBegin(GL_QUADS);
	for (int i = 0; i < length; i++) {
		const unsigned int c = (unsigned int)string[i];
		if (c >= 32 && c < 127) {
			const Glyph& g = rep->glyph[c - 32];
			const float w = width * g.width;
			const float h = height * g.height;
			const float dx = width * g.su;
			const float dy = -height * g.sv;
			const float x0 = floorf(x + 0.5f + dx);
			const float y0 = floorf(y + 0.5f + dy);
			glTexCoord2f(g.u, g.v);
			glVertex3f(x0, y0, z);
			glTexCoord2f(g.u + g.du, g.v);
			glVertex3f(x0 + w, y0, z);
			glTexCoord2f(g.u + g.du, g.v + g.dv);
			glVertex3f(x0 + w, y0 + h, z);
			glTexCoord2f(g.u, g.v + g.dv);
			glVertex3f(x0, y0 + h, z);

			x += width * g.advance;
		}
		else if (c == '\n') {
			x  = x0;
			y -= height * rep->spacing;
		}
	}
	glEnd();
}
