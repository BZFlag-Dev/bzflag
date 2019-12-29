/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// Own include
#include "SDL2Window.h"

// Common includes
#include "OpenGLGState.h"
#include "TimeKeeper.h"

#ifdef _WIN32
HWND SDLWindow::hwnd = NULL;
#endif

SDLWindow::SDLWindow(const SDLDisplay* _display, SDLVisual*)
    : BzfWindow(_display), hasGamma(true), origGamma(-1.0f), lastGamma(1.0f),
      windowId(NULL), glContext(NULL), canGrabMouse(true), fullScreen(false),
      vsync(false), base_width(640), base_height(480), min_width(-1), min_height(-1)
{
}

SDLWindow::~SDLWindow()
{
    // Restore the original gamma when we exit the client
    setGamma(origGamma);
}

void SDLWindow::setTitle(const char *_title)
{
    title = _title;
    if (windowId)
        SDL_SetWindowTitle(windowId, title.c_str());
}

void SDLWindow::setFullscreen(bool on)
{
    fullScreen = on;
}

void SDLWindow::iconify(void)
{
    // workaround for SDL 2 bug on macOS (or just a macOS bug?) where trying to
    // iconify the window just sends it to the background instead
    // bug report: https://bugzilla.libsdl.org/show_bug.cgi?id=4177
    // TODO: Remove this workaround when/if SDL2 includes their own workaround.
#if defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_13) && \
    MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_13
    if (fullScreen)
        SDL_SetWindowFullscreen(windowId, 0);
#endif

    SDL_MinimizeWindow(windowId);

    // continuation of above workaround; so far, it seems sufficient to simply
    // set the window back to fullscreen after minimizing it; this does seem a
    // bit precarious...
#if defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_13) && \
    MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_13
    if (fullScreen)
        SDL_SetWindowFullscreen(windowId, SDL_WINDOW_FULLSCREEN);
#endif
}


void SDLWindow::disableConfineToMotionbox()
{
#ifndef _WIN32
    SDL_SetWindowGrab(windowId, SDL_FALSE);
#else
    ClipCursor(NULL);
#endif
}


void SDLWindow::confineToMotionbox(int x1, int y1, int x2, int y2)
{
#ifndef _WIN32
    if (! SDL_GetWindowGrab(windowId))
        SDL_SetWindowGrab(windowId, SDL_TRUE);

    BzfWindow::confineToMotionbox(x1, y1, x2, y2);
#else
    int posx, posy;
    SDL_GetWindowPosition(windowId, &posx, &posy);

    // Store the boundary positions as rectangle
    RECT rect;
    rect.top = y1 + posy;
    rect.left = x1 + posx;
    rect.bottom = y2 + posy;
    rect.right = x2 + posx;

    // Restrict cursor to that rectangle
    ClipCursor(&rect);
#endif
}


void SDLWindow::warpMouse(int _x, int _y)
{
    SDL_WarpMouseInWindow(windowId, _x, _y);
}

void SDLWindow::getMouse(int& _x, int& _y) const
{
    SDL_GetMouseState(&_x, &_y);
}

void SDLWindow::setSize(int _width, int _height)
{
    base_width  = _width;
    base_height = _height;
    if (!fullScreen && windowId)
        SDL_SetWindowSize(windowId, base_width, base_height);
}

void SDLWindow::getSize(int& width, int& height) const
{
    SDL_GL_GetDrawableSize(windowId, &width, &height);
}

void SDLWindow::setGamma(float gamma)
{
    lastGamma = gamma;
    int result = SDL_SetWindowBrightness(windowId, gamma);
    if (result == -1)
    {
        printf("Could not set Gamma: %s.\n", SDL_GetError());
        hasGamma = false;
    }
}

float SDLWindow::getGamma() const
{
    return SDL_GetWindowBrightness(windowId);
}

bool SDLWindow::hasGammaControl() const
{
    return hasGamma;
}

void SDLWindow::swapBuffers()
{
    SDL_GL_SwapWindow(windowId);

    // workaround for SDL 2 bug on mac where an application window obstructed
    // by another window will not honor a vsync restriction
    // bug report: https://bugzilla.libsdl.org/show_bug.cgi?id=2998
    // TODO: Remove this workaround when/if SDL2 includes their own workaround.
#ifdef __APPLE__
    if (! SDL_GL_GetSwapInterval())
        return;

    const int maxRunawayFPS = 65;

    static TimeKeeper lastFrame = TimeKeeper::getSunGenesisTime();
    const TimeKeeper now = TimeKeeper::getCurrent();

    const double remaining = 1.0 / (double) maxRunawayFPS - (now - lastFrame);

    // this doesn't create our exact desired FPS, since our handling is
    // frame-to-frame and some frames will be late already and will not be
    // delayed, but it's close enough for the purposes of this workaround
    if (remaining > 0.0)
        TimeKeeper::sleep(remaining);

    lastFrame = now;
#endif //__APPLE__
}

