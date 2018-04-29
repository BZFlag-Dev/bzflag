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


/* interface headers */
#include "MainWindow.h"

/* common implementation headers */
#include "global.h"
#include "SceneRenderer.h"

//
// MainWindow
//

MainWindow::MainWindow(BzfWindow* _window, BzfJoystick* _joystick) :
				window(_window),
				joystick(_joystick),
				quit(false),
				quadrant(FullWindow),
				isFullscreen(false),
				isFullView(true),
				allowMouseGrab(true),
				grabEnabled(true),
				width(0),
				minWidth(MinX),
				minHeight(MinY),
				faulting(false)
{
  window->addResizeCallback(resizeCB, this);
  resize();
}

MainWindow::~MainWindow()
{
  window->removeResizeCallback(resizeCB, this);
}

void			MainWindow::setMinSize(int _minWidth, int _minHeight)
{
  minWidth = _minWidth;
  minHeight = _minHeight;
  window->setMinSize(minWidth, minHeight);
  resize();
}

void			MainWindow::setPosition(int x, int y)
{
  window->setPosition(x, y);
}

void			MainWindow::setSize(int _width, int _height)
{
  window->setSize(_width, _height);
  resize();
}

void			MainWindow::showWindow(bool on)
{
  window->showWindow(on);
  if (on) resize();
}


inline int MainWindow::getYOffset() const
{
  if (quadrant == FullWindow) {
    return 0;
  }
  return ((trueHeight + 1) >> 1) - yOrigin;
}

void			MainWindow::warpMouse()
{
  // move mouse to center of view window (zero motion box)
  window->warpMouse((width >> 1) + xOrigin, (viewHeight >> 1) + getYOffset());
}

void			MainWindow::warpMouse(int mx, int my)
{
  window->warpMouse(mx, my);
}

void			MainWindow::warpMouseCenterX()
{
  int mx, my;
  getMousePosition(mx, my);
  window->warpMouse((width >> 1) + xOrigin, my + (viewHeight >> 1) + getYOffset());
}

void			MainWindow::warpMouseCenterY()
{
  int mx, my;
  getMousePosition(mx, my);
  const int y = (viewHeight >> 1) + getYOffset();
  window->warpMouse(mx + (width >> 1) + xOrigin, y);
}

void			MainWindow::getMousePosition(int& mx, int& my) const
{
  window->getMouse(mx, my);
  mx -= (width >> 1) + xOrigin;
  my -= (viewHeight >> 1) + getYOffset();
}

void			MainWindow::disableConfineToMotionbox()
{
  window->disableConfineToMotionbox();
}

void			MainWindow::confineToMotionbox(int x1, int y1, int x2, int y2)
{
  window->confineToMotionbox(x1, y1, x2, y2);
}

void			MainWindow::grabMouse()
{
  if (allowMouseGrab) window->grabMouse();
}

void			MainWindow::ungrabMouse()
{
  if (allowMouseGrab) window->ungrabMouse();
}

void			MainWindow::enableGrabMouse(bool on)
{
  window->enableGrabMouse(on);
  grabEnabled = on;
}

bool			MainWindow::isGrabEnabled(void) const
{
  return grabEnabled;
}

bool			MainWindow::getFullscreen() const
{
  return isFullscreen;
}

void			MainWindow::setFullscreen()
{
  isFullscreen = false;
  toggleFullscreen();
}

void			MainWindow::toggleFullscreen()
{
  isFullscreen = !isFullscreen;
  window->setFullscreen(isFullscreen);
  resize();
}

void			MainWindow::setFullView(bool _isFullView)
{
  isFullView = _isFullView;
}

void			MainWindow::setNoMouseGrab()
{
  allowMouseGrab = false;
}

