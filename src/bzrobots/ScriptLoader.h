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

#ifndef __SCRIPTLOADER_H__
#define __SCRIPTLOADER_H__

#include "common.h"

/* system interface headers */
#include <string>

/* local interface headers */
#include "BZAdvancedRobot.h"


class ScriptLoader
{
public:
  virtual ~ScriptLoader() {}
  
  virtual bool load(std::string filename) = 0;
  virtual BZAdvancedRobot *create(void) = 0;
  virtual void destroy(BZAdvancedRobot *instance) = 0;
  
  std::string getError() const { return error; }
protected:
  std::string error;
};

#endif /* __SCRIPTLOADER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
