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

#include <map>

class EventHandler : public Singleton<EventHandler>
{
public:
  typedef void (*CBFunc)(void *);
  friend class EventPtr;
  EventHandler();
  ~EventHandler();
  void update();
  void addDelta(CBFunc func, void * data, double delta);
private:
  class Event;
  typedef std::multimap<double, Event *> TimeMapType;
  class Event
  {
  public:
    ~Event();
    Event(CBFunc f, void *d, TimeMapType::iterator &i);
    void call();
    void delink();
  
    uint16 refCounter;
    CBFunc func;
    void *data;
    TimeMapType::iterator itr;
  };

  TimeMapType timeMap;
};

class EventPtr
{
public:
  friend class EventHandler;
  EventPtr() { ev = NULL; }
  EventPtr(const EventPtr &ptr) { incRef(ptr.ev); }
  ~EventPtr() { decRef(); }
  void cancel() { if(!ev) return; delete ev; ev = NULL; }
  operator bool() { return ev != NULL; }
private:
  EventHandler::Event *ev;
  void incRef(EventHandler::Event *e) { decRef(); ev = e; ev->refCounter++; }
  void decRef() { if(!ev) return; ev->refCounter--; if(!ev->refCounter) { delete ev; ev = NULL; } }
};

#define sEventHandler EventHandler::instance()

#endif // __BZAUTHD_EVENTHANDLER_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8