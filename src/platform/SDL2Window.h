/* bzflag
 * Copyright (c) 1993-2025 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* SDL2Window:
 *  Encapsulates an SDL window
 */

#ifndef BZF_SDL2WINDOW_H
#define BZF_SDL2WINDOW_H

// Inherits from
#include "BzfWindow.h"

// System includes
#include <string>
#include <vector>
#include <cstdint>

// Local includes
#include "SDL2Display.h"
#include "SDL2Visual.h"
#ifdef _WIN32
#  include "SDL2/SDL_syswm.h"
#endif

class SDLWindow final : public BzfWindow
{
public:
    SDLWindow(const SDLDisplay* _display, SDLVisual*);
    ~SDLWindow();
    bool  isValid() const override
    {
        return true;
    };
    void  showWindow(bool) override
    {
        ;
    };
    void  getPosition(int &, int &) override
    {
        ;
    };
    void  getSize(int& width, int& height) const override;
    void  setSize(int width, int height) override;
    void  setTitle(const char * title) override;
    void  setPosition(int, int) override
    {
        ;
    };
    void  setMinSize(int, int) override;
    void  setFullscreen(bool) override;
    void  iconify(void) override;
    void  disableConfineToMotionbox() override;
    void  confineToMotionbox(int x1, int y1, int x2, int y2) override;
    void  warpMouse(int x, int y) override;
    void  getMouse(int& x, int& y) const override;
    void  grabMouse() override
    {
        ;
    };
    void  ungrabMouse() override
    {
        ;
    };
    void  enableGrabMouse(bool) override;
    void  showMouse() override
    {
        ;
    };
    void  hideMouse() override
    {
        ;
    };
    void  setGamma(float newGamma) override;
    float getGamma() const override;
    bool  hasGammaControl() const override;
    bool hasVerticalSync() const override
    {
        return true;
    }
    void  setVerticalSync(bool) override;
    void  makeCurrent() override;
    void  swapBuffers() override;
    void  makeContext() override;
    void  freeContext() override;
    bool  create(void) override;
#ifdef _WIN32
    static HWND getHandle()
    {
        return hwnd;
    }
#endif
private:
    bool   hasGamma;
    float  origGamma;
    float  lastGamma;
    SDL_Window *windowId;
#ifdef _WIN32
    SDL_SysWMinfo info;
    static HWND hwnd;
#endif
    SDL_GLContext glContext;
    std::string title;
    bool canGrabMouse;
    bool fullScreen;
    bool vsync;
    int  base_width;
    int  base_height;
    int  min_width;
    int  min_height;
};

#endif // BZF_SDL2WINDOW_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
