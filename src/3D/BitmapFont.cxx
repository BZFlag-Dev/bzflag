/* bzflag
* Copyright (c) 1993 - 2006 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

// Interface header
#include "BitmapFont.h"

// System headers
#include <string>
#include <string.h>
#include <assert.h>

// Common implementation headers
#include "bzfgl.h"
#include "bzfio.h"
#include "MediaFile.h"
#include "OpenGLGState.h"

BitmapFont::BitmapFont()
{
  loaded = false;
  for (int i = 0; i < MAX_TEXTURE_FONT_CHARS; i++) {
    bitmaps[i] = 0;
    fontMetrics[i].charWidth = -1;
  }
}

BitmapFont::~BitmapFont()
{
  for (int i = 0; i < MAX_TEXTURE_FONT_CHARS; i++) {
    if (bitmaps[i])
      delete[] bitmaps[i];
  }
}

void BitmapFont::build(void)
{
  if (texture.size() < 1) {
    logDebugMessage(2,"Font %s does not have an associated texture name, not loading\n", texture.c_str());
    return;
  }

  int width, height;
  std::string textureAndDir = "fonts/" + texture;
  unsigned char* image = MediaFile::readImage(textureAndDir, &width, &height);

  for (int i = 0; i < numberOfCharacters; i++) {
    delete[] bitmaps[i];
    const int h = fontMetrics[i].endY - fontMetrics[i].startY;
    const int w = fontMetrics[i].endX - fontMetrics[i].startX;
    const int sx = fontMetrics[i].startX;
    const int ey = fontMetrics[i].endY;
    const int bytesPerRow = ((w + 31) / 8) & ~3;
    bitmaps[i] = new unsigned char[h * bytesPerRow + 4];

    const int IT = 80;	// intensity threshold
    for (int j = 0; j < h; j++) {
      const unsigned char* srcRow = &image[4*((height-ey+j)*width+sx)];
      unsigned char* dstData = bitmaps[i] + j * bytesPerRow;

      int b;
      for (b = 0; b < w - 7; b += 8) {
	unsigned char data = 0;

	if (srcRow[4*(b+0)] >= IT) data |= 0x80u;
	if (srcRow[4*(b+1)] >= IT) data |= 0x40u;
	if (srcRow[4*(b+2)] >= IT) data |= 0x20u;
	if (srcRow[4*(b+3)] >= IT) data |= 0x10u;
	if (srcRow[4*(b+4)] >= IT) data |= 0x08u;
	if (srcRow[4*(b+5)] >= IT) data |= 0x04u;
	if (srcRow[4*(b+6)] >= IT) data |= 0x02u;
	if (srcRow[4*(b+7)] >= IT) data |= 0x01u;
	*dstData++ = data;
      }

      unsigned char data = 0;
      switch (w - b) {
	case 7:
	  if (srcRow[4*(b+6)] >= IT) data |= 0x02u;
	case 6:
	  if (srcRow[4*(b+5)] >= IT) data |= 0x04u;
	case 5:
	  if (srcRow[4*(b+4)] >= IT) data |= 0x08u;
	case 4:
	  if (srcRow[4*(b+3)] >= IT) data |= 0x10u;
	case 3:
	  if (srcRow[4*(b+2)] >= IT) data |= 0x20u;
	case 2:
	  if (srcRow[4*(b+1)] >= IT) data |= 0x40u;
	case 1:
	  if (srcRow[4*(b+0)] >= IT) data |= 0x80u;
	  *dstData++ = data;
      }
    }
  }
  delete[] image;
  loaded = true;

  // create GState
  OpenGLGStateBuilder builder(gstate);
  builder.enableTexture(false);
  builder.setBlending();
  builder.setAlphaFunc();
  gstate = builder.getState();
}


void BitmapFont::free(void)
{
  loaded = false;
}

void BitmapFont::filter(bool /*dofilter*/)
{
}

void BitmapFont::drawString(float scale, GLfloat color[4], const char *str,
			    int len)
{
  // BitmapFont cannot scale, should never be asked to
  if (scale != 1.0f) {
    logDebugMessage(1,"ERROR: BitmapFont should not be asked to scale!\n");
    assert(scale == 1.0f);
  }

  if (!str)
    return;

  if (!loaded)
    build();

  if (!loaded)
    return;

  if (color[0] >= 0)
    glColor4fv(color);

  glRasterPos3f(0, 0, 0);

  gstate.setState();

  int charToUse = 0;
  for (int i = 0; i < len; i++) {
    const char space = ' '; // decimal 32
    if (str[i] < space)
      charToUse = space;
    else if (str[i] > (numberOfCharacters + space))
      charToUse = space;
    else
      charToUse = str[i];

    charToUse -= space;

    const float dx = float(fontMetrics[charToUse].fullWidth);
    if (charToUse == 0) {
      glBitmap(0, 0, 0, 0, dx, 0, 0);
    } else {
      const int h = fontMetrics[charToUse].endY - fontMetrics[charToUse].startY;
      const int w = fontMetrics[charToUse].endX - fontMetrics[charToUse].startX;
      glBitmap(w, h, float(-fontMetrics[charToUse].initialDist), 0, dx, 0, bitmaps[charToUse]);
    }
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
