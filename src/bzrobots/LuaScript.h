/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __LUA_SCRIPT_H__
#define __LUA_SCRIPT_H__

// system headers
#include <string>

// local headers
#include "RobotScript.h"
#include "Robot.h"


class LuaScript : public RobotScript {
  public:
    LuaScript();
    ~LuaScript();

    bool load(std::string filename);
    BZRobots::Robot* create();
    void destroy(BZRobots::Robot* instance);

  private:
    std::string scriptFile;
    bool initialized;
    bool initialize();
    bool addSysPath(std::string path);
};


#endif /* __LUA_SCRIPT_H__ */


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
