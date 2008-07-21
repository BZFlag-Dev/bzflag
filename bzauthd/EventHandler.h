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

#ifndef __BZAUTHD_EVENTHANDLER_H__
#define __BZAUTHD_EVENTHANDLER_H__

class EventHandler : public Singleton<EventHandler>
{
public:
  typedef void (*CBFunc)(void *);
  EventHandler();
  ~EventHandler();
  void update();
  void addOffset(CBFunc func, void * data, uint64 offset_ms);
private:
};

#endif // __BZAUTHD_EVENTHANDLER_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8