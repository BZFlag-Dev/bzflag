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

#ifdef _MSC_VER
#pragma warning( 4: 4786)
#endif

#include <iostream>

#include "StdOutUI.h"


// add this UI to the map
UIAdder StdOutUI::uiAdder("stdout", &StdOutUI::creator);


void StdOutUI::outputMessage(const std::string& msg, ColorCode) {
  std::cout<<msg<<std::endl;
}


BZAdminUI* StdOutUI::creator(const PlayerIdMap&, PlayerId) {
  return new StdOutUI();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
