/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* BeOSWindow:
 *	Encapsulates a BeOS GL window
 */

#ifndef BEOS_WINDOW_H
#define BEOS_WINDOW_H

#include "common.h"
#include "BzfWindow.h"

class BeOSDisplay;
class BeOSVisual;
#ifdef BEOS_USE_GL2
class MyGLWindow;
#endif
class BView;

class BeOSWindow : public BzfWindow {
public:
  BeOSWindow(const BeOSDisplay*, const BeOSVisual*);
  virtual ~BeOSWindow();

  virtual bool		isValid() const;

  virtual void		showWindow(bool);

  virtual void		getPosition(int& x, int& y);
  virtual void		getSize(int& width, int& height) const;

  virtual void		setTitle(const char*);
  virtual void		setPosition(int x, int y);
  virtual void		setSize(int width, int height);
  virtual void		setMinSize(int width, int height);
  virtual void		setFullscreen();

  virtual void		warpMouse(int x, int y);
  virtual void		getMouse(int& x, int& y) const;
  virtual void		grabMouse();
  virtual void		ungrabMouse();
  virtual void		showMouse();
  virtual void		hideMouse();

  virtual void		setGamma(float);
  virtual float		getGamma() const;
  virtual bool		hasGammaControl() const;

  virtual void		makeCurrent();
  virtual void		yieldCurrent();
  virtual void		releaseCurrent();
  virtual void		swapBuffers();
  virtual void		makeContext();
  virtual void		freeContext();

  /* other methods */

  //  bool				applyVisual(BeOSVisual* _visual);

private:
  friend class BeOSDisplay;
  //#ifdef BEOS_USE_GL2
  friend class MyGLWindow;
  MyGLWindow				*bWindow;
  //#else
  //  BGLView				*glView;
  //#endif
  const BeOSDisplay			*display;
  const BeOSVisual			*visual;
  BView					*utilView;
  uint32				openglDevice;
  bool					oglContextInitialized;
  thread_id				currentOglContext;
};

#endif // BEOS_WINDOW_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

