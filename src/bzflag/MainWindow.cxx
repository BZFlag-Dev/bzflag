/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "MainWindow.h"
#include "BzfWindow.h"
#include "bzfgl.h"

//
// MainWindow
//

MainWindow::MainWindow(BzfWindow* _window) :
				window(_window),
				quit(False),
				quadrant(FullWindow),
				isFullscreen(False),
				allowMouseGrab(True),
				zoomFactor(1),
				width(0),
				minWidth(256),
				minHeight(192)
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

void			MainWindow::setQuit()
{
  quit = True;
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

void			MainWindow::showWindow(boolean on)
{
  window->showWindow(on);
  if (on) resize();
}

void			MainWindow::warpMouse()
{
  // move mouse to center of view window (zero motion box)
  int y = height >> 1;
  if (quadrant != FullWindow) y += ((trueHeight+1) >> 1) - yOrigin;
  window->warpMouse((width >> 1) + xOrigin, y);
}

void			MainWindow::getMousePosition(int& mx, int& my) const
{
  window->getMouse(mx, my);
  mx -= (width >> 1) + xOrigin;
  my -= (height >> 1);
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

boolean			MainWindow::getFullscreen()
{
  return isFullscreen;
}

void			MainWindow::setFullscreen()
{
  isFullscreen = True;
  window->setFullscreen();
  resize();
}

void			MainWindow::setNoMouseGrab()
{
  allowMouseGrab = False;
}

void			MainWindow::setQuadrant(Quadrant _quadrant)
{
  int inWidth = trueWidth;
  if (inWidth < 256) inWidth = 256;
  int inHeight = trueHeight;
  if (inHeight < 192) inHeight = 192;

  quadrant = _quadrant;
  switch (quadrant) {
    default:
    case FullWindow:
      width = inWidth;
      height = inHeight;
      xOrigin = 0;
      yOrigin = 0;
      break;
    case UpperLeft:
      width = inWidth >> 1;
      height = inHeight >> 1;
      xOrigin = 0;
      yOrigin = (inHeight+1) >> 1;
      break;
    case UpperRight:
      width = (inWidth+1) >> 1;
      height = inHeight >> 1;
      xOrigin = inWidth >> 1;
      yOrigin = (inHeight+1) >> 1;
      break;
    case LowerLeft:
      width = inWidth >> 1;
      height = (inHeight+1) >> 1;
      xOrigin = 0;
      yOrigin = 0;
      break;
    case LowerRight:
      width = (inWidth+1) >> 1;
      height = (inHeight+1) >> 1;
      xOrigin = inWidth >> 1;
      yOrigin = 0;
      break;
    case UpperHalf:
      width = inWidth;
      height = inHeight >> 1;
      xOrigin = 0;
      yOrigin = (inHeight+1) >> 1;
      break;
    case LowerHalf:
      width = inWidth;
      height = inHeight >> 1;
      xOrigin = 0;
      yOrigin = 0;
      break;
    case ZoomRegion:
      width = inWidth;
      height = inHeight;
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


boolean			MainWindow::joystick() const
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

// ex: shiftwidth=2 tabstop=8
