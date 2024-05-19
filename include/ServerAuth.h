/* bzflag
 * Copyright (c) 1993-2024 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_SERVERAUTH_H
#define BZF_SERVERAUTH_H

#include "common.h"

/* system interface headers */
#include <vector>

/* common interface headers */
#include "StartupInfo.h"
#include "cURLManager.h"

class ServerAuth : cURLManager
{
public:
    ServerAuth();
    virtual ~ServerAuth();

    void requestToken(StartupInfo *info);
    void finalization(char *data, unsigned int length, bool good);

private:
    StartupInfo *startupInfo;
};

#endif // BZF_SERVERAUTH_H
