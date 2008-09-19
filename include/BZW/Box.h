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

#ifndef __BOX_H__
#define __BOX_H__

#include <string>
#include <sstream>

/* BZW */
#include "WorldObject.h"

namespace BZW
{
  class Box : public PostionalWorldObject
  {
    public:
      /// Default Constructor
      Box();

      // for the base object this will probably be NOTHING
      // maybe a geometric descriptor or something
    private:
  };

}

#endif // __BOX_H__

// Local Variables: ***
// Mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
