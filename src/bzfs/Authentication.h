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

#ifndef BZF_AUTHENTICATION_H
#define BZF_AUTHENTICATION_H

/* bzflag special common - 1st one */
#include "common.h"

class Authentication
{
public:
    Authentication();

    bool isGlobal(void) const
    {
        return globallyAuthenticated;
    };
    void global(bool set)
    {
        globallyAuthenticated = set;
    }
private:
    bool         globallyAuthenticated;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
