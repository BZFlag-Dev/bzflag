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

#ifndef __FONTSIZER_H__
#define __FONTSIZER_H__

#include "common.h"

/* system interface headers */
#include <string>
#include <vector>

class LocalFontFace;

/**
 * FontSizer provides automatic font sizing routines so that font
 * sizes are appropriate and consistent for a given context size.
 */
class FontSizer {
  public:
    FontSizer(int xpixels = 640, int ypixels = 480);
    FontSizer(float xpixels, float ypixels);
    ~FontSizer();

    /**
     * adjust parameters for a different context size
     */
    void resize(int xpixels, int ypixels);

    /**
     * set the minimum X and Y character counts
     */
    void setMin(int _xchars, int _ychars) {
      xchars = _xchars;
      ychars = _ychars;
    }

    /**
     * returns a font point size based on a BZDB var containing the size.
     * if the value is greater than 1, then it is normalized to the context
     * size and treated as a point size.  Otherwise, it's treated as a
     * zeroToOne value.
     */
    float getFontSize(LocalFontFace* face, const std::string& bzdbExpr);

    /**
     * returns a font point size based on a BZDB var containing the size.
     * if the value is greater than 1, then it is normalized to the context
     * size and treated as a point size.  Otherwise, it's treated as a
     * zeroToOne value.
     */
    float getFontSize(int faceID, const std::string& bzdbExpr);

    /**
     * returns a font point size based on a 0->1 scale for a requested
     * input font size.  Smaller font sizes are grouped together based on
     * the values in the "fontSizes" BZDB variable.
     */
    float getFontSize(int faceID = 0, float zeroToOneSize = 0.0f);

  private:
    int xpixels;
    int ypixels;
    int xchars;
    int ychars;
};

#endif /* __FONTSIZER_H__ */

/** @} */
// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
