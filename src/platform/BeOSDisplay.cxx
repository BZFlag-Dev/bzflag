/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "BeOSDisplay.h"
#include "BeOSWindow.h"
#include "BzfEvent.h"
#include <OS.h>
#include <stdio.h>
#include <string.h>

BeOSDisplay::BeOSDisplay(const char* displayName, const char*) :
  BzfDisplay()
{
  printf("BeOSDisplay::BeOSDisplay()\n");
  /* this port will be checked by BeOSDisplay for events, and fed by the BWindow */
  eventPort = create_port(50, "bzflag_event_port"); /* should check for errors */

  // get resolutions
  bScreen = new BScreen;
  if (isValid()) {
    int numModes, currentMode;
    ResInfo** resInfo = NULL;
    display_mode dm;

    // if no modes then make default
    if (!resInfo) {
      resInfo = new ResInfo*[1];
      bScreen->GetMode(&dm);
      resInfo[0] = new ResInfo("default", dm.virtual_width, dm.virtual_height, 0);
      numModes = 1;
      currentMode = 0;
    }
    printf("default rez\n");
    // register modes
    initResolutions(resInfo, numModes, currentMode);
  }
}

BeOSDisplay::~BeOSDisplay()
{
  printf("BeOSDisplay::~BeOSDisplay()\n");
  /* some cleanup */
  delete_port(eventPort);

  //setDefaultResolution();
  delete bScreen;
}

bool					BeOSDisplay::isValid() const
{
  printf("BeOSDisplay::isValid() %d\n", (int)bScreen->IsValid());
  return bScreen->IsValid();
}

bool					BeOSDisplay::isEventPending() const
{
  if (beosWin && beosWin->currentOglContext == find_thread(NULL)) {
    beosWin->yieldCurrent();
  }
  //  snooze(10000);
  return (port_buffer_size_etc(eventPort, B_TIMEOUT, 0LL) > 0);
}

bool					BeOSDisplay::peekEvent(BzfEvent& event) const
{
  return false;
}

bool					BeOSDisplay::getEvent(BzfEvent& event) const
{
  //printf("BeOSDisplay::getEvent()\n");
  bool is_current_ogl = false;
  status_t err;
  int32 what;
  if (beosWin && beosWin->currentOglContext == find_thread(NULL)) {
    is_current_ogl = true;
    beosWin->releaseCurrent();
  }
  err = read_port_etc(eventPort, &what, (void *)&event, sizeof(BzfEvent), B_TIMEOUT, 0LL);
  if (is_current_ogl)
    beosWin->makeCurrent();
  //printf("<BeOSDisplay::getEvent()\n");
  return (err >= B_OK);
}

void				BeOSDisplay::postBzfEvent(BzfEvent &event)
{
  int32 what = 'BzfE';
  write_port_etc(eventPort, what, (void *)&event, sizeof(BzfEvent), B_TIMEOUT, 10000LL);
}

bool					BeOSDisplay::doSetDefaultResolution()
{
  return true;
}

bool					BeOSDisplay::doSetResolution(int index)
{
  // try setting the format
  const bool changed  = true;

  return changed;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

