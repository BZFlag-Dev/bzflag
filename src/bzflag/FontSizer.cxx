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

static BZDB_bool debugFontSizer("debugFontSizer");


static int getViewHeight()
{
  MainWindow* mw = getMainWindow();
  if (mw == NULL)  {
    return 480;
  }
  const int sizex = mw->getWidth();
  const int sizey = mw->getHeight();
  return (sizey < sizex) ? sizey : sizex;
}


//============================================================================//

FontSizer::FontSizer(int _width, int _height)
{
  resize(_width, _height);
}


FontSizer::FontSizer(float _width, float _height)
{
  resize((int)_width, (int)_height);
}


FontSizer::~FontSizer()
{
}


//============================================================================//

//============================================================================//

void FontSizer::resize(int _width, int _height)
{
  width  = _width;  // unused
  height = _height; // unused

  fontSizes.clear();
  const std::string s = BZDB.get("fontSizes");
  std::vector<std::string> args = TextUtils::tokenize(s, " \t");
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
  std::sort(fontSizes.begin(), fontSizes.end());

  if (fontSizes.empty()) {
    fontSizes.push_back(16.0f);
  }

  if (debugFontSizer && false) {
    printf("FontSizer: minSize = %.3f\n", fontSizes.front());
    printf("FontSizer: maxSize = %.3f\n", fontSizes.back());
    for (size_t i = 0; i < fontSizes.size(); i++) {
      printf("FontSizer: fontSizes[%i] = %.3f\n", (int)i, fontSizes[i]);
    }
  }

  setMin(0, 10);
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


float FontSizer::getFontSize(int /*faceID*/, float zeroToOneSize)
{
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

  fontSize = findClosest(fontSize);

  if (debugFontSizer) {
    printf("FontSizer: height = %i, zeroToOne = %.3f,"
                     " inSize = %.3f, outSize = %.3f\n",
           viewHeight, zeroToOneSize, (float)viewHeight * zeroToOneSize, fontSize);
  }

  return fontSize;
}


float FontSizer::findClosest(float fontSize) const
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


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
