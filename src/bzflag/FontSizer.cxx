/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

// interface header
#include "FontSizer.h"

// system headers
#include <algorithm>

// common headers
#include "bzfio.h"
#include "BzTime.h"
#include "BZDBCache.h"
#include "FontManager.h"
#include "StateDatabase.h"
#include "TextUtils.h"

// local headers
#include "guiplaying.h"
#include "LocalFontFace.h"
#include "MainWindow.h"


//============================================================================//
//============================================================================//

static std::vector<float> fontSizes;
static std::string        fontSizesString;

static BZDB_string bzdbFontSizes("fontSizes");

static BZDB_bool debugFontSizer("debugFontSizer");


//============================================================================//

static int getViewHeight()
{
  MainWindow* mw = getMainWindow();
  return (mw != NULL) ? mw->getHeight() : 480;
}


static float findClosest(float fontSize)
{
  if (fontSize > fontSizes.back()) {
    return fontSize;
  }
  for (size_t i = 0; i < (fontSizes.size() - 1); i++) {
    const float avgSize = 0.5f * (fontSizes[i] + fontSizes[i + 1]);
    if (fontSize < avgSize) {
      return fontSizes[i];
    }
  }
  return fontSizes.back();
}


static void setupDefaultSizes()
{
  fontSizes.clear();
  fontSizes.push_back( 8.0f);
  fontSizes.push_back(12.0f);
  fontSizes.push_back(16.0f);
  fontSizes.push_back(24.0f);
  fontSizes.push_back(32.0f);
  fontSizes.push_back(40.0f);
  fontSizes.push_back(64.0f);
}


static void setupFontSizes()
{
  const std::string& bzdbString = bzdbFontSizes;

  if (fontSizesString != bzdbString) {

    fontSizesString = bzdbString;

    fontSizes.clear();

    const std::vector<std::string> args =
      TextUtils::tokenize(fontSizesString, " \t");
    for (size_t i = 0; i < args.size(); i++) {
      char* end;
      const char* start = args[i].c_str();
      const float value = strtof(start, &end);
      if ((end != start) && !isnan(value)) {
        if (value >= 1.0f) {
          fontSizes.push_back(value);
        } else {
          fontSizes.push_back(floorf(value * getViewHeight()));
        }
      }
    }

    if (fontSizes.empty()) {
      setupDefaultSizes();
    }

    std::sort(fontSizes.begin(), fontSizes.end());

    if (debugFontSizer) {
      printf("FontSizer: minSize = %.3f\n", fontSizes.front());
      printf("FontSizer: maxSize = %.3f\n", fontSizes.back());
      for (size_t i = 0; i < fontSizes.size(); i++) {
        printf("FontSizer: fontSizes[%i] = %.3f\n", (int)i, fontSizes[i]);
      }
    }
  }

}

//============================================================================//
//============================================================================//

FontSizer::FontSizer(int _xpixels, int _ypixels)
: xchars(0)
, ychars(10)
{
  resize(_xpixels, _ypixels);
}


FontSizer::FontSizer(float _xpixels, float _ypixels)
: xchars(0)
, ychars(10)
{
  resize((int)_xpixels, (int)_ypixels);
}


FontSizer::~FontSizer()
{
}


//============================================================================//

void FontSizer::resize(int _xpixels, int _ypixels)
{
  xpixels = _xpixels;
  ypixels = _ypixels;

  setupFontSizes();
}


//============================================================================//

float FontSizer::getFontSize(LocalFontFace* face, const std::string& bzdbExpr)
{
  return getFontSize(face->getFMFace(), bzdbExpr);
}


float FontSizer::getFontSize(int faceID, const std::string& bzdbExpr)
{
  const float size = BZDB.eval(bzdbExpr);
  if (size > 1.0f) {
    return size;
  }

  return getFontSize(faceID, size);
}


float FontSizer::getFontSize(int faceID, float zeroToOneSize)
{
  FontManager& fm = FontManager::instance();

  if (debugFontSizer) {
    printf("FontSizer: %s  0to1=%.3f, xpixels=%i, ypixels=%i"
           " xchars=%i ychars=%i\n", fm.getFaceName(faceID),
           zeroToOneSize, xpixels, ypixels, xchars, ychars);
  }

  // sanitize and clamp inputs
  if (isnan(zeroToOneSize)) {
    return fontSizes[0];
  }
  else if (zeroToOneSize < 0.0f) {
    zeroToOneSize = 0.0f;
  }
  else if (zeroToOneSize > 1.0f) {
    zeroToOneSize = 1.0f;
  }

  const int viewHeight = getViewHeight();

  float fontSize = (float)viewHeight * zeroToOneSize;

  const float inSize = fontSize;

  fontSize = findClosest(fontSize);

  if ((ychars > 0) && (ypixels > 0)) {
    const float ysize = float(ypixels) / float(ychars);
    const float lineHeight = fm.getStringHeight(faceID, fontSize);
    if (lineHeight > ysize) {
      fontSize *= (ysize / lineHeight);
      fontSize = findClosest(fontSize);
    }
    if (debugFontSizer) {
      printf("  y-limit: %i/%i (%.3f)\n", ypixels, ychars, ysize);
    }
  }

  fontSize = findClosest(fontSize);

  if ((xchars > 0) && (xpixels > 0)) {
    const float xsize = float(xpixels) / float(xchars);
    const std::string testStr = "BZFlag";
    const float charSize =
      fm.getStringWidth(faceID, fontSize, testStr) / float(testStr.size());;
    if (charSize > xsize) {
      fontSize *= (xsize / charSize);
      fontSize = findClosest(fontSize);
    }
    if (debugFontSizer) {
      const float newCharSize =
        fm.getStringWidth(faceID, fontSize, testStr) / float(testStr.size());;
      printf("  x-limit: %i/%i (%.3f) charSize=%.3f, newCharSize=%.3f\n",
             xpixels, xchars, xsize, charSize, newCharSize);
    }
  }

  if (debugFontSizer) {
    printf("  height = %i, zeroToOne = %.3f,"
           " inSize = %.3f, outSize = %.3f, outHeight = %.3f\n",
           viewHeight, zeroToOneSize, inSize,
           fontSize, fm.getStringHeight(faceID, fontSize));
  }

  return fontSize;
}


//============================================================================//
//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
