/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __MASTER_BAN_LIST_H__
#define __MASTER_BAN_LIST_H__

// common implementation headers
#include "cURLManager.h"

class MasterBanList : cURLManager
{
public:
    const std::string &get(const std::string &URL);
protected:
    std::string data;
private:
    void finalization(char *cURLdata, unsigned int length, bool good);
};
#endif //__MASTER_BAN_LIST_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
