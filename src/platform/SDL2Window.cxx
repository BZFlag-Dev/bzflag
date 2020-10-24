/* bzflag
 * Copyright (c) 1993-2020 Tim Riker
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

    if (windowId != NULL)
        SDL_DestroyWindow(windowId);
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
    // workaround for two issues on Linux, where resizing by dragging the window corner causes glitching, and where
    // iconifying or switching applications while using a scaled fullscreen resolution causes the non-fullscreen
    // window resolution to assume the fullscreen resolution
#ifdef __linux__
    if(!fullScreen)
    {
        base_width  = _width;
        base_height = _height;
    }
#else
    base_width  = _width;
    base_height = _height;
    if (!fullScreen && windowId)
        SDL_SetWindowSize(windowId, base_width, base_height);
#endif // __linux__
}

void SDLWindow::getSize(int& width, int& height) const
{
    if (fullScreen)
        const_cast<SDLDisplay *>(static_cast<const SDLDisplay *>(getDisplay()))->getWindowSize(width, height);
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

    int maxRunawayFPS = 65;
    SDL_DisplayMode desktopDisplayMode;
    if (SDL_GetDesktopDisplayMode(0, &desktopDisplayMode) == 0)
        maxRunawayFPS = desktopDisplayMode.refresh_rate + 5;

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

// For some reason, when creating a new fullscreen window on Linux with a different resolution than before, SDL throws
// a resize event with the old window resolution, which is not what we want. This function is called to filter SDL
// window resize events right after the resolution change and adjust the resolution to the correct one.
#ifdef __linux__
int SDLWindowEventFilter(void *resolution, SDL_Event *event)
{
    if(event->type == SDL_WINDOWEVENT && (event->window.event == SDL_WINDOWEVENT_RESIZED ||
                                          event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED))
    {
        // adjust the window resolution to match the values passed to us
        event->window.data1 = static_cast<int *>(resolution)[0];
        event->window.data2 = static_cast<int *>(resolution)[1];
    }

    return 1; // allow the event
}
#endif // __linux__

bool SDLWindow::create(void)
{
    int targetWidth, targetHeight;
    getSize(targetWidth, targetHeight);
    SDL_bool windowWasGrabbed = SDL_FALSE;
    if (windowId != NULL)
        windowWasGrabbed = SDL_GetWindowGrab(windowId);

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

    // workaround for an SDL 2 bug on Linux with the GNOME Window List extension enabled, where attempting to create a
    // fullscreen window on a lower-resolution primary display while a higher-resolution secondary display is plugged in
    // causes an infinite loop of window creation on the secondary display
    // bug report: https://bugzilla.libsdl.org/show_bug.cgi?id=4990
#ifdef __linux__
    if(! fullScreen || SDL_GetNumVideoDisplays() < 2) // create the window with the standard logic
    {
#endif // __linux__
        windowId = SDL_CreateWindow(
                       title.c_str(),
                       SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED,
                       targetWidth,
                       targetHeight,
                       SDL_WINDOW_OPENGL |
                       (fullScreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE) |
                       (windowWasGrabbed ? SDL_WINDOW_INPUT_GRABBED : 0));

        // continuation of above workaround
#ifdef __linux__
    }
    else // create the window in windowed mode first and then switch to fullscreen
    {
        windowId = SDL_CreateWindow(
                       title.c_str(),
                       SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED,
                       base_width,
                       base_height,
                       SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | (windowWasGrabbed ? SDL_WINDOW_INPUT_GRABBED : 0));

        SDL_DisplayMode displayMode;
        if(SDL_GetDesktopDisplayMode(0, &displayMode) < 0)
        {
            printf("Unable to get desktop display mode: %s", SDL_GetError());
            return false;
        }
        displayMode.w = targetWidth;
        displayMode.h = targetHeight;
        if(SDL_SetWindowDisplayMode(windowId, &displayMode))
        {
            printf("Unable to set display mode: %s", SDL_GetError());
            return false;
        }
        if(SDL_SetWindowFullscreen(windowId, SDL_WINDOW_FULLSCREEN) < 0)
        {
            printf("Unable to set window to fullscreen mode: %s", SDL_GetError());
            return false;
        }
    }

    // Depending on the distribution (or possibly the window manager), SDL will not recognize the new window resolution
    // and will keep returning the previous resolution when SDL_GetWindowSize() is called, until a period of time has
    // passed and events have been pumped. Wait up to two seconds for the correct resolution to start being returned,
    // checking every quarter second, to avoid repeating the window destruction/re-creation process based on bad data.
    int currentWidth, currentHeight, resCheckLoops = 0;

    do
    {
        SDL_PumpEvents();
        SDL_GetWindowSize(windowId, &currentWidth, &currentHeight);

        if(currentWidth == targetWidth && currentHeight == targetHeight)
            break;

        TimeKeeper::sleep(0.25f);
    }
    while (resCheckLoops++ < 8);
#endif // __linux__

    // Apply filters due to resize event issues on Linux (see the explanation above for SDLWindowEventFilter())
#ifdef __linux__
    SDL_PumpEvents();
    int windowResolution[] = { targetWidth, targetHeight };
    SDL_FilterEvents(&SDLWindowEventFilter, windowResolution);
#endif // __linux__

    // Work around an issue with SDL on macOS where a window that gets resized by the operating system for various
    // reasons (e.g., creating a window that doesn't fit between the dock and menu bar, or switching from a maximized
    // window to native fullscreen then back to windowed mode) doesn't always correctly throw a resize event
#ifdef __APPLE__
    SDL_PumpEvents();

    int currentWidth, currentHeight;
    SDL_GetWindowSize(windowId, &currentWidth, &currentHeight);

    if(! fullScreen && (currentWidth != targetWidth || currentHeight != targetHeight))
    {
        SDL_Event fakeResizeEvent;
        SDL_zero(fakeResizeEvent);

        fakeResizeEvent.window.type = SDL_WINDOWEVENT;
        fakeResizeEvent.window.windowID = 0; // deliberately not matching SDL_GetWindowID() so SDL doesn't purge event
        fakeResizeEvent.window.event = SDL_WINDOWEVENT_RESIZED;
        fakeResizeEvent.window.data1 = currentWidth;
        fakeResizeEvent.window.data2 = currentHeight;

        SDL_PushEvent(&fakeResizeEvent);
    }
#endif // __APPLE__

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

    if(SDL_GL_SetSwapInterval(vsync ? -1 : 0) == -1 && vsync)
        // no adaptive vsync; set regular vsync
        SDL_GL_SetSwapInterval(1);

    // init opengl context
    OpenGLGState::initContext();

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
