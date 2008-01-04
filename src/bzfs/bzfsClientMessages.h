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

#ifndef _BZFS_CLIENT_MESSAGES_H_
#define _BZFS_CLIENT_MESSAGES_H_

#include "global.h"
#include "bzfs.h"

void packWorldSettings ( void );

class PlayerNetworkMessageHandler
{
public:
  virtual ~PlayerNetworkMessageHandler(){};

  virtual void *unpackPlayer ( void * buf, int len ) = 0;
  virtual bool execute ( uint16_t &code, void * buf, int len ) = 0;

  GameKeeper::Player *getPlayer(void) {return player;}

protected:
  GameKeeper::Player *player;
};

extern std::map<uint16_t,PlayerNetworkMessageHandler*> playerNetworkHandlers;

class ClientNetworkMessageHandler
{
public:
  virtual ~ClientNetworkMessageHandler(){};
  virtual bool execute ( NetHandler *handler, uint16_t &code, void * buf, int len ) = 0;
};

extern std::map<uint16_t,ClientNetworkMessageHandler*> clientNetworkHandlers;

void registerDefaultHandlers ( void );
void cleanupDefaultHandlers ( void );

// util functions
bool updatePlayerState(GameKeeper::Player *playerData, PlayerState &state, float timeStamp, bool shortState);


#endif //_BZFS_CLIENT_MESSAGES_H_


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
