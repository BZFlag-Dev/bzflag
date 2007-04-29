/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* BeOS GUI backend by François Revol
 * "Dano" (GL2) implementation
 */

#include <Region.h>
#include <DirectGLWindow.h>
#include <Message.h>
#include <Point.h>
#include <View.h>
#include <WindowScreen.h> // for set_mouse_position()
#include <stdio.h>
#include <math.h>
#include "BeOSDisplay.h"
#include "BeOSWindow.h"
#include "BeOSVisual.h"
#include "BzfEvent.h"
#include "OpenGLGState.h"

#define MSGDBG(a) (printf a )


/****** MyDirectGLWindow ******/

class MyGLWindow : public BDirectGLWindow {
public:
  MyGLWindow(BeOSWindow		*beosWindow,
	     BRect		frame,
	     const char	*title,
	     window_type	type,
	     uint32		flags,
	     uint32		workspace = B_CURRENT_WORKSPACE);

  virtual bool QuitRequested(void);
  virtual void MessageReceived(BMessage *msg);
  virtual void FrameResized(float width, float height);
  virtual void DeviceInfo(uint32 device_id, uint32 monitor, const char *name, bool depth, bool stencil, bool accum);

  void PostBzfEvent(void);

private:
  BzfEvent		bzfEvent;
  BeOSWindow		*ref;
};

MyGLWindow::MyGLWindow(BeOSWindow *beosWindow, BRect frame, const char *title, window_type type, uint32 flags, uint32 workspace)
  : BDirectGLWindow(frame, title, type, flags, workspace)
{
  ref = beosWindow;
  bzfEvent.window = ref;
}

bool MyGLWindow::QuitRequested(void)
{
  bzfEvent.type = BzfEvent::Quit;
  PostBzfEvent();
  // Let the application close us.
  //  be_app->PostMessage(B_QUIT_REQUESTED);
  return false;
}

void MyGLWindow::MessageReceived(BMessage *msg)
{
  BPoint p;
  int32 raw_char;
  int32 buttons;
  int32 modifiers;
  //printf("MyGLWindow::MessageReceived()\n");
  //msg->PrintToStream();
  switch (msg->what) {
  case B_MOUSE_MOVED:
    msg->FindPoint("where", &p);
    bzfEvent.type = BzfEvent::MouseMove;
    bzfEvent.mouseMove.x = (int)p.x;
    bzfEvent.mouseMove.y = (int)p.y;
    break;
  case B_MOUSE_DOWN:
    msg->FindInt32("buttons", &buttons);
    msg->FindInt32("modifiers", &modifiers);
    bzfEvent.type = BzfEvent::KeyDown;
    bzfEvent.keyDown.ascii = 0;
    bzfEvent.keyDown.button = (buttons == B_PRIMARY_MOUSE_BUTTON)?(BzfKeyEvent::LeftMouse):(BzfKeyEvent::RightMouse);
    bzfEvent.keyDown.shift = 0;
    break;
  case B_MOUSE_UP:
    msg->FindInt32("buttons", &buttons);
    msg->FindInt32("modifiers", &modifiers);
    bzfEvent.type = BzfEvent::KeyUp;
    bzfEvent.keyUp.ascii = 0;
    bzfEvent.keyUp.button = (buttons == B_PRIMARY_MOUSE_BUTTON)?(BzfKeyEvent::LeftMouse):(BzfKeyEvent::RightMouse);
    bzfEvent.keyUp.shift = 0;
    break;
  case B_KEY_DOWN:
    {
      int byteslen;
      msg->FindInt32("modifiers", &modifiers);
      msg->FindInt32("raw_char", &raw_char);
      const char *bytes = msg->FindString("bytes");
      byteslen = strlen(bytes);
      bzfEvent.type = BzfEvent::KeyDown;
      bzfEvent.keyDown.button = BzfKeyEvent::NoButton;
      bzfEvent.keyDown.ascii = 0;
      bzfEvent.keyUp.shift = 0;
      MSGDBG(("### raw_char = 0x%08lx\n", raw_char));
      switch (raw_char) {
      case B_ESCAPE: bzfEvent.keyDown.ascii = '\033'; break;
      case B_RETURN: bzfEvent.keyDown.ascii = '\r'; break; // CR sux, LF rulz
      case B_SPACE: bzfEvent.keyDown.ascii = ' '; break;
      case B_HOME: bzfEvent.keyDown.button = BzfKeyEvent::Home; break;
      case B_END: bzfEvent.keyDown.button = BzfKeyEvent::End; break;
      case B_LEFT_ARROW: bzfEvent.keyDown.button = BzfKeyEvent::Left; break;
      case B_RIGHT_ARROW: bzfEvent.keyDown.button = BzfKeyEvent::Right; break;
      case B_UP_ARROW: bzfEvent.keyDown.button = BzfKeyEvent::Up; break;
      case B_DOWN_ARROW: bzfEvent.keyDown.button = BzfKeyEvent::Down; break;
      case B_PAGE_UP: bzfEvent.keyDown.button = BzfKeyEvent::PageUp; break;
      case B_PAGE_DOWN: bzfEvent.keyDown.button = BzfKeyEvent::PageDown; break;
      case B_INSERT: bzfEvent.keyDown.button = BzfKeyEvent::Insert; break;
      case B_DELETE: bzfEvent.keyDown.button = BzfKeyEvent::Delete; break;
      default:
	if (byteslen == 1) {
	  MSGDBG(("default char\n"));
	  bzfEvent.keyDown.ascii = bytes[0];
	} else if (byteslen == 2 && bytes[0] == B_FUNCTION_KEY) {
	  if (bytes[1] >= B_F1_KEY && bytes[1] < B_PRINT_KEY) {
	    bzfEvent.keyDown.button = BzfKeyEvent::F1 + bytes[1] - B_F1_KEY;
	  } else if (bytes[1] < B_PAUSE_KEY) {
	    bzfEvent.keyDown.button = BzfKeyEvent::Pause;
	  }
	} else {
	  BDirectGLWindow::MessageReceived(msg);
	  return;
	}
	break;
      }
    }
    break;
  default:
    BDirectGLWindow::MessageReceived(msg);
    return;
  }
  PostBzfEvent();
}

