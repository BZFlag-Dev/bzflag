/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef  AUTOHUNT_H
#define  AUTOHUNT_H

#include "common.h"

// system headers
#include <string>


namespace AutoHunt {

  void update();

  const char* getColorString(int level);

  int   getChevronCount(int level);
  float getChevronSpace(int level);
  float getChevronDelta(int level);
  float getChevronInnerAlpha(int level);
  float getChevronOuterAlpha(int level);
  float getBlinkPeriod(int level);
  float getInnerBlinkThreshold(int level); // tank square
  float getOuterBlinkThreshold(int level); // corner chevrons
  // NOTE: (blink thresholds)
  //       0 -> always blink color
  //       1 -> always normal color
}


#endif // AUTOHUNT_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
