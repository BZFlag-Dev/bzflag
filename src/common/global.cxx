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

#include "global.h"


GlobalDBItem globalDBItems[5] = {
  { "_gravity",		"-9.81",	false,	StateDatabase::Locked },
  { "_tankLength",	"6.0",		false,	StateDatabase::Locked },
  { "_tankWidth",	"2.8",		false,	StateDatabase::Locked },
  { "_tankSpeed",       "25.0",         false,  StateDatabase::Locked },
  { "_lockOnAngle",     "0.15",         false,  StateDatabase::Locked }
};
