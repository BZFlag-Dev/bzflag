/* bzflag
 * Copyright (c) 1993 - 2000 Tim Riker
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
				panelRatio(0.2f),
				isFullscreen(False),
				allowMouseGrab(True),
				zoomFactor(1),
				width(0),
				panelHeight(0),
				viewHeight(0),
				minWidth(300),
				minPanelHeight(120),
				minViewHeight(120)
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

void			MainWindow::setMinSize(int _minWidth,
				int _minPanelHeight, int _minViewHeight)
{
  minWidth = _minWidth;
  minPanelHeight = _minPanelHeight;
  minViewHeight = _minViewHeight;
  window->setMinSize(minWidth, minPanelHeight + minViewHeight);
  resize();
  if (panelHeight < minPanelHeight) {
    panelHeight = minPanelHeight;
    viewHeight = trueHeight - panelHeight;
    if (viewHeight < _minViewHeight)
      viewHeight = _minViewHeight;
  }
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

void			MainWindow::setPanelRatio(float _panelRatio)
{
  panelRatio = _panelRatio;
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
  if (inWidth < 600) inWidth = 600;
  int inHeight = trueHeight;
  if (inHeight < 400) inHeight = 400;

  quadrant = _quadrant;
  int height, minPanelHeight2 = minPanelHeight >> 1;
  switch (quadrant) {
    default:
    case FullWindow:
      width = inWidth;
      height = inHeight;
      xOrigin = 0;
      yOrigin = 0;
      minPanelHeight2 = minPanelHeight;
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
      minPanelHeight2 = minPanelHeight;
      break;
  }

  panelHeight = (int)((float)width * panelRatio + 0.5f);
  if (panelHeight < minPanelHeight2)
    panelHeight = minPanelHeight2;
  else if (panelHeight > (int)(0.333f * height))
    panelHeight = (int)(0.333f * height);
  viewHeight = height - panelHeight;
  if (viewHeight < 0) {
    panelHeight = height;
    viewHeight = 0;
  }
  if (quadrant == ZoomRegion) {
    width = inWidth / zoomFactor + 1;
    viewHeight = viewHeight / zoomFactor + 1;
  }

  glViewport(xOrigin, yOrigin, width, panelHeight + viewHeight);
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
