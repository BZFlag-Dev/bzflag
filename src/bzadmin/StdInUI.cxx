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
#include "StdInUI.h"

/* system implementation headers */
#include <iostream>


// add this UI to the map
UIAdder StdInUI::uiAdder("stdin", &StdInUI::creator);


StdInUI::StdInUI(BZAdminClient& c) : BZAdminUI(c)
{

}


bool StdInUI::checkCommand(std::string& str)
{
    if (std::cin.eof())
    {
        str = "/quit";
        return true;
    }
    std::getline(std::cin, str);
    if (str == "")
        return false;
    return true;
}


BZAdminUI* StdInUI::creator(BZAdminClient& client)
{
    return new StdInUI(client);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
