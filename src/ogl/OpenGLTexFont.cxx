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

#ifdef _MSC_VER
#pragma warning( 4:4786)
#endif

#include <assert.h>
#include <iostream>
#include <string>
#include <string.h>
#include <math.h>
#include "TimeKeeper.h"
#include "OpenGLTexFont.h"


// ANSI (ISO 6429) colors codes (these are all used in BRIGHT mode)

#define ANSI_STR_RESET		"\033[0;1m"	// reset & bright
#define ANSI_STR_RESET_FINAL	"\033[0m"	// only reset
#define ANSI_STR_BRIGHT		"\033[1m"	// unimplemented
#define ANSI_STR_DIM		"\033[2m"	// unimplemented
#define ANSI_STR_UNDERLINE	"\033[4m"
#define ANSI_STR_BLINK		"\033[5m"
#define ANSI_STR_REVERSE	"\033[7m"	// unimplemented

#define ANSI_STR_FG_BLACK	"\033[30m"	// grey
#define ANSI_STR_FG_RED		"\033[31m"
#define ANSI_STR_FG_GREEN	"\033[32m"
#define ANSI_STR_FG_YELLOW	"\033[33m"
#define ANSI_STR_FG_BLUE	"\033[34m"
#define ANSI_STR_FG_MAGENTA	"\033[35m"	// purple
#define ANSI_STR_FG_CYAN	"\033[36m"
#define ANSI_STR_FG_WHITE	"\033[37m"


// These enum values have to line up with those in OpenGLTexFont.h

const char * ColorStrings[FONT_CODES] = {
  ANSI_STR_FG_YELLOW,   // 0  Rogue     (yellow)
  ANSI_STR_FG_RED,      // 1  Red
  ANSI_STR_FG_GREEN,    // 2  Green
  ANSI_STR_FG_BLUE,     // 3  Blue
  ANSI_STR_FG_MAGENTA,  // 4  Purple
  ANSI_STR_FG_WHITE,    // 5  White
  ANSI_STR_FG_BLACK,    // 6  Grey      (bright black is grey)
  ANSI_STR_FG_CYAN,     // 7  Cyan
  ANSI_STR_RESET,       // 8  Reset
  ANSI_STR_BLINK,       // 9  Blink
  ANSI_STR_UNDERLINE,   // 10 Underline
  ANSI_STR_RESET_FINAL  // 11 Really reset (no brightness added)
};

// This maps the ANSI LSB code to bzflag color numbers

const int color_map [8] = {
  6,  // ANSI BLACK   (30)
  1,  // ANSI RED     (31)
  2,  // ANSI GREEN   (32)
  0,  // ANSI YELLOW  (33)
  3,  // ANSI BLUE    (34)
  4,  // ANSI MAGENTA (35)
  7,  // ANSI CYAN    (36)
  5   // ANSI WHITE   (37)
};


// Constant for blinking text

#define BLINK_DEPTH	(0.5f)
#define BLINK_RATE	(0.25f)



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
#if defined(GL_VERSION_1_1)
  static const int format = GL_LUMINANCE4_ALPHA4; // GL_INTENSITY4;
#elif defined(GL_EXT_texture)
  static const int format = (strstr((const char*)glGetString(GL_EXTENSIONS),
		"GL_EXT_texture") != NULL) ? GL_LUMINANCE4_ALPHA4_EXT :
		GL_LUMINANCE_ALPHA;
#else
  static const int format = GL_LUMINANCE_ALPHA;
#endif

  // copy pixel data
  data = new unsigned char[dx * (dy - 28)];
  for (int j = 0; j < dy - 28; j++)
    for (int i = 0; i < dx; i++)
      data[i + j * dx] = pixels[4 * (i + (j + 28) * dx)];

  // make texture
  TextureManager &tm = TextureManager::instance();

  char  temp[512];
  sprintf(temp,"%d",(int)this);

  texture = tm.newTexture(temp,dx, dy - 28, (unsigned char*)(pixels + 4 * 28 * dx),OpenGLTexture::Linear, true, format);

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
  builder.setBlending();
  builder.setAlphaFunc();
  builder.enableTextureReplace(false);
  gstate = builder.getState();
}

OpenGLTexFont::Rep::~Rep()
{
  delete[] data;
}

void			OpenGLTexFont::Rep::ref()
{
  refCount++;
}

void			OpenGLTexFont::Rep::unref()
{
  if (--refCount <= 0) delete this;
}

const unsigned char*	OpenGLTexFont::Rep::getRow(int row) const
{
  return data + width * row;
}

const OpenGLTexFont::Glyph* OpenGLTexFont::Rep::getGlyphs() const
{
  return glyph;
}

