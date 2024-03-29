/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "StdOutUI.h"

/* system implementation headers */
#include <iostream>


// add this UI to the map
UIAdder StdOutUI::uiAdder("stdout", &StdOutUI::creator);


StdOutUI::StdOutUI(BZAdminClient& c) : BZAdminUI(c)
{

}


void StdOutUI::outputMessage(const std::string& msg, ColorCode)
{
    std::cout<<msg<<std::endl;
}


BZAdminUI* StdOutUI::creator(BZAdminClient& client)
{
    return new StdOutUI(client);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