void MyGLWindow::FrameResized(float width, float height)
{
  bzfEvent.type = BzfEvent::Resize;
  bzfEvent.resize.width = (int)width;
  bzfEvent.resize.height = (int)height;
  PostBzfEvent();
  BDirectGLWindow::FrameResized(width, height);
}

void MyGLWindow::DeviceInfo(uint32 device_id, uint32 monitor, const char *name, bool depth, bool stencil, bool accum)
{
  if (device_id != BGL_DEVICE_SOFTWARE &&
      ref->openglDevice == BGL_DEVICE_SOFTWARE) {
    ref->openglDevice = device_id;

    printf("Using OpenGL device #%ld: %s\n", device_id, name);
  }
}

void MyGLWindow::PostBzfEvent(void)
{
  ((BeOSDisplay *)ref->display)->postBzfEvent(bzfEvent);
}

/****** BeOSWindow ******/

BeOSWindow::BeOSWindow(const BeOSDisplay* _display, const BeOSVisual* _visual) :
  BzfWindow(_display),
  display(_display),
  visual(_visual),
  openglDevice(BGL_DEVICE_SOFTWARE),
  oglContextInitialized(false),
  currentOglContext(-1)
{
  bWindow = new MyGLWindow(this, BRect(50, 50, 640+50, 480+50), "bzflag", B_TITLED_WINDOW,
			   B_OUTLINE_RESIZE | B_QUIT_ON_WINDOW_CLOSE | B_NOT_RESIZABLE | B_NOT_ZOOMABLE);
  if (!bWindow)
    return;
  utilView = new BView(BRect(0, 0, 0, 0), "utilview", B_FOLLOW_NONE, B_WILL_DRAW);
  if (!utilView)
    return;
  bWindow->AddChild(utilView); /* for getMouse() */
  makeContext();
  //	bWindow->ReleaseCurrent();
}

BeOSWindow::~BeOSWindow()
{
  if (!bWindow)
    return;
  bWindow->Lock();
  bWindow->Quit();
}

bool					BeOSWindow::isValid() const
{
  return (bWindow != NULL && utilView != NULL);
}

void					BeOSWindow::showWindow(bool on)
{
  MSGDBG(("BeOSWindow::showWindow(%s)\n", on?"true":"false"));
  //	bWindow->Lock();
  thread_id tid = find_thread(NULL);
  if (tid == currentOglContext)
    bWindow->ReleaseCurrent();

  if (on)
    bWindow->Show();
  else
    bWindow->Hide();

  if (tid == currentOglContext)
    bWindow->MakeCurrent();
  /*
    bWindow->Lock();
    bWindow->Sync();
    bWindow->Unlock();
  */
  MSGDBG(("< BeOSWindow::showWindow()\n"));
}

void					BeOSWindow::getPosition(int& x, int& y)
{
  BRect rect;
  bWindow->Lock();
  rect = bWindow->Frame();
  bWindow->Unlock();
  x = (int)rect.left;
  y = (int)rect.top;
}

void					BeOSWindow::getSize(int& width, int& height) const
{
  BRect rect;
  bWindow->Lock();
  rect = bWindow->Frame();
  bWindow->Unlock();
  width = (int)rect.right - (int)rect.left;
  height = (int)rect.bottom - (int)rect.top;
}

void					BeOSWindow::setTitle(const char* title)
{
  bWindow->Lock();
  bWindow->SetTitle(title);
  bWindow->Unlock();
}