bool SDLWindow::create(void)
{
    int targetWidth, targetHeight;
    // If fullscreen, the target size will be that of the selected fullscreen resolution
    if (fullScreen)
        const_cast<SDLDisplay *>((const SDLDisplay *)getDisplay())->getWindowSize(targetWidth, targetHeight);
    // Otherwise, it will be the base size, which controls the windowed size
    else
    {
        targetWidth = base_width;
        targetHeight = base_height;
    }

    // Create the SDL window if it doesn't already exist
    if (!windowId)
    {
        const Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
        windowId = SDL_CreateWindow(
                       title.c_str(),
                       SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED,
                       base_width,
                       base_height,
                       flags);

        // If the window could not be created, bail out
        if (!windowId)
        {
            printf("Could not create the window: %s\n", SDL_GetError());
            return false;
        }

        // Store the gamma immediately after creating the first window
        if (origGamma < 0)
            origGamma = getGamma();

        // Set the minimum window size
        setMinSize(min_width, min_height);

        // Create the OpenGL context and make it current
        makeContext();
        makeCurrent();

#ifdef _WIN32
        SDL_VERSION(&info.version);
        if (SDL_GetWindowWMInfo(windowId,&info))
        {
            if (info.subsystem == SDL_SYSWM_WINDOWS)
                hwnd = info.info.win.window;
        }
#endif

        // Set desired vertical-sync mode
        setVerticalSync(vsync);

        // init opengl context
        OpenGLGState::initContext();
    }

    // Get the current window dimensions
    int currentWidth, currentHeight;
    SDL_GetWindowSize(windowId, &currentWidth, &currentHeight);

    // Get the current window flags
    Uint32 currentWindowFlags = SDL_GetWindowFlags(windowId);

    if (targetWidth != currentWidth || targetHeight != currentHeight)
    {
        // If we're fullscreen (or switching to fullscreen), find the closest resolution and set the display mode
        if (fullScreen)
        {
            SDL_DisplayMode closest;
            SDL_DisplayMode target;
            target.w = targetWidth;
            target.h = targetHeight;
            target.format = 0;
            target.refresh_rate = 0;
            target.driverdata = nullptr;

            // Attempt to find a usable resolution close to our target resolution
            if (SDL_GetClosestDisplayMode(0, &target, &closest) == nullptr)
            {
                printf("Unable to find a usable fullscreen resolution: %s\n", SDL_GetError());
                return false;
            }

            // Attempt to set the display mode
            if (SDL_SetWindowDisplayMode(windowId, &closest) < 0)
            {
                printf("Unable to set display mode: %s", SDL_GetError());
                return false;
            }
        }
        // Otherwise just set the window size
        else
        {
            //SDL_SetWindowSize(windowId, targetWidth, targetHeight);
        }
    }

    // Check if we need to toggle between fullscreen and windowed
    if (fullScreen != (currentWindowFlags & SDL_WINDOW_FULLSCREEN))
    {
        // Adjust the fullscreen/resizable flags
        SDL_SetWindowFullscreen(windowId, fullScreen?SDL_WINDOW_FULLSCREEN:0);
        SDL_SetWindowResizable(windowId, fullScreen?SDL_FALSE:SDL_TRUE);
    }

    return true;
}

void SDLWindow::enableGrabMouse(bool on)
{
    canGrabMouse = on;
    if (canGrabMouse)
        SDL_SetWindowGrab(windowId, SDL_TRUE);
    else
        SDL_SetWindowGrab(windowId, SDL_FALSE);
}

void SDLWindow::makeContext()
{
    glContext = SDL_GL_CreateContext(windowId);
    if (!glContext)
        printf("Could not Create GL Context: %s.\n", SDL_GetError());
}

void SDLWindow::setVerticalSync(bool setting)
{
    vsync = setting;

    if (windowId != NULL)
        if (glContext != NULL)
            if(SDL_GL_SetSwapInterval(vsync ? -1 : 0) == -1 && vsync)
                // no adaptive vsync; set regular vsync
                SDL_GL_SetSwapInterval(1);
}

void SDLWindow::setMinSize(int width, int height)
{
    min_width  = width;
    min_height = height;
    if (!windowId)
        return;
    SDL_SetWindowMinimumSize (windowId, width, height);
}

void SDLWindow::makeCurrent()
{
    if (!windowId)
        return;
    if (!glContext)
        return;
    int result = SDL_GL_MakeCurrent(windowId, glContext);
    if (result < 0)
    {
        printf("Could not Make GL Context Current: %s.\n", SDL_GetError());
        abort();
    }
}

void SDLWindow::freeContext()
{
    SDL_GL_DeleteContext(glContext);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
