/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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

MainWindow::MainWindow(BzfWindow *_window) :
				window(_window),
				quit(false),
				quadrant(FullWindow),
				isFullscreen(false),
				isFullView(true),
				allowMouseGrab(true),
				grabEnabled(true),
				zoomFactor(1),
				width(0),
				minWidth(MinX),
				minHeight(MinY)
{
  window->addResizeCallback(resizeCB, this);
  resize();

  // create & initialize the joystick
  if (BZDB.get("joystickname") == "off" || BZDB.get("joystickname") == "") {
    joystickNumber = 255;
  } else {
    joystickNumber = atoi(BZDB.get("joystickname").c_str());
    joystickXAxis  = atoi(BZDB.get("jsXAxis").c_str());
    joystickYAxis  = atoi(BZDB.get("jsYAxis").c_str());
  }

  joy = csQueryRegistry<iJoystickDriver>
    (csApplicationFramework::GetObjectRegistry());
  if (!joy)
    csApplicationFramework::ReportError("Failed to locate Joystick Driver!\n");

  mouse = csQueryRegistry<iMouseDriver>
    (csApplicationFramework::GetObjectRegistry());
  if (!mouse)
    csApplicationFramework::ReportError("Failed to locate Mouse Driver!\n");

  kbd = csQueryRegistry<iKeyboardDriver>
    (csApplicationFramework::GetObjectRegistry());
  if (!kbd)
    csApplicationFramework::ReportError("Failed to locate Keyboard Driver!");
  
  g3d = csQueryRegistry<iGraphics3D>
    (csApplicationFramework::GetObjectRegistry());
  if (!g3d)
    csApplicationFramework::ReportError("Failed to locate 3D Graphic Driver!");

  g2d = g3d->GetDriver2D();
  if (!g2d)
    csApplicationFramework::ReportError("Failed to locate 2D Graphic Driver!");

  hasGamma = g2d->SetGamma(g2d->GetGamma());
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

void			MainWindow::warpMouse()
{
  // move mouse to center of view window (zero motion box)
  int y = viewHeight >> 1;
  if (quadrant != FullWindow) y += ((trueHeight+1) >> 1) - yOrigin;
  g2d->SetMousePosition(width >> 1, height >> 1);
}

void			MainWindow::getMousePosition(int& mx, int& my) const
{
  mx = mouse->GetLastX();
  my = mouse->GetLastY();
  mx -= (width >> 1);
  my -= (viewHeight >> 1);
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

bool			MainWindow::isGrabEnabled(void)
{
  return grabEnabled;
}

bool			MainWindow::getFullscreen()
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
    width = inWidth / zoomFactor + 1;
    height = inHeight / zoomFactor + 1;
  }
}

void			MainWindow::resize()
{
  csRef<iGraphics3D> g3d
    = CS_QUERY_REGISTRY(csApplicationFramework::GetObjectRegistry(),
			iGraphics3D);
  trueWidth  = g3d->GetWidth();
  trueHeight = g3d->GetHeight();
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
  return joy;
}

void			MainWindow::getJoyPosition(int& mx, int& my) const
{
  mx = 0;
  my = 0;

  if (joystickNumber == 255)
    return;

  if (!joy)
    return;

  mx = joy->GetLast(joystickNumber, joystickXAxis);
  my = joy->GetLast(joystickNumber, joystickYAxis);

  mx = ((width >> 1)  * mx) / 900;
  my = ((height >> 1) * my) / 900;
}

unsigned long		  MainWindow::getJoyButtonSet() const
{
  unsigned long buttons = 0;

  if (joystickNumber == 255)
    return 0;

  if (!joy)
    return 0;

  for (int i = 0; i < CS_MAX_JOYSTICK_BUTTONS; i++)
    buttons |= joy->GetLastButton(joystickNumber, i) << i;

  return buttons;
}

void		    MainWindow::getJoyDevices(std::vector<std::string>
						  &list) const
{
  list.erase(list.begin(), list.end());
  for (int i = 0; i < (int)CS_MAX_JOYSTICK_COUNT; i++) {
    char joystickDevice[3];
    sprintf(joystickDevice, "%d", i);
    list.push_back(joystickDevice);
  }
}

void		    MainWindow::getJoyDeviceAxes(std::vector<std::string>
						 &list) const
{
  for (int i = 0; i < CS_MAX_JOYSTICK_AXES; i++) {
    char joystickAxes[3];
    sprintf(joystickAxes, "%d", i);
    list.push_back(joystickAxes);
  }
}

void		    MainWindow::setJoyXAxis(const std::string axis)
{
  joystickXAxis = atoi(axis.c_str());
}

void		    MainWindow::setJoyYAxis(const std::string axis)
{
  joystickYAxis = atoi(axis.c_str());
}

void			MainWindow::initJoystick(std::string &joystickName) {
  if (!strcasecmp(joystickName.c_str(), "off")
      || !strcmp(joystickName.c_str(), ""))
    joystickNumber = 255;
  else
    joystickNumber = atoi(joystickName.c_str());
}

void MainWindow::getModState(bool &shift, bool &ctrl, bool &alt)
{
  shift = (kbd->GetModifierState(CSKEY_SHIFT) != 0);
  ctrl  = (kbd->GetModifierState(CSKEY_CTRL) != 0);
  alt   = (kbd->GetModifierState(CSKEY_ALT) != 0);
}

bool MainWindow::hasGammaControl() {
  return hasGamma;
}

float MainWindow::getGamma() {
  return g2d->GetGamma();
}

void MainWindow::setGamma(float gamma) {
  g2d->SetGamma(gamma);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

