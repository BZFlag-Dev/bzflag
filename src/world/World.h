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

#ifndef __BZWORLD_H__
#define __BZWORLD_H__

/**
 * \file BZWorld.h
 * \brief libBZW header file
 * 
 * libBZW attempts to abstract world management
 */

#include <iostream>

/* bzflag common headers */
#include "common.h"

/**
 * \class BZWorld
 * \brief BZFlag World class
 *
 *  Details to come...
 */

namespace BZW {

  class World
  {
    public:
      BZWorld();
      BZWorld(std::iostream &input);
      ~BZWorld();

      void write(std::ostream &output); // same goes here

    private:

  };

}

#endif // __BZWORLD_H__

// Local Variables: ***
// Mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
