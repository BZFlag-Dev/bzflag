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

#ifndef __CUSTOMWORLD_H__
#define __CUSTOMWORLD_H__

// system headers
#include <iostream>

// bzfs-specific headers
#include "WorldFileObject.h"
#include "WorldInfo.h"


class CustomWorld : public WorldFileObject {
public:
  CustomWorld();
  virtual bool read(const char *cmd, std::istream&);
  virtual void write(WorldInfo*) const;

protected:
  double _size;
  double _fHeight;
};

#endif  /* __CUSTOMWORLD_H__ */

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
