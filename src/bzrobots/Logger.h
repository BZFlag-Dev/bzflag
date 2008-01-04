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

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "common.h"

/* system headers */
#include <string>
#include <fstream>

/* common interface headers */
#include "Singleton.h"

/** convenience handle on the frontend logger singleton */
#define FRONTENDLOGGER (FrontendLogger::instance())

/** convenience handle on the backedn logger singleton */
#define BACKENDLOGGER (BackendLogger::instance())


class FrontendLogger : public Singleton< FrontendLogger >,
		       public std::ofstream
{
protected:
  friend class Singleton<FrontendLogger>;

private:
  FrontendLogger() : std::ofstream("frontend.log") {}
  ~FrontendLogger() { }
};


class BackendLogger : public Singleton< BackendLogger >,
		      public std::ofstream
{
protected:
  friend class Singleton<BackendLogger>;

private:
  BackendLogger() : std::ofstream("backend.log") {}
  ~BackendLogger() { }
};

#else
class FrontendLogger;
#endif  /* __LOGGER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
