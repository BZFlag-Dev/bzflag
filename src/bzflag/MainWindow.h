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

#ifndef	__MAINWINDOW_H__
#define	__MAINWINDOW_H__

/* BZFlag common header */
#include "common.h"

/* system interface headers */
#include <vector>
#include <string>

/* common interface headers */
#include "BzfWindow.h"
#include "BzfJoystick.h"

#define	USE_GL_STEREO

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

			MainWindow(BzfWindow*, BzfJoystick*);
			~MainWindow();

    BzfWindow*		getWindow() const { return window; }
    BzfJoystick*	getJoystick() const { return joystick; }

    int			getOriginX() const;
    int			getOriginY() const;
    int			getWidth() const;
    int			getHeight() const;
    int			getViewHeight() const;
    bool		getFullscreen() const;
    bool		getFullView() const;

    void		setPosition(int x, int y);
    void		setSize(int width, int height);
    void		setMinSize(int width, int height);
    void		setFullView(bool);
    void		setFullscreen();
    void		toggleFullscreen();
    void		iconify(void);
    void		setNoMouseGrab();

    void		setQuadrant(Quadrant);

    void		showWindow(bool = true);
    void		warpMouse();
    void		warpMouse(int mx, int my);
    void		warpMouseCenterX();
    void		warpMouseCenterY();
    void		disableConfineToMotionbox();
    void		confineToMotionbox(int x1, int y1, int x2, int y2);
    void		grabMouse();
    void		enableGrabMouse(bool on);
    bool		isGrabEnabled(void) const;
    void		ungrabMouse();

    void		resize();

    // return true iff there's a joystick available (and it's been initialized)
    bool		haveJoystick() const;

    // FIXME -- try to get rid of these.  we'd like to receive
    // events instead because it means no round trip to the server
    // for these values that we need every frame.
    void		getMousePosition(int& mx, int& my) const;
    void		getJoyPosition(int& mx, int& my) const;
    int			getNumHats() const;
    void		getJoyHat(int hat, float &hatX, float &hatY) const;
    unsigned long	getJoyButtonSet() const;
    void		getJoyDevices(std::vector<std::string> &list) const;
    void		getJoyDeviceAxes(std::vector<std::string> &list) const;
    void		setJoyXAxis(const std::string &axis);
    void		setJoyYAxis(const std::string &axis);
    void		initJoystick(std::string &joystickName);

    bool		isInFault() const { return faulting; };

  private:
    // no copying
			MainWindow(const MainWindow&);
    MainWindow&		operator=(const MainWindow&);

    static void		resizeCB(void*);

    inline int		getYOffset() const;

  private:
    BzfWindow*		window;
    BzfJoystick*	joystick;
    bool		quit;
    Quadrant		quadrant;
    bool		isFullscreen;
    bool		isFullView;
    bool		allowMouseGrab;
    bool		grabEnabled;
    int			trueWidth, trueHeight;
    int			xOrigin, yOrigin;
    int			width;
    int			height;
    int			viewHeight;
    int			minWidth;
    int			minHeight;
    bool		faulting;
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

#endif /* __MAINWINDOW_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
