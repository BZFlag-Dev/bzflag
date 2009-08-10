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

#ifndef __SCRIPTTOOL_H__
#define __SCRIPTTOOL_H__

#include "common.h"

/* system interface headers */
#include <string>
#include <map>

/* local interface headers */
#include "BZAdvancedRobot.h"


class BZRobotScript
{
public:
  BZRobotScript();
  virtual ~BZRobotScript() {}
  
  static BZRobotScript *loadFile(std::string filename);
  
  void setCallbacks(BZRobotCallbacks *_bzrobotcb);
  
  bool start();
  bool stop();
  
  bool loaded() const { return _loaded; }
  std::string getError() const { return error; }
  
  virtual bool load(std::string /*filename*/) { return false; }
  virtual BZRobot *create(void) { return NULL; }
  virtual void destroy(BZRobot * /*instance*/) { }
  
private:
  BZRobot *robot;
  
#ifndef _WIN32
  pthread_t rthread;
#else
  HANDLE rthread;
#endif // _WIN32

protected:
  bool _loaded;
  std::string error;
};

#endif /* __SCRIPTTOOL_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