void					BeOSWindow::setPosition(int x, int y)
{
  bWindow->Lock();
  bWindow->MoveTo((float)x, (float)y);
  bWindow->Unlock();
}

void					BeOSWindow::setSize(int width, int height)
{
  bWindow->Lock();
  bWindow->ResizeTo((float)width, (float)height);
  bWindow->Unlock();
}

void					BeOSWindow::setMinSize(int width, int height)
{
  float minW, maxW, minH, maxH;
  bWindow->Lock();
  bWindow->GetSizeLimits(&minW, &maxW, &minH, &maxH);
  bWindow->SetSizeLimits(width, maxW, height, maxH);
  bWindow->Unlock();
}

void					BeOSWindow::setFullscreen(bool on)
{
  bWindow->Lock();
  // FIXME
  bWindow->Unlock();
}

void					BeOSWindow::warpMouse(int x, int y)
{
  int px, py;
  getPosition(px, py);
  set_mouse_position(px+x, py+y);
}

void					BeOSWindow::getMouse(int& x, int& y) const
{
  BPoint point;
  uint32 buttons;
  if (utilView) {
    bWindow->Lock();
    utilView->GetMouse(&point, &buttons);
    bWindow->Unlock();
    x = (int)point.x;
    y = (int)point.y;
  }
}

void					BeOSWindow::grabMouse()
{
  // FIXME
}

void					BeOSWindow::ungrabMouse()
{
  // FIXME
}

void					BeOSWindow::showMouse()
{
  // FIXME
}

void					BeOSWindow::hideMouse()
{
  // FIXME
}

void					BeOSWindow::setGamma(float newGamma)
{
  // FIXME
}

float					BeOSWindow::getGamma() const
{
  return 1.0f;
}

bool					BeOSWindow::hasGammaControl() const
{
  // FIXME
  return false;
  //	return useColormap || hasGamma || has3DFXGamma;
}

void					BeOSWindow::makeCurrent()
{
  MSGDBG(("BeOSWindow::makeCurrent()\n"));
  if (!oglContextInitialized)
    return;
  if (bWindow != NULL) {
    thread_id tid = find_thread(NULL);
    //		bWindow->Lock();
    /* deadlocks... */
    //		bWindow->ReleaseCurrent();
    if (tid != currentOglContext) {
      MSGDBG(("bWindow->MakeCurrent()\n"));
      bWindow->MakeCurrent();
      currentOglContext = tid;
    }
    //		bWindow->Unlock();
  }
}


void					BeOSWindow::yieldCurrent()
{
  MSGDBG(("BeOSWindow::yieldCurrent()\n"));
  if (!oglContextInitialized)
    return;
  if (bWindow != NULL) {
    thread_id tid = find_thread(NULL);
    if (tid == currentOglContext) {
      MSGDBG(("bWindow->YieldCurrent()\n"));
      //			bWindow->YieldCurrent();

      bWindow->ReleaseCurrent();
      snooze(10000);
      bWindow->MakeCurrent();
    }
  }
}

void					BeOSWindow::releaseCurrent()
{
  MSGDBG(("BeOSWindow::releaseCurrent()\n"));
  if (!oglContextInitialized)
    return;
  if (bWindow != NULL) {
    thread_id tid = find_thread(NULL);
    if (tid == currentOglContext) {
      MSGDBG(("bWindow->ReleaseCurrent()\n"));
      bWindow->ReleaseCurrent();
      currentOglContext = -1;
    }
  }
}

void					BeOSWindow::swapBuffers()
{
  if (bWindow != NULL && oglContextInitialized) {
    bWindow->SwapBuffers();
  }
}

void					BeOSWindow::makeContext()
{
  MSGDBG(("BeOSWindow::makeContext()\n"));
  if (oglContextInitialized)
    return;
  uint32 minColor = BGL_ANY | (visual->doubleBuffer?BGL_DOUBLE:BGL_SINGLE);
  if (bWindow == NULL)
    return;
  bWindow->Lock();

  bWindow->EnumerateDevices( BGL_MONITOR_PRIMARY, minColor, BGL_ANY, BGL_NONE, BGL_NONE );
  bWindow->InitializeGL( openglDevice, minColor, BGL_ANY, BGL_NONE, BGL_NONE );

  //	bWindow->SaveDebuggingInfo("/boot/home/bzf_ogl_debug_log.txt");

  bWindow->Unlock();

  oglContextInitialized = true;
  //showWindow(true);
  makeCurrent();
  //	OpenGLGState::init();
  OpenGLGState::initContext();
}

void					BeOSWindow::freeContext()
{
  MSGDBG(("BeOSWindow::freeContext()\n"));
  // release context data
  //	OpenGLGState::freeContext();
  //bWindow->ReleaseCurrent();
  releaseCurrent();
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
