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

#ifndef __BZWREADER_H__
#define __BZWREADER_H__

// system headers
#include <iostream>
#include <string>
#include <vector>

// implementation headers
#include "BZWError.h"

class WorldFileObject;
class WorldInfo;

class BZWReader {
public:
  BZWReader(std::string filename);
  ~BZWReader();

  // external interface
  WorldInfo *defineWorldFromFile();

private:
  // functions for internal use
  void readToken(char *buffer, int n);
  bool readWorldStream(std::vector<WorldFileObject*>& wlist);

  // stream to open
  std::string location;
  std::istream *input;

  // data/dependent objects
  BZWError *errorHandler;

  // no default constructor
  BZWReader();
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
