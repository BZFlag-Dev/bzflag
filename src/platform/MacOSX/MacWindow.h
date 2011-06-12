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

#ifndef BZF_WINWINDOW_H
#define  BZF_WINWINDOW_H

#include <Carbon/Carbon.h>

#include "bzfgl.h"

#include "platform/BzfWindow.h"
#include "MacDisplay.h"
#include "MacVisual.h"

class MacWindow : public BzfWindow {
  public:
    MacWindow(const MacDisplay*, MacVisual*);
    ~MacWindow();

    bool isValid() const;

    void showWindow(bool);

    void getPosition(int& x, int& y);
    void getSize(int& width, int& height) const;

    void setTitle(const char*);
    void setPosition(int x, int y);
    void setSize(int width, int height);
    void setMinSize(int width, int height);
    void setFullscreen(bool unused = true);
    bool getFullscreen() const;

    void warpMouse(int x, int y);
    void getMouse(int& x, int& y) const;
    void grabMouse();
    void ungrabMouse();
    void showMouse();
    void hideMouse();

    void setGamma(float);
    float getGamma() const;
    bool hasGammaControl() const;

    void makeCurrent();
    void swapBuffers();
    void makeContext();
    void freeContext();

  private:
    Rect rect;

    WindowRef window;
    CGLContextObj context;
};

#endif // BZF_WINWINDOW_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
