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

class Bzflag : public csApplicationFramework,
	       public csBaseEventHandler {
public:
  Bzflag();
  virtual ~Bzflag();

  virtual bool Application();
  virtual void OnCommandLineHelp();
  virtual void OnExit();
  virtual bool OnInitialize(int argc, char *argv[]);

private:
  virtual void PreProcessFrame();
  virtual void Frame();
  virtual void ProcessFrame();
  virtual void PostProcessFrame();
  virtual void FinishFrame();

  virtual bool OnKeyboard(iEvent &event);
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
  Playing         *playing;

  csRef<iCommandLineParser> clp;
  csRef<iGraphics3D>        g3d;
  csRef<iGraphics2D>        g2d;

  CS_EVENTHANDLER_NAMES("application.bzflag");
  CS_EVENTHANDLER_NIL_CONSTRAINTS
};

#endif // BZF_BZFLAG_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
