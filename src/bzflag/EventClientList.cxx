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
using std::string;

// common headers
#include "EventClient.h"


//============================================================================//
//============================================================================//

inline bool EventClientList::lessThan(const EventClient* a,
                                      const EventClient* b)
{
  const int aOrder = a->GetOrder();
  const int bOrder = b->GetOrder();
  if (aOrder < bOrder) { return !reversed; }
  if (aOrder > bOrder) { return  reversed; }

  const string& aName = a->GetName();
  const string& bName = b->GetName();
  if (aName < bName) { return !reversed; }
  if (aName > bName) { return  reversed; }

  return (a < b) ? !reversed : reversed;
}


//============================================================================//
//============================================================================//

inline void EventClientList::inc_ptrs(size_t index)
{
  for (size_t p = 0; p < ptrs.size(); p++) {
    size_t& nextIndex = *(ptrs[p]);
    if (nextIndex > index) {
      nextIndex++;
    }
  }
}


inline void EventClientList::dec_ptrs(size_t index)
{
  for (size_t p = 0; p < ptrs.size(); p++) {
    size_t& nextIndex = *(ptrs[p]);
    if (nextIndex > index) {
      nextIndex--;
    }
  }
}


//============================================================================//
//============================================================================//

bool EventClientList::insert(EventClient* ec)
{
  size_t index = 0;
  std::vector<EventClient*>::iterator it;

  // forbid duplicates
  for (it = data.begin(); it != data.end(); ++it) {
    EventClient* ec2 = *it;
    if ((ec->GetName()  == ec2->GetName()) &&
        (ec->GetOrder() == ec2->GetOrder())) {
      return false;
    }
  }

  for (it = data.begin(); it != data.end(); ++it) {
    if (lessThan(ec, *it)) {
      data.insert(it, ec);
      inc_ptrs(index); // adjust the pointers
      return true;
    }
    index++;
  }
  data.push_back(ec);
  inc_ptrs(index); // adjust the pointers

  return true;
}


bool EventClientList::remove(const EventClient* ec)
{
  std::vector<EventClient*>::iterator it;
  size_t index = 0;
  for (it = data.begin(); it != data.end(); ++it) {
    if (*it == ec) {
      data.erase(it);
      dec_ptrs(index); // adjust the pointers
      return true;
    }
    index++;
  }
  return false;
}


//============================================================================//
//============================================================================//
