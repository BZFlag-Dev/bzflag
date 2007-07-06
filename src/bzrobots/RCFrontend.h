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

/*
 * Remote Control Frontend: Class to encapsulate the frontend.
 */

#ifndef	BZF_RC_FRONTEND_H
#define	BZF_RC_FRONTEND_H

#include "RCLinkFrontend.h"

class RCFrontend
{
    RCLinkFrontend *link;
    public:
      static bool run(const char *host, int port); 

      RCFrontend(const char *host, int port);
      void update(void);
};

#endif
