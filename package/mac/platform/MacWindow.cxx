#include "MacWindow.h"

#ifndef USE_DSP
  #define GLOBAL_OFFSET_X 0
  #define GLOBAL_OFFSET_Y 20
#else
  #define GLOBAL_OFFSET_X 0
  #define GLOBAL_OFFSET_Y 0
#endif

//Point gMousePosition;
bool  gMouseGrabbed;

MacWindow::MacWindow (const MacDisplay *display, MacVisual *visual) :
			BzfWindow (display)
{
  GLboolean ok;

 // Force 640x480 for now
#ifndef USE_DSP
   SetRect (&rect, GLOBAL_OFFSET_X, GLOBAL_OFFSET_Y, 640 + GLOBAL_OFFSET_X, 480 + GLOBAL_OFFSET_Y);
   window = NewCWindow (NULL, &rect, "\pBZFlag", false,
								   plainDBox, (WindowPtr) -1L, true, 0L);
#else
	window = dsSetupScreen (640, 480 );
#endif

	if (!window) DebugStr  ("\pCould not create a window.");

#ifdef USE_DSP
	display->setWindow (window);
#endif

	visual->reset ();
	visual->setLevel(0);
visual->setIndex(24);
//	visual->setDepth (24);
	visual->setDoubleBuffer (true);
	visual->setRGBA(1, 1, 1, 0);
//	visual->setStencil(1);

//	visual->addAttribute1(AGL_FULLSCREEN);

	visual->build ();

	context = aglCreateContext (visual->get(), NULL);

  if (!context) DebugStr ("\pCould not create an OpenGL context.");

  display->setContext    (context);

  makeCurrent ();

// 3dfx routine


//context = aglCreateContext (visual->get (), NULL);

// ok = aglSetFullScreen (context, 640, 480, 85, 0);


  ok = aglSetDrawable (context, (AGLDrawable)window);

  if (!ok) DebugStr ("\paglSetDrawable failed.");

  ok = aglSetCurrentContext (context);

  if (!ok) DebugStr ("\paglSetCurrent failed.");

}

MacWindow::~MacWindow () {

  if (window != NULL)

#ifndef USE_DSP
	DisposeWindow (window);
#else
  dsShutdownScreen ((CGrafPtr)window);
#endif
}

boolean MacWindow::isValid () const { return window != NULL; }

void MacWindow::showWindow (boolean show) {

#ifndef USE_DSP
    if (window == NULL)  return;

       if (show)
	ShowWindow (window);
       else
	HideWindow (window);
#endif
}

void MacWindow::getPosition (int &x, int &y) { x = 0, y = 0; }

void MacWindow::getSize     (int &width, int &height) const {

  if (window == NULL) return;

  width  = window->portRect.right  - window->portRect.left;
  height = window->portRect.bottom - window->portRect.top;
}

void MacWindow::setTitle (const char *title) {

#ifndef USE_DSP
  if (window == NULL) return;

  setwtitle (window, title);
#endif

}

void MacWindow::setPosition (int x, int y) {

#ifndef USE_DSP
  if (window == NULL) return;

  MoveWindow (window, x, y, true);

  GLboolean ok = aglSetDrawable (context, (AGLDrawable) window);

  if (!ok) DebugStr ("\paglSetDrawable failed.");
#endif
}

void MacWindow::setSize (int width, int height) {
#ifndef USE_DSP
  if (window == NULL) return;

  SizeWindow (window, width, height, true);

  GLboolean ok = aglSetDrawable (context, (AGLDrawable) window);

  if (!ok) DebugStr ("\paglSetDrawable failed.");
#endif
}

void MacWindow::setMinSize (int width, int height) {
#ifndef USE_DSP
  if (window == NULL) return;

  // Not sure what this wants, leave out for now.
  //CollapseWindow (window, true);
#endif
}

void MacWindow::setFullscreen () {} // do nothing for now
void MacWindow::warpMouse   (int x, int y) {

 Point moveTo;
 GrafPtr savePort;

 GetPort (&savePort);
 SetPort (window);

 moveTo.h = x;
 moveTo.v = y;

 LocalToGlobal (&moveTo);

 LMSetMouseTemp(moveTo);
 LMSetRawMouseLocation(moveTo);
 LMSetCursorNew(1);

 SetPort (savePort);
}

void MacWindow::getMouse    (int &x, int &y) const {

    Point   tmpMouse;
    GrafPtr savedPort;

    GetPort(&savedPort);
    SetPort(window);
    GetMouse (&tmpMouse);  // returns mouse location in coords of local
			  // window which must be set

    x = tmpMouse.h;
    y = tmpMouse.v;
    SetPort(savedPort);
}

void MacWindow::grabMouse   () { gMouseGrabbed = true;  }
void MacWindow::ungrabMouse () { gMouseGrabbed = false; }
void MacWindow::showMouse   () { /*ShowCursor ();*/ }
void MacWindow::hideMouse   () { /*HideCursor ();*/ }

void	  MacWindow::setGamma(float)	 {}
float	  MacWindow::getGamma()	const { return 0.0;   }
boolean	MacWindow::hasGammaControl() const { return false; }

void MacWindow::makeContext() {}
void MacWindow::freeContext() {}
void MacWindow::makeCurrent () {

//#ifndef USE_DSP
 // This function is pointless in fullscreen mode

  if (window == NULL) return;

// GLboolean ok = aglSetDrawable (context, (AGLDrawable) window);

// if (!ok) DebugStr ("\paglSetDrawable failed.");

 //if (context == aglGetCurrentContext ());
 // return;

 GLboolean ok = aglSetCurrentContext (context);

 if (!ok) DebugStr ("\paglSetCurrentContext failed.");
//#endif
}

void MacWindow::swapBuffers () {

  aglSwapBuffers (context);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

