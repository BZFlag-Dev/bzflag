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

#include <cstring>
#include <iostream>

#include "BZWorld.h"

BZW::World * world;

int main(int argc, char ** argv)
{
  if(argc > 1)
  {
    std::cout << "Usage: BZWorldTest\n\nExamples:\n  BZWorldTest\n  cat testfile | BZWorldTest\n  BZWorldTest < testfile" << std::endl;
    return 1;
  }

  world = new World();


  delete world;

  return 0;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
