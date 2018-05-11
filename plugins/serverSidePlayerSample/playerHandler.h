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

#ifndef _PLAYER_HANDLER_H_
#define _PLAYER_HANDLER_H_

#include "bzfsAPI.h"

class PlayerHandler: public bz_ServerSidePlayerHandler
{
public:
  virtual void added(int player);	// it is required that the bot provide this method

  virtual void textMessage(int dest, int source, const char *text);

  virtual void playerSpawned(int player, const float pos[3], float rot);
  virtual void shotFired(int player, unsigned short shotID);
};

#endif //_PLAYER_HANDLER_H_


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
