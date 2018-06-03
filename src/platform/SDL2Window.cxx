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
      base_width(640), base_height(480), min_width(-1), min_height(-1)
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
    SDL_MinimizeWindow(windowId);
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
    if (fullScreen)
        const_cast<SDLDisplay *>((const SDLDisplay *)getDisplay())->getWindowSize(width, height);
    else
    {
        width  = base_width;
        height = base_height;
    }
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
    getSize(targetWidth, targetHeight);
    SDL_bool windowWasGrabbed = SDL_FALSE;
    if (windowId != NULL)
        windowWasGrabbed = SDL_GetWindowGrab(windowId);
    int swapInterval = 0;
    if (windowId != NULL)
        if (glContext != NULL)
            swapInterval = SDL_GL_GetSwapInterval() == 1;

    // if we have an existing identical window, go no further
    if (windowId != NULL)
    {
        int currentWidth, currentHeight;
        SDL_GetWindowSize(windowId, &currentWidth, &currentHeight);

        Uint32 priorWindowFlags = SDL_GetWindowFlags(windowId);
        if (fullScreen == (priorWindowFlags & SDL_WINDOW_FULLSCREEN) &&
                targetWidth == currentWidth && targetHeight == currentHeight)
            return true;
    }

    // destroy the pre-existing window if it exists
    if (windowId != NULL)
    {
        if (glContext)
            SDL_GL_DeleteContext(glContext);
        glContext = NULL;

        SDL_DestroyWindow(windowId);
    }

    // (re)create the window
    const Uint32 flags = SDL_WINDOW_OPENGL |
                         (fullScreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE) |
                         (windowWasGrabbed ? SDL_WINDOW_INPUT_GRABBED : 0);

    windowId = SDL_CreateWindow(
                   title.c_str(),
                   SDL_WINDOWPOS_UNDEFINED,
                   SDL_WINDOWPOS_UNDEFINED,
                   targetWidth,
                   targetHeight,
                   flags);

    // Store the gamma immediately after creating the first window
    if (origGamma < 0)
        origGamma = getGamma();

    // At least on Windows, recreating the window resets the gamma, so set it
    setGamma(lastGamma);

#ifdef _WIN32
    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(windowId,&info))
    {
        if (info.subsystem == SDL_SYSWM_WINDOWS)
            hwnd = info.info.win.window;
    }
#endif

    if (!windowId)
    {
        printf("Could not set Video Mode: %s.\n", SDL_GetError());
        return false;
    }

    if (min_width >= 0)
        setMinSize(min_width, min_height);

    makeContext();
    makeCurrent();

    SDL_GL_SetSwapInterval(swapInterval);

    // init opengl context
    OpenGLGState::initContext();

    // workaround for SDL 2 bug on mac where toggling fullscreen will
    // generate a resize event and mess up the window size/resolution
    // (TODO: remove this if they ever fix it)
    // bug report: https://bugzilla.libsdl.org/show_bug.cgi?id=3146
#ifdef __APPLE__
    if (fullScreen)
        return true;

    int currentDisplayIndex = SDL_GetWindowDisplayIndex(windowId);
    if (currentDisplayIndex < 0)
    {
        printf("Unable to get current display index: %s\n", SDL_GetError());
        return true;
    }

    SDL_DisplayMode desktopDisplayMode;
    if (SDL_GetDesktopDisplayMode(currentDisplayIndex, &desktopDisplayMode) < 0)
    {
        printf("Unable to get desktop display mode: %s\n", SDL_GetError());
        return true;
    }

    std::vector<SDL_Event> eventStack;
    SDL_Event thisEvent;

    // pop off all the events except a resize event
    while (SDL_PollEvent(&thisEvent))
    {
        if (thisEvent.type == SDL_WINDOWEVENT && thisEvent.window.event == SDL_WINDOWEVENT_RESIZED)
        {
            // switching from "native" fullscreen to SDL fullscreen and then going back to
            // windowed mode will generate a legitimate resize event, so add it back
            if (thisEvent.window.data1 != desktopDisplayMode.w || thisEvent.window.data2 != desktopDisplayMode.h)
                eventStack.push_back(thisEvent);
        }
        else
            eventStack.push_back(thisEvent);
    }

    // push them back on in the same order
    while (eventStack.size() > 0)
    {
        SDL_PushEvent(&eventStack[0]);

        eventStack.erase(eventStack.begin());
    }
#endif //__APPLE__

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
    SDL_GL_SetSwapInterval(setting ? 1 : 0);
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
