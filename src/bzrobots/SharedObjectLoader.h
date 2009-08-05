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
#include "BZRobotScript.h"
#include "BZAdvancedRobot.h"


class SharedObjectLoader : public BZRobotScript
{
  typedef BZRobot *(*createHandle)(void);
  typedef void (*destroyHandle)(BZRobot *);

  createHandle createFunction;
  destroyHandle destroyFunction;

#ifdef _WIN32
	HINSTANCE soHandle;
#else
	void *soHandle;
#endif /* _WIN32 */

  public:
    ~SharedObjectLoader();
    bool load(std::string filename);
    BZRobot *create(void);
    void destroy(BZRobot *instance);
};

#endif /* __SHAREDOBJECTLOADER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
