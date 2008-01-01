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

#ifndef _BZFS_CHAT_VERIFY_H_
#define _BZFS_CHAT_VERIFY_H_

#include "global.h"
#include "bzfs.h"

bool checkChatSpam(char* message, GameKeeper::Player* playerData, int t);
bool checkChatGarbage(char* message, GameKeeper::Player* playerData, int t);

#endif //_BZFS_CHAT_VERIFY_H_


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
