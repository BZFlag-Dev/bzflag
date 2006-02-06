/* bzflag
* Copyright (c) 1993 - 2006 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef _BZFS_CLIENT_MESSAGES_H_
#define _BZFS_CLIENT_MESSAGES_H_

#include "global.h"
#include "bzfs.h"

bool handleClientEnter ( void **buf, GameKeeper::Player *playerData );
void handleClientExit ( GameKeeper::Player *playerData );
void handleSetVar ( NetHandler *netHandler );
void handlePlayerUpdate ( void **buf, uint16_t &code, GameKeeper::Player *playerData, const void* rawbuf, int len );	//once relay is based on state, remove rawbuf

#endif //_BZFS_CLIENT_MESSAGES_H_


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