void			MainWindow::setQuadrant(Quadrant _quadrant)
{
  int inWidth = trueWidth;
  if (inWidth < MinX) inWidth = MinX;
  int inHeight = trueHeight;
  if (inHeight < MinY) inHeight = MinY;

  quadrant = _quadrant;
  switch (quadrant) {
    default:
    case FullWindow:
      width = inWidth;
      height = inHeight;
      if (isFullView) {
	viewHeight = height;
      } else {
	viewHeight = inHeight * (46 - RENDERER.getRadarSize()) / 60;
      }
      xOrigin = 0;
      yOrigin = 0;
      break;
    case UpperLeft:
      width = inWidth >> 1;
      height = inHeight >> 1;
      viewHeight = height;
      xOrigin = 0;
      yOrigin = (inHeight+1) >> 1;
      break;
    case UpperRight:
      width = (inWidth+1) >> 1;
      height = inHeight >> 1;
      viewHeight = height;
      xOrigin = inWidth >> 1;
      yOrigin = (inHeight+1) >> 1;
      break;
    case LowerLeft:
      width = inWidth >> 1;
      height = (inHeight+1) >> 1;
      viewHeight = height;
      xOrigin = 0;
      yOrigin = 0;
      break;
    case LowerRight:
      width = (inWidth+1) >> 1;
      height = (inHeight+1) >> 1;
      viewHeight = height;
      xOrigin = inWidth >> 1;
      yOrigin = 0;
      break;
    case UpperHalf:
      width = inWidth;
      height = inHeight >> 1;
      viewHeight = height;
      xOrigin = 0;
      yOrigin = (inHeight+1) >> 1;
      break;
    case LowerHalf:
      width = inWidth;
      height = inHeight >> 1;
      viewHeight = height;
      xOrigin = 0;
      yOrigin = 0;
      break;
    case ZoomRegion:
      width = inWidth;
      height = inHeight;
      viewHeight = height;
      xOrigin = 0;
      yOrigin = 0;
      break;
  }

  if (quadrant == ZoomRegion) {
    width = inWidth + 1;
    height = inHeight + 1;
  }

  glViewport(xOrigin, yOrigin, width, height);
}

void			MainWindow::resize()
{
  window->getSize(trueWidth, trueHeight);
  window->makeCurrent();
  if (!window->create())
    faulting = true;
  setQuadrant(quadrant);
}

void			MainWindow::resizeCB(void* _self)
{
  MainWindow* self = (MainWindow*)_self;
  self->resize();
}

void			MainWindow::iconify()
{
  window->iconify();
}


bool			MainWindow::haveJoystick() const
{
  return joystick->joystick();
}

void			MainWindow::getJoyPosition(int& mx, int& my) const
{
  // joystick axes inversion values
  // 0: no inversion
  // 1: invert X
  // 2: invert Y
  // 3: invert both
  joystick->getJoy(mx, my);
  mx = ((width >> 1) * mx * (BZDB.evalInt("jsInvertAxes") % 2 == 1 ? -1 : 1)) / (900);
  my = ((height >> 1) * my * (BZDB.evalInt("jsInvertAxes") > 1 ? -1 : 1)) / (900);
}

int			MainWindow::getNumHats() const
{
  return joystick->getNumHats();
}

void			MainWindow::getJoyHat(int hat, float &hatX, float &hatY) const
{
  joystick->getJoyHat(hat, hatX, hatY);
}

unsigned long		MainWindow::getJoyButtonSet() const
{
  return joystick->getJoyButtons();
}

void			MainWindow::getJoyDevices(std::vector<std::string>
						  &list) const
{
  joystick->getJoyDevices(list);
}

void			MainWindow::getJoyDeviceAxes(std::vector<std::string>
						 &list) const
{
  joystick->getJoyDeviceAxes(list);
}

void			MainWindow::setJoyXAxis(const std::string &axis)
{
  joystick->setXAxis(axis);
}

void			MainWindow::setJoyYAxis(const std::string &axis)
{
  joystick->setYAxis(axis);
}

void			MainWindow::initJoystick(std::string &joystickName) {
  joystick->initJoystick(joystickName.c_str());
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
