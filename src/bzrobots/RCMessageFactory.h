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

#ifndef BZROBOTS_RCMESSAGEFACTORY_H
#define BZROBOTS_RCMESSAGEFACTORY_H

#include "common.h"

/* system headers */
#include <string>

/* common interface headers */
#include "Factory.h"
#include "Singleton.h"

#include "RCRequests.h"
#include "RCReplies.h"

/** convenience handle on the singleton instance */
#define RCREQUEST (RCMessageFactory<RCRequest>::instance())
#define RCREPLY (RCMessageFactory<RCReply>::instance())
#define RCEVENT (RCMessageFactory<RCEventNotification>::instance())

template<class C>
class RCMessageFactory : public Singleton< RCMessageFactory<C> >,
                            public Factory<C, std::string>
{

public:
  void setLink(RCLink *_link)
  {
    link = _link;
  }

  C *Message(std::string s)
  {
    C *pointer = Factory<C, std::string>::Create(s);
    if (pointer != NULL)
      pointer->setLink(link);

    return pointer;
  }
  
  static void initialize();

protected:
  friend class Singleton<RCMessageFactory>;

private:
  RCLink *link;
  RCMessageFactory() { }
  ~RCMessageFactory() { }
};

// initialize the singleton
template<class T>
T* Singleton<T>::_instance = NULL;

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
