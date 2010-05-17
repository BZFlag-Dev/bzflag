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

// interface header
#include "FontSizer.h"

// common headers
#include "bzfio.h"
#include "BzTime.h"
#include "FontManager.h"
#include "StateDatabase.h"
#include "LocalFontFace.h"


FontSizer::FontSizer(int width, int height)
{
  resize(width, height);
  setMin(0, 0);
  setDebug(false);
}


FontSizer::FontSizer(float width, float height)
{
  resize((int)width, (int)height);
  setMin(0, 0);
  setDebug(false);
}


FontSizer::~FontSizer()
{
}


void FontSizer::resize(int width, int height)
{
  _width = width;
  _height = height;

  _tiny     = BZDB.eval("tinyFontSize");
  _small    = BZDB.eval("smallFontSize");
  _medium   = BZDB.eval("mediumFontSize");
  _large    = BZDB.eval("largeFontSize");
  _gigantic = BZDB.eval("giganticFontSize");

  _biggest = _gigantic; // probably, but check the rest anyways
  if (_biggest < _large)  _biggest = _large;
  if (_biggest < _medium) _biggest = _medium;
  if (_biggest < _small)  _biggest = _small;
  if (_biggest < _tiny)   _biggest = _tiny;

  _smallest = _tiny; // probably, but check the rest anyways
  if (_smallest > _small)    _smallest = _small;
  if (_smallest > _medium)   _smallest = _medium;
  if (_smallest > _large)    _smallest = _large;
  if (_smallest > _gigantic) _smallest = _gigantic;

  setMin(0, 10);
}


void FontSizer::setMin(int charWide, int charTall)
{
  _charWide = charWide;
  _charTall = charTall;
}


float FontSizer::getFontSize(LocalFontFace* face, const std::string& sizeVar)
{
  return getFontSize(face->getFMFace(), sizeVar);
}


float FontSizer::getFontSize(int faceID, const std::string& sizeVar)
{
  float size = BZDB.eval(sizeVar);
  if (size < 0.0f || isnan(size)) {
    size = _medium;
  }
  if (size > 1.0f) {
    // need to normalize
    FontManager &fm = FontManager::instance();

    // approx width of a char in this font size, just need the ratio
    const float wide = fm.getStringWidth(faceID, size, "BZFlag") / 6.0f;

    size = (wide / (float)_width);
  }
  return getFontSize(faceID, size);
}


float FontSizer::getFontSize(int faceID, float zeroToOneSize)
{
  float fontSize;

  // clamp inputs
  if (zeroToOneSize < 0.0f) {
    zeroToOneSize = 0.0f;
  } else if (zeroToOneSize > 1.0f) {
    zeroToOneSize = 1.0f;
  }

  // make sure the font will "fit", otherwise go even smaller
  FontManager &fm = FontManager::instance();

  // approx width of a char in this font size, just need the ratio
  //  const float wide = fm.getStringWidth(faceID, BZDB.eval("mediumFontSize"), "BZFlag") / 6.0f;
  //  const float tall = fm.getStringHeight(faceID, BZDB.eval("mediumFontSize"));
  fontSize = (float)_width * zeroToOneSize;

#if 0
  // requested font size, use most limiting aspect
  const bool fillsVertically = ((float)_height / tall) < ((float)_width / wide);
  if (fillsVertically) {
    // font fills vertically first
    if (_debug) {
      printf("fills vertically hdiv is %f (tall is %f)\n", _height / tall, tall);
      printf("fills vertically wdiv is %f (tall is %f)\n", _width / wide, wide);
    }
    fontSize = (float)_height * zeroToOneSize;
  } else {
    // font fills horizontally
    if (_debug) {
      printf("fills horizontally\n");
    }
  }
#endif

  // make sure the font is at least as small as needed for char grid
  bool underMin = false;
  register float curWide;
  register float curTall;
  static double totalTime = 0.0;
  do {
    const BzTime t0 = BzTime::getCurrent();
    curWide = (float)_charWide * (fm.getStringWidth(faceID, fontSize, "BZFlag") / 6.0f);
    curTall = (float)_charTall * (fm.getStringHeight(faceID, fontSize));
    const BzTime t1 = BzTime::getCurrent();
    totalTime += (t1 - t0);

    // FIXME -- leave this here until it gets fixed
    logDebugMessage(0,
      "FontSizer::getFontSize(%i, %5.3f, '%s')"
      "  %8.3f for [%4ix%4i]"
      "  (%5.3f seconds, total = %.3f)\n",
      faceID, zeroToOneSize, fm.getFaceName(faceID),
      fontSize, _width, _height,
      (t1 - t0), totalTime);

    if (_debug) {
      printf("character grid is %d x %d\n", _charWide, _charTall);
    }

    if ((curWide <= (float)_width) && (curTall <= (float)_height)) {
      if (_debug) {
	printf("UNDER at %f (cur is %f x %f, 0to1 is %f)\n", fontSize, curWide, curTall, zeroToOneSize);
      }
      underMin = true;
    } else {
      if (_debug) {
	if (curWide > (float)_width) {
	  printf("OVER WIDE min at %f (cur est is %f x %f (win is %f x %f), 0to1 is %f (1/%d)\n", fontSize, curWide, curTall, (float)_width, (float)_height, zeroToOneSize, (int)(1.0 / zeroToOneSize));
	} else {
	  printf("OVER TALL min at %f (cur is %f x %f, 0to1 is %f (%d dpi))\n", fontSize, curWide, curTall, zeroToOneSize, (int)(zeroToOneSize * 100.0f));
	}
      }
      fontSize -= 4.0f;
    }
  } while (!underMin && fontSize > _smallest);

  // don't care about biggest, but do care about smallest for readability
  if (fontSize < _smallest) {
    fontSize = _smallest;
  }

#if 0
  // is it close to one of the fixed sizes? group them together.
  // clamp to the closest size.
  if (fontSize < _biggest) {
    if ((fontSize < _tiny) || (fontSize < _tiny + ((_small - _tiny) * 0.5f))) {
      fontSize = _tiny;
    } else if ((fontSize < _small) || (fontSize < _small + ((_medium - _small) * 0.5f))) {
      fontSize = _small;
    } else if ((fontSize < _medium) || (fontSize < _medium + ((_large - _medium) * 0.5f))) {
      fontSize = _medium;
    } else if ((fontSize < _large) || (fontSize < _large + ((_gigantic - _large) * 0.5f))) {
      fontSize = _large;
    } else if ((fontSize < _gigantic) || (fontSize < _gigantic + (_tiny * 0.5f))) {
      fontSize = _gigantic;
    }
  }
#endif

  if (_debug) {
    printf("USING %f\n", fontSize);
  }

  return fontSize;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
