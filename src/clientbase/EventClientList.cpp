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

#include "common.h"

// implementation header
#include "EventClientList.h"

// system headers
#include <string>
#include <list>

// common headers
#include "clientbase/EventClient.h"


//============================================================================//
//============================================================================//

inline bool EventClientList::lessThan(const EventClient* a,
                                      const EventClient* b) {
  if ((a == &DummyEventClient::instance) ||
      (b == &DummyEventClient::instance)) {
    return false;
  }

  const int aOrder = a->GetOrder(orderType);
  const int bOrder = b->GetOrder(orderType);
  if (aOrder < bOrder) { return !reversed; }
  if (aOrder > bOrder) { return  reversed; }

  const std::string& aName = a->GetName();
  const std::string& bName = b->GetName();
  if (aName < bName) { return !reversed; }
  if (aName > bName) { return  reversed; }

  return (a < b) ? !reversed : reversed;
}


//============================================================================//

bool EventClientList::insert(EventClient* ec) {
  // forbid duplicates
  for (iterator it = clients.begin(); it != clients.end(); ++it) {
    EventClient* ec2 = *it;
    if ((ec->GetName() == ec2->GetName()) &&
        (ec->GetOrder(orderType) == ec2->GetOrder(orderType))) {
      return true;
    }
  }

  // insert if lessThan
  for (iterator it = clients.begin(); it != clients.end(); ++it) {
    EventClient* ec2 = *it;
    if (lessThan(ec, ec2)) {
      clients.insert(it, ec);
      return true;
    }
  }

  clients.push_back(ec);

  return true;
}


//============================================================================//

bool EventClientList::remove(const EventClient* ec) {
  for (iterator it = clients.begin(); it != clients.end(); ++it) {
    EventClient* ec2 = *it;
    if (ec == ec2) {
      *it = &DummyEventClient::instance;
      return true;
    }
  }
  return false;
}


//============================================================================//

void EventClientList::purify() {
  clients.remove(&DummyEventClient::instance);
}


//============================================================================//
//============================================================================//
