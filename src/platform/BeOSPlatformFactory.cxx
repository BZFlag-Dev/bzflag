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

#include "BeOSPlatformFactory.h"
#include "BeOSDisplay.h"
#include "BeOSMedia.h"
#include "BeOSVisual.h"
#include "BeOSWindow.h"
#include <Application.h>
#include <OS.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

PlatformFactory*		PlatformFactory::getInstance()
{
  if (instance == NULL)
    instance = new BeOSPlatformFactory;
  return instance;
}

static thread_id main_thid;
static thread_id bapp_thid;
static int own_BApp_created = 0;
static int refcount = 0;

/* create the BApplication and Run() it */
static int32 bapp_thread(void *arg)
{
  new BApplication("application/x-vnd.bzflag");
  own_BApp_created = 1;
  be_app->Run();
  /* kill the process group */
//  kill(0, SIGINT);
//  kill(main_thid, SIGHUP);
  return B_OK;
}

BeOSPlatformFactory::BeOSPlatformFactory()
{
  /* create the BApplication only if needed */
  if (refcount++ == 0) {
    /* needed by libmedia */
    if (be_app == NULL) {
      bapp_thid = spawn_thread(bapp_thread, "bzflag BApplication", B_NORMAL_PRIORITY, NULL);
      resume_thread(bapp_thid);
      while (!own_BApp_created)
	snooze(50000);
    }
  }
}

BeOSPlatformFactory::~BeOSPlatformFactory()
{
  /* destroy the BApplication if we created it */
  if (--refcount == 0 && own_BApp_created) {
    be_app->Lock();
    be_app->Quit();
    delete be_app;
    be_app = NULL;
  }
}

BzfDisplay*				BeOSPlatformFactory::createDisplay(
								const char* name, const char*videoFormat)
{

  BeOSDisplay* display = new BeOSDisplay(name, videoFormat);
  if (!display || !display->isValid()) {
    delete display;
    return NULL;
  }

  return display;
}

BzfVisual*				BeOSPlatformFactory::createVisual(
								const BzfDisplay* display)
{
  return new BeOSVisual((BeOSDisplay *)display);
}

BzfWindow*				BeOSPlatformFactory::createWindow(
								const BzfDisplay* display, BzfVisual* visual)
{
  BeOSWindow *win;
  win = new BeOSWindow((BeOSDisplay *)display, (BeOSVisual *)visual);
  if (!win)
    return NULL;
  ((BeOSDisplay *)display)->beosWin = win;
//  win->applyVisual(visual);
//  win->makeCurrent(); /* activate the OpenGL context for this thread */
  return win;
}

BzfMedia*				BeOSPlatformFactory::createMedia()
{
  return new BeOSMedia;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
