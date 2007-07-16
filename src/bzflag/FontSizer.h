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
/** @{ */
/** @file FontSizer.h
 *
 * The FontSizer is a font sizing class that is used by the client to
 * consistently provide sizes of fonts, ensuring that a particular
 * requested font size is not too big or too small given the current
 * graphics context size.  The class also autosizes based on the
 * current context size so that the display consistently scales
 * regardless of the window size or aspect ratio.
 *
 * Authors -
 *   Christopher Sean Morrison
 */

#ifndef	__FONTSIZER_H__
#define	__FONTSIZER_H__

#include "common.h"

/* system interface headers */
#include <string>


/**
 * FontSizer provides automatic font sizing routines so that font
 * sizes are appropriate and consistent for a given context size.
 */
class FontSizer {

 public:

  FontSizer(int width = 640, int height = 480);
  ~FontSizer();

  /**
   * adjust parameters for a different context size
   */
  void resize(int width, int height);

  /**
   * ensure size returned will fit the provided character grid size.
   */
  void setMin(int charWide, int charTall);

  /**
   * enable/disable compile-time debugging
   */
  void setDebug(bool on = true) {
    _debug = on;
  }

  /**
   * returns a font point size based on a BZDB font name.  if the
   * value is greater than 1, then it is normalized to the context
   * size and treated as a point size.  Otherwise, it's treated as a
   * zeroToOne value.
   */
  float getFontSize(int faceID, std::string fontName);

  /**
   * returns a font point size based on a 0->1 scale for a requested
   * input font size.  Smaller font sizes are grouped together into
   * BZDB sizes of "tinyFontSize", "smallFontSize", "mediumFontSize",
   * "largeFontSize", and "hugeFontSize".
   */
  float getFontSize(int faceID = 0, float zeroToOneSize = 0.0f);

 protected:
  int _width;
  int _height;

  int _charWide;
  int _charTall;

 private:
  float _tiny;
  float _small;
  float _medium;
  float _large;
  float _huge;

  float _smallest;
  float _biggest;

  bool _debug;
};

#endif /* __FONTSIZER_H__ */

/** @} */
// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
