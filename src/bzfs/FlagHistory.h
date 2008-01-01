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

#ifndef __FLAGHISTORY_H__
#define __FLAGHISTORY_H__

#include "common.h"

// system headers
#include <vector>

// bzflag library headers
#include "Flag.h"

class FlagHistory {
 public:
  void clear();
  void get(char message[]);
  void add(FlagType* type);
  std::vector<FlagType*> get(void) { return flagHistory;}
 private:
  std::vector<FlagType*> flagHistory;
};

#endif /* __FLAGHISTORY_H__ */

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
