/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __CUSTOMLINK_H__
#define __CUSTOMLINK_H__

// system headers
#include <iostream>

// bzfs-specific headers
#include "WorldFileObject.h"
#include "WorldInfo.h"


class CustomLink : public WorldFileObject {
  public:
    CustomLink();
  virtual bool read(const char *cmd, std::istream& input);
    virtual void write(WorldInfo*) const;

  protected:
    int from;
    int to;
};

#endif  /* __CUSTOMLINK_H__ */

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
