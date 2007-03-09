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

#ifndef	BZF_BZFLAG_H
#define	BZF_BZFLAG_H

#include "common.h"

// system includes
#include <string>
#include <crystalspace.h>

extern void dumpResources();

extern bool echoToConsole;
extern bool echoAnsi;
extern std::string alternateConfig;
extern const char *argv0;
extern struct tm   userTime;

#ifdef ROBOT
extern int numRobotTanks;
#endif

class WordFilter;
class BzfVisual;
class PlatformFactory;
class BundleMgr;
class MainWindow;
class BzfWindow;
class Playing;
class Bzflag;

extern Playing *playing;

class FrameBegin2DDraw : public csBaseEventHandler {
public:
  FrameBegin2DDraw(Bzflag *_bzflag);
  virtual ~FrameBegin2DDraw();

private:
  Bzflag *bzflag;

public:
  /**
   * Setup everything that needs to be rendered on screen. This routine
   * is called from the event handler in response to a csevFrame
   * message, and is called in the "logic" phase (meaning that all
   * event handlers for 3D, 2D, Console, Debug, and Frame phases
   * will be called after this one).
   */
  void Frame();

  /* Declare the name by which this class is identified to the event scheduler.
   * Declare that we want to receive the frame event in the "LOGIC" phase,
   * and that we're not terribly interested in having other events
   * delivered to us before or after other modules, plugins, etc. */
  CS_EVENTHANDLER_PHASE_2D("application.bzflag.2dHandler");
};

class Bzflag : public csApplicationFramework,
	       public csBaseEventHandler {
public:
  Bzflag();
  virtual ~Bzflag();

  virtual bool Application();
  virtual void OnCommandLineHelp();
  virtual void OnExit();
  virtual bool OnInitialize(int argc, char *argv[]);

  /// A pointer to the 3D renderer plugin.
  csRef<iGraphics3D> g3d;
	
private:
  virtual bool OnMouseDown(iEvent &event);
  virtual bool OnMouseUp(iEvent &event);

  void parse();
  void parseConfigName();

  WordFilter      *filter;
  MainWindow      *pmainWindow;
  BzfWindow       *window;
  BzfVisual       *visual;
  PlatformFactory *platformFactory;
  BundleMgr       *bm;

  csRef<iCommandLineParser> clp;
  csRef<iGraphics2D>        g2d;

  FrameBegin2DDraw         *frame2D;

public:
  bool SetupModules ();

  /**
   * Handle keyboard events - ie key presses and releases.
   * This routine is called from the event handler in response to a 
   * csevKeyboard event.
   */
  bool OnKeyboard(iEvent&);
  
  virtual void PreProcessFrame();

  /**
   * Setup everything that needs to be rendered on screen. This routine
   * is called from the event handler in response to a csevFrame
   * message, and is called in the "logic" phase (meaning that all
   * event handlers for 3D, 2D, Console, Debug, and Frame phases
   * will be called after this one).
   */
  void Frame();
  
  virtual void ProcessFrame();
  virtual void PostProcessFrame();
  virtual void FinishFrame();

  /* Declare the name by which this class is identified to the event scheduler.
   * Declare that we want to receive the frame event in the "LOGIC" phase,
   * and that we're not terribly interested in having other events
   * delivered to us before or after other modules, plugins, etc. */
  CS_EVENTHANDLER_PHASE_LOGIC("application.bzflag")
};

#endif // BZF_BZFLAG_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
