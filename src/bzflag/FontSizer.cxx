/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "FontSizer.h"

/* common implementation headers */
#include "FontManager.h"
#include "StateDatabase.h"


FontSizer::FontSizer(int width, int height)
{
  resize(width, height);
}

FontSizer::~FontSizer()
{
}

void
FontSizer::resize(int width, int height)
{
  _width = width;
  _height = height;

  _tiny = BZDB.eval("tinyFontSize");
  _small = BZDB.eval("smallFontSize");
  _medium = BZDB.eval("mediumFontSize");
  _large = BZDB.eval("largeFontSize");
  _huge = BZDB.eval("hugeFontSize");

  _biggest = _huge; // probably, but check the rest anyways
  if (_biggest < _large) _biggest = _large;
  if (_biggest < _medium) _biggest = _medium;
  if (_biggest < _small) _biggest = _small;
  if (_biggest < _tiny) _biggest = _tiny;

  _smallest = _tiny; // probably, but check the rest anyways
  if (_smallest > _small) _smallest = _small;
  if (_smallest > _medium) _smallest = _medium;
  if (_smallest > _large) _smallest = _large;
  if (_smallest > _huge) _smallest = _huge;
}

float
FontSizer::getFontSize(int faceID, std::string name)
{
  float size = BZDB.eval(name);
  if (size < 0.0f || isnan(size)) {
    size = _medium;
  }
  if (size > 1.0f) {
    // need to normalize
    FontManager &fm = FontManager::instance();

    // approx width of a char in this font size, just need the ratio
    const float wide = fm.getStringWidth(faceID, size, "BZ") / 2.0f;
    const float tall = fm.getStringHeight(faceID, size);

    size = tall / _height;
    if (size < (wide / _width)) {
      size = (wide / _width);
    }
  }
  return getFontSize(faceID, size);
}

float
FontSizer::getFontSize(int faceID, float zeroToOneSize)
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
  const float wide = fm.getStringWidth(faceID, BZDB.eval("mediumFontSize"), "BZ") / 2.0f;
  const float tall = fm.getStringHeight(faceID, BZDB.eval("mediumFontSize"));

  // requested font size, use most limiting aspect
  if ((_height / tall) < (_width / wide)) {
    // font fills vertically
    fontSize = _height * zeroToOneSize;
  } else {
    // font fills horizontally
    fontSize = _width * zeroToOneSize;
  }

  // don't care about biggest, but do care about smallest for readability
  if (fontSize < _smallest) {
    fontSize = _smallest;
  }

  // is it close to one of the fixed sizes? group them together
  if (fontSize < _biggest) {
    if (fontSize <= _tiny) {
      fontSize = _tiny;
    } else if (fontSize <= _small) {
      fontSize = _small;
    } else if (fontSize <= _medium) {
      fontSize = _medium;
    } else if (fontSize <= _large) {
      fontSize = _large;
    } else if (fontSize <= _huge) {
      fontSize = _huge;
    }
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
