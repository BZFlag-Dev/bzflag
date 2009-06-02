/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * main game loop stuff
 */

#ifndef	BZF_GUIPLAYING_H
#define	BZF_GUIPLAYING_H

#include "common.h"

// system includes
#include <string>
#include <vector>

/* common headers */
#include "StartupInfo.h"
#include "CommandCompleter.h"
#include "Address.h"
#include "vectors.h"

/* local headers */
#include "MainWindow.h"
#include "ControlPanel.h"

#define MAX_MESSAGE_HISTORY (20)

BzfDisplay* getDisplay();
MainWindow* getMainWindow();
RadarRenderer* getRadarRenderer();
void notifyBzfKeyMapChanged();
bool setVideoFormat(int, bool test = false);
void forceControls(bool enabled, float speed, float angVel);

std::vector<std::string>& getSilenceList();

int curlProgressFunc(void* clientp,  double dltotal, double dlnow,  double ultotal, double ulnow);

void setRoamingLabel();
void drawFrame(const float dt);
void injectMessages(uint16_t code, uint16_t len, void *msg);

#endif // BZF_GUIPLAYING_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
