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

#ifndef __SHAREDOBJECTLOADER_H__
#define __SHAREDOBJECTLOADER_H__

#include "common.h"

/* system interface headers */
#include <string>

/* local interface headers */
#include "ScriptLoader.h"
#include "BZAdvancedRobot.h"


class SharedObjectLoader : public ScriptLoader
{
  typedef BZAdvancedRobot *(*createHandle)(void);
  typedef void (*destroyHandle)(BZAdvancedRobot *);

  createHandle createFunction;
  destroyHandle destroyFunction;

  void *soHandle;

  public:
    ~SharedObjectLoader();
    bool load(std::string filename);
    BZAdvancedRobot *create(void);
    void destroy(BZAdvancedRobot *instance);
};

#endif /* __SHAREDOBJECTLOADER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
