/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 *
 */

#ifndef	BZF_MAIN_WINDOW_H
#define	BZF_MAIN_WINDOW_H

#define	USE_GL_STEREO

#include "common.h"

class BzfWindow;

class MainWindow {
  public:
    enum Quadrant {
			FullWindow,
			UpperLeft,
			UpperRight,
			LowerLeft,
			LowerRight,
			ZoomRegion,
			UpperHalf,
			LowerHalf
    };

			MainWindow(BzfWindow*);
			~MainWindow();

    BzfWindow*		getWindow() const { return window; }

    int			getOriginX() const;
    int			getOriginY() const;
    int			getWidth() const;
    int			getHeight() const;
    int			getViewHeight() const;
    bool		getFullscreen();
    bool		getFullView() const;

    void		setPosition(int x, int y);
    void		setSize(int width, int height);
    void		setMinSize(int width, int height);
    void		setFullView(bool);
    void		setFullscreen();
    void		setNoMouseGrab();

    void		setZoomFactor(int);
    void		setQuadrant(Quadrant);

    void		showWindow(bool = true);
    void		warpMouse();
    void		grabMouse();
    void		ungrabMouse();

    void		resize();

    // return true iff there's a joystick available (and it's been initialized)
    bool		joystick() const;

    // FIXME -- try to get rid of these.  we'd like to receive
    // events instead because it means no round trip to the server
    // for these values that we need every frame.
    void		getMousePosition(int& mx, int& my) const;
    void		getJoyPosition(int& mx, int& my) const;
    unsigned long	getJoyButtonSet() const;

  private:
    // no copying
			MainWindow(const MainWindow&);
    MainWindow&		operator=(const MainWindow&);

    static void		resizeCB(void*);

  private:
    BzfWindow*		window;
    bool		quit;
    Quadrant		quadrant;
    bool		isFullscreen;
    bool		isFullView;
    bool		allowMouseGrab;
    int			zoomFactor;
    int			trueWidth, trueHeight;
    int			xOrigin, yOrigin;
    int			width;
    int			height;
    int			viewHeight;
    int			minWidth;
    int			minHeight;
};

//
// MainWindow
//

inline int		MainWindow::getOriginX() const
{
  return xOrigin;
}

inline int		MainWindow::getOriginY() const
{
  return yOrigin;
}

inline int		MainWindow::getWidth() const
{
  return width;
}

inline int		MainWindow::getHeight() const
{
  return height;
}

inline int		MainWindow::getViewHeight() const
{
  return viewHeight;
}

inline bool		MainWindow::getFullView() const
{
  return isFullView;
}

#endif // BZF_MAIN_WINDOW_H
// ex: shiftwidth=2 tabstop=8
