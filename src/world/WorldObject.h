/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef __WORLDOBJECT_H__
#define __WORLDOBJECT_H__

#include <iostream>

class WorldObject {
public:
  WorldObject() { };
  virtual ~WorldObject() { };

  virtual std::string myToken() const = 0;
  static std::istream& operator>>(std::istream& o, WorldObject&);
  static std::ostream& operator<<(std::ostream& o, const WorldObject&) const;
}

#endif //__WORLDOBJECT_H__
