/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"
#include "global.h"
#include "MainWindow.h"
#include "BzfWindow.h"
#include "SceneRenderer.h"
#include "bzfgl.h"

//
// MainWindow
//

MainWindow::MainWindow(BzfWindow* _window) :
				window(_window),
				quit(false),
				quadrant(FullWindow),
				isFullscreen(false),
				isFullView(true),
				allowMouseGrab(true),
				zoomFactor(1),
				width(0),
				minWidth(MinX),
				minHeight(MinY)
{
  window->addResizeCallback(resizeCB, this);
  resize();
}

MainWindow::~MainWindow()
{
  window->removeResizeCallback(resizeCB, this);
}

void			MainWindow::setZoomFactor(int _zoomFactor)
{
  zoomFactor = _zoomFactor;
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

void			MainWindow::setSize(int width, int height)
{
  window->setSize(width, height);
  resize();
}

void			MainWindow::showWindow(bool on)
{
  window->showWindow(on);
  if (on) resize();
}

void			MainWindow::warpMouse()
{
  // move mouse to center of view window (zero motion box)
  int y = viewHeight >> 1;
  if (quadrant != FullWindow) y += ((trueHeight+1) >> 1) - yOrigin;
  window->warpMouse((width >> 1) + xOrigin, y);
}

void			MainWindow::getMousePosition(int& mx, int& my) const
{
  window->getMouse(mx, my);
  mx -= (width >> 1) + xOrigin;
  my -= (viewHeight >> 1);
  if (quadrant != FullWindow) my -= ((trueHeight+1) >> 1) - yOrigin;
}

void			MainWindow::grabMouse()
{
  if (allowMouseGrab) window->grabMouse();
}

void			MainWindow::ungrabMouse()
{
  if (allowMouseGrab) window->ungrabMouse();
}

bool			MainWindow::getFullscreen()
{
  return isFullscreen;
}

void			MainWindow::setFullscreen()
{
  isFullscreen = true;
  window->setFullscreen();
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
      if (isFullView)
	viewHeight = height;
      else
	viewHeight = inHeight * (46 - SceneRenderer::getInstance()->getRadarSize()) / 60;
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
    width = inWidth / zoomFactor + 1;
    height = inHeight / zoomFactor + 1;
  }

  glViewport(xOrigin, yOrigin, width, height);
}

void			MainWindow::resize()
{
  window->getSize(trueWidth, trueHeight);
  window->makeCurrent();
  setQuadrant(quadrant);
}

void			MainWindow::resizeCB(void* _self)
{
  MainWindow* self = (MainWindow*)_self;
  self->resize();
}


bool			MainWindow::joystick() const
{
  return window->joystick();
}

void			MainWindow::getJoyPosition(int& mx, int& my) const
{
  window->getJoy(mx, my);
  mx = ((width >> 1)*mx)/(900);
  my = ((height >> 1)*my)/(900);
}

unsigned long                  MainWindow::getJoyButtonSet() const
{
  return window->getJoyButtons();
}

void                    MainWindow::getJoyDevices(std::vector<std::string>
						  &list) const
{
  window->getJoyDevices(list);
}

void	                MainWindow::initJoystick(std::string &joystickName) {
  window->initJoystick(joystickName.c_str());  
};

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

