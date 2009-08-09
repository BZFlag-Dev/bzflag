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

#ifndef __BZROBOTEVENT_H__
#define __BZROBOTEVENT_H__

class BZRobotEvent
{
public:
  BZRobotEvent() { eventName = "Unknown"; eventID = -1; }
  virtual ~BZRobotEvent() {}
  int getPriority() { return priority; }
  long getTime() { return time; }
  void setPriority(int _priority) { priority = _priority; }
  void setTime(long _time) { time = _time; }
  std::string const getEventName() { return eventName; }
  int const getEventID() { return eventID; }
protected:
  std::string eventName;
  int eventID;
private:
  int priority;
  long time;
};

#else

class BZRobotEvent;

#endif /* __BZROBOTEVENT_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
