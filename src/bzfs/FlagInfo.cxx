/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* no header other than FlagInfo.h should be included here */

#ifdef _WIN32
#pragma warning( 4:4786)
#endif

#include "FlagInfo.h"


/* private */

/* protected */

/* public */

// flags list
FlagInfo *flag = NULL;

bool setRequiredFlag(FlagInfo& flag, FlagDesc *desc)
{
  flag.required = true;
  flag.flag.desc = desc;
  return true;
}


// ex: shiftwidth=2 tabstop=8
