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

#ifndef __SCRIPTTOOLFACTORY_H__
#define __SCRIPTTOOLFACTORY_H__

#include "common.h"

/* system headers */
#include <string>

/* common interface headers */
#include "Factory.h"
#include "Singleton.h"
#include "TextUtils.h"

/* local interface headers */
#include "BZRobotScript.h"


/** convenience handle on the singleton instance */
#define SCRIPTTOOLFACTORY (ScriptLoaderFactory::instance())

class ScriptLoaderFactory : public Singleton<ScriptLoaderFactory>,
			    public Factory<BZRobotScript, std::string>
{

public:
  BZRobotScript *scriptTool(std::string extension);
  static void initialize();


protected:
  friend class Singleton<ScriptLoaderFactory>;

private:
  ScriptLoaderFactory();
  ~ScriptLoaderFactory();
};


#endif /* __SCRIPTTOOLFACTORY_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
