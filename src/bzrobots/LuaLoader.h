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

#ifndef __LUA_ROBOT_H__
#define __LUA_ROBOT_H__

// system headers
#include <string>

// local headers
#include "BZRobotScript.h"
#include "BZAdvancedRobot.h"


class LuaLoader : public BZRobotScript {
  public:
    LuaLoader();
    ~LuaLoader();

    bool load(std::string filename);
    BZRobot* create();
    void destroy(BZRobot* instance);

  private:
    std::string scriptFile;
    bool initialized;
    bool initialize();
    bool addSysPath(std::string path);
};


#endif /* __LUA_ROBOT_H__ */


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
