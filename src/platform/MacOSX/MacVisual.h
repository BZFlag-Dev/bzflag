/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* WinVisual:
 *  Builders for Windows visuals suitable for OpenGL contexts and windows.
 */

#ifndef BZF_MACVISUAL_H
#define BZF_MACVISUAL_H

#include "bzfgl.h"
#include "BzfVisual.h"
#include "MacVisual.h"
#include "MacDisplay.h"

#include <vector>
using std::vector;

#include <Carbon/Carbon.h>
#include <AGL/agl.h>
class MacVisual : public BzfVisual {
  public:
    MacVisual(const MacDisplay*);
    MacVisual();

    void setLevel(int level);
    void setDoubleBuffer(bool);
    void setIndex(int minDepth);
    void setRGBA(int minRed, int minGreen,
    int minBlue, int minAlpha);
    void setDepth(int minDepth);
    void setStencil(int minDepth);
    void setAccum(int minRed, int minGreen,
    int minBlue, int minAlpha);
    void setStereo(bool);
    void setMultisample(int minSamples);

    bool build();
    AGLPixelFormat get() const { return pixel_format; }

    void reset() { attributes.clear(); }
    void addAttribute1(GLint attribute);
  protected:
    int findAttribute(GLint attribute);

    void addAttribute2(GLint attribute, int value);
    void removeAttribute1(GLint attribute);
    void removeAttribute2(GLint attribute);

  private:
    const MacDisplay *display;
    // attributes we would like for our pixel format
    // last attribute must be AGL_NONE
    vector<GLint>       attributes;
    AGLPixelFormat      pixel_format;
};

#endif // BZF_MACVISUAL_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