int			OpenGLTexFont::Rep::getValue(
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
OpenGLGState		OpenGLTexFont::BitmapRep::gstate;

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

void			OpenGLTexFont::BitmapRep::ref()
{
  refCount++;
}

void			OpenGLTexFont::BitmapRep::unref()
{
  if (--refCount == 0) delete this;
}

void			OpenGLTexFont::BitmapRep::draw(
				const char* string, int length,
				float x, float y, float z)
{
  unsigned int c;

  gstate.setState();
  glRasterPos3f(x, y, z);

  length = rawStrlen(string, length);

  for (int i = 0; i < length; i++) {
    c = (unsigned int)string[i];
    if (c >= 32 && c < 127) {
      const Glyph& g = glyph[c - 32];
      glBitmap(g.width, g.height, g.xorig, g.yorig, g.xmove, g.ymove, g.bitmap);
    }
  }
}

float OpenGLTexFont::BitmapRep::drawChar (char c)
{
  const Glyph& g = glyph[c - 32];
  glBitmap (g.width, g.height, g.xorig, g.yorig, g.xmove, g.ymove, g.bitmap);

  return g.xmove;
}

void OpenGLTexFont::BitmapRep::setState (void)
{
  gstate.setState();
  return;
}

void			OpenGLTexFont::BitmapRep::createGlyph(int index)
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
  dstGlyph.origBitmap = new GLubyte[dy * bytesPerRow + 4];
  dstGlyph.bitmap = (GLubyte*)(((unsigned long)dstGlyph.origBitmap & ~3) + 4);

  // copy bitmap with scaling
  const int IT = 32;	// intensity threshold
  const float xInvScale = (float)srcGlyph.dx / (float)dx;
  const float yInvScale = (float)srcGlyph.dy / (float)dy;
  for (int j = 0; j < dy; j++) {
    const int srcRow = srcGlyph.y + (int)(yInvScale * (float)j + 0.0f);
    const unsigned char* srcData = rep->getRow(srcRow) + srcGlyph.x;
    GLubyte* dstData = dstGlyph.bitmap + j * bytesPerRow;

    int b;
    for (b = 0; b < dx - 7; b += 8) {
      GLubyte data = 0;

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

    GLubyte data = 0;
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

int OpenGLTexFont::underlineColor = CyanColor;

OpenGLTexFont::OpenGLTexFont() : bitmapRep(NULL), width(1.0f), height(1.0f)
{
  const float color[3] = {1.0, 1.0, 1.0};

  rep = new Rep;

  for (unsigned int i=0; i < STORED_COLORS; i++) {
    setColor(i, color);
  }
}

OpenGLTexFont::OpenGLTexFont(int dx, int dy, const unsigned char* pixels) :
				bitmapRep(NULL), width(1.0f), height(1.0f)
{
  const float color[3] = {1.0, 1.0, 1.0};

  rep = new Rep(dx, dy, pixels);

  for (unsigned int i=0; i < STORED_COLORS; i++) {
    setColor(i, color);
  }
}

OpenGLTexFont::OpenGLTexFont(const OpenGLTexFont& f)
{
  rep = f.rep;
  rep->ref();
  bitmapRep = f.bitmapRep;
  if (bitmapRep) bitmapRep->ref();
  width = f.width;
  height = f.height;

  for (unsigned int i=0; i < STORED_COLORS; i++) {
    setColor(i, f.storedColor[i]);
  }
}

OpenGLTexFont::~OpenGLTexFont()
{
  if (bitmapRep) bitmapRep->unref();
  rep->unref();
}

OpenGLTexFont&		OpenGLTexFont::operator=(const OpenGLTexFont& f)
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
    for (unsigned int i=0; i < STORED_COLORS; i++) {
      setColor(i, f.storedColor[i]);
    }
  }
  return *this;
}

bool			OpenGLTexFont::isValid() const
{
  return rep->texture >=0;
}

void			OpenGLTexFont::setSize(float _width, float _height)
{
  float wm = _width / rep->height;
  float hm = _height / rep->height;
  if (rep->height < 14.0f && (wm <= 2.0f || hm <= 2.0f)) {
    wm = floorf(wm);
    hm = floorf(hm);
    if (wm < 1.0f) wm = 1.0f;
    if (hm < 1.0f) hm = 1.0f;
  }
  width = wm * rep->height;
  height = hm * rep->height;

  BitmapRep* newBitmapRep = BitmapRep::getBitmapRepIfExists(rep,
						(int)width, (int)height);
  if (bitmapRep != newBitmapRep && bitmapRep) bitmapRep->unref();
  bitmapRep = newBitmapRep;
}

void OpenGLTexFont::setColor(unsigned short int index, const float *color) {
  assert(index < STORED_COLORS);

  storedColor[index][0] = color[0];
  storedColor[index][1] = color[1];
  storedColor[index][2] = color[2];
}

float			OpenGLTexFont::getAscent() const
{
  return height * rep->ascent;
}

float			OpenGLTexFont::getDescent() const
{
  return height * rep->descent;
}

float			OpenGLTexFont::getWidth() const
{
  return width;
}

float			OpenGLTexFont::getHeight() const
{
  return height;
}

float			OpenGLTexFont::getWidth(const std::string& str) const
{
  float dx = 0.0f;

  const char* s = str.c_str();

  int length = rawStrlen (s, str.size());

  for (int i = 0; i < length; i++) {
    if ((s[i] >= 32) && (s[i] < 127))
      dx += rep->glyph[s[i] - 32].advance;
  }

  return width * dx;
}

float			OpenGLTexFont::getSpacing() const
{
  return height * rep->spacing;
}

float			OpenGLTexFont::getBaselineFromCenter() const
{
  // offset of baseline from centerline
  return 0.5f * (getDescent() - getAscent());
}

void			OpenGLTexFont::draw(const std::string& s,
				float x, float y, float z) const
{
  draw(s.c_str(), s.length(), x, y, z);
}

void			OpenGLTexFont::draw(const char* s,
				float x, float y, float z) const
{
  draw(s, ::strlen(s), x, y, z);
}

void			OpenGLTexFont::getBlinkColor(const GLfloat *color, float blinkFactor, GLfloat *blinkColor) const
{
  blinkFactor = fmodf(blinkFactor, BLINK_RATE) - BLINK_RATE/2.0f;
  blinkFactor = fabsf (blinkFactor) / (BLINK_RATE/2.0f);
  blinkFactor = BLINK_DEPTH * blinkFactor + (1.0f - BLINK_DEPTH);

  blinkColor[0] = color[0] * blinkFactor;
  blinkColor[1] = color[1] * blinkFactor;
  blinkColor[2] = color[2] * blinkFactor;
}

void			OpenGLTexFont::draw(const char* string, int length,
				float x, float y, float z) const
{
  bool blinking = false;
  bool underline = false;
  bool textures = OpenGLTexture::getFilter() != OpenGLTexture::Off;
  const GLfloat white_color[3] = {1.0f, 1.0f, 1.0f};
  const GLfloat grey_color[3]  = {0.5f, 0.5f, 0.5f};
  const GLfloat cyan_color[3]  = {0.5f, 0.8f, 0.85f};
  const GLfloat *color = white_color;
  GLfloat blinkColor[3];
  float xpos = x;

  if (textures) {
    rep->gstate.setState();
    glBegin(GL_QUADS);
  }
  else {
    if (!bitmapRep)
      ((OpenGLTexFont*)this)->bitmapRep =
			BitmapRep::getBitmapRep(rep, (int)width, (int)height);
    OpenGLTexFont::BitmapRep::setState();
  }

  float blinkTime = TimeKeeper::getCurrent().getSeconds();


  for (int i = 0; i < length; i++) {
    const unsigned int c = (unsigned int)string[i];

    if (blinking) {
      getBlinkColor(color, blinkTime, blinkColor);
      glColor3fv(blinkColor);
    }


    if (c >= 32 && c < 127) {
      if (textures) {
	const Glyph& g = rep->glyph[c - 32];
	const float w = width * g.width;
	const float h = height * g.height;
	const float dx = width * g.su;
	const float dy = -height * g.sv;
	const float x0 = floorf(x + 0.5f + dx);
	const float y0 = floorf(y + 0.5f + dy);

	if (underline == true) {
	  glEnd();	// GL_QUADS

	  OpenGLGState::resetState();   // FIXME -- full reset required?

	  if (underlineColor == CyanColor) {
	    glColor3fv(cyan_color);
	  }
	  else if (underlineColor == GreyColor) {
	    glColor3fv(grey_color);
	  }

	  glBegin(GL_LINES);
	  glVertex2f (x, y - 1.0f);
	  glVertex2f (x + width * g.advance + 1.0f, y - 1.0f);
	  glEnd();

	  rep->gstate.setState();
	  if (blinking)
	    glColor3fv(blinkColor);
	  else
	    glColor3fv(color);
	  glBegin(GL_QUADS);
	}

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
      else {
	float xmove;

	// FIXME -- RasterPos call required?
	// this call seems to set the current raster color,
	// straight calls to glColor don't do the trick.
	// as well, the x and y coordinates seem off by -1.0
	// compared to the texture mapped version

	glRasterPos3f(xpos + 1.0f, y + 1.0f, z);

	xmove = bitmapRep->drawChar (c);

	if (underline == true) {
	  if (underlineColor == CyanColor) {
	    glColor3fv(cyan_color);
	  }
	  else if (underlineColor == GreyColor) {
	    glColor3fv(grey_color);
	  }

	  glBegin(GL_LINES);
	  glVertex2f(xpos, y - 1.0f);		// compared to textured, -1.0
	  glVertex2f(xpos + xmove, y - 1.0f);	// compared to textured, -1.0
	  glEnd ();

	  // because we've underlined first, reset the color
	  if (blinking)
	    glColor3fv(blinkColor);
	  else
	    glColor3fv(color);
	}

	xpos = xpos + xmove;
      }
    }
    else if (c == ESC_CHAR) {          // process the ANSI color codes
      i++;

      if ((i < length) && (string[i] == '[')) {
	int pos;
	bool blink_tmp = blinking;
	bool uline_tmp = underline;
	int color_tmp = -1; // color

	do {
	  i++;
	  pos = i;

	  while ((i < length) && (string[i] >= '0') && (string[i] <= '9'))
	    i++;

	  if ((i < length) && ((string[i] == ';') || (string[i] == 'm'))) {

	    // process the single digit codes

	    if ((i - pos) == 1) {
	      switch (string[pos]) {
		case '0' : {
		  // RESET
		  blink_tmp = false;
		  uline_tmp = false;
		  color_tmp = DefaultColor;
		  break;
		}
		case '4' : {
		  // UNDERLINE
		  uline_tmp = true;
		  break;
		}
		case '5' : {
		  // BLINK
		  blink_tmp = true;
		  break;
		}
		default : {
		  // unknown or unused code
		  break;
		}
	      }     // end switch
	    }       // end (i - pos) == 1

	    // process the double-digit codes (colors)

	    else if (((i - pos) == 2) && (string[pos] == '3')) {
	      if ((string[pos + 1] >= '0') && (string[pos + 1] <= '7')) {
		color_tmp = color_map[string[pos + 1] - '0'];
	      }
	    }
	  }
	  else {
	    break;	// not ';' or 'm'
	  }
	} while (string[i] != 'm');


	// the codes are only valid if terminated with a 'm'

	if (string[i] == 'm') {

	  blinking = blink_tmp;
	  underline = uline_tmp;

	  if (color_tmp != -1) {
	    /* we could check up to the number of stored colors,
	     * but we don't need that many
	     */
	    if ((color_tmp < 5) && (color_tmp >= 0)) {
	      color = storedColor[color_tmp];
	    }
	    else if (color_tmp == GreyColor) {
	      color = grey_color;
	    }
	    else if (color_tmp == WhiteColor) {
	      color = white_color;
	    }
	    else if (color_tmp == CyanColor) {
	      color = cyan_color;
	    } else {
	      std::cerr << "Unknown color encountered in " << __FILE__ << " on line " << __LINE__ << std::endl;
	    }
	  }
	  glColor3fv(color);
	}
	else {
	  // Bad Ending code
	}
      }
      else {
	// Bad Beginning code
      }
    } // IF (c == ESC_CHAR)
  }

  if (textures)
    glEnd();
}

int OpenGLTexFont::stripAnsiCodes(char * string, int length)
{
  int i, j;

  j=0;

  if (string == NULL) {
    return j;
  }

  for (i=0 ; i<length ; i++) {
    if (string[i] == ESC_CHAR) {
      i++;

      if ((i < length) && (string[i] == '[')) {
	i++;

	while ((i < length) && ((string[i] == ';') ||
	       ((string[i] >= '0') && (string[i] <= '9')))) {
	  i++;
	}
      }
    }
    else {
      string[j] = string[i];
      j++;
    }
  }

  string[j] = '\0';

  return j;
}


int OpenGLTexFont::rawStrlen(const char * string, int length)
{
  int i, j;

  j=0;

  if (string == NULL) {
    return j;
  }

  for (i=0 ; i<length ; i++) {
    if (string[i] == ESC_CHAR) {
      i++;

      if ((i < length) && (string[i] == '[')) {
	i++;

	while ((i < length) && ((string[i] == ';') ||
	       ((string[i] >= '0') && (string[i] <= '9')))) {
	  i++;
	}
      }
    } else {
      j++;
    }
  }

  return j;
}

void OpenGLTexFont::setUnderlineColor(int code)
{
  switch (code) {
    case 0 :
      underlineColor = CyanColor;
      break;
    case 1 :
      underlineColor = GreyColor;
      break;
    default:
      underlineColor = -1;
      break;
  }
  return;
}

std::string OpenGLTexFont::getUnderlineColor()
{
  switch (underlineColor) {
    case CyanColor:
      return "0";
    case GreyColor:
      return "1";
    default:
      return "2";
  }
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

