/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __REJOIN_LIST_H__
#define __REJOIN_LIST_H__

/* common header */
#include "common.h"

/* system headers */
#include <list>

class RejoinList {
  public:
    RejoinList ();
    ~RejoinList ();

    bool add (int playerIndex);
    float waitTime (int playerIndex);

  private:
    std::list<struct RejoinNode*> queue;
};


#endif  /* __REJOIN_LIST_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
