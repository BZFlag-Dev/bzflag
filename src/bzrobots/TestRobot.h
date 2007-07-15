/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
 * TestRobot: Testing basic stuff.
 */

#ifndef BZROBOTS_TESTROBOT_H
#define BZROBOTS_TESTROBOT_H

#include "BZAdvancedRobot.h"

struct TestRobot :public BZAdvancedRobot
{
  TestRobot() {}
  TestRobot(RCLinkFrontend *_link) { setLink(_link); }
  void run();
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
