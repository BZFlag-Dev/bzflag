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

#ifndef __FORCE_FEEDBACK_H__
#define __FORCE_FEEDBACK_H__

/* All functions in this namespace start playing a force feedback
 * effect if we have a FF-enabled joystick connected and the user
 * has enabled force feedback.
 */
namespace ForceFeedback {

  void death();
  void shotFired();
  void laserFired();
  void shockwaveFired();
  void solidMatterFriction();

}

#endif /* __FORCE_FEEDBACK_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
