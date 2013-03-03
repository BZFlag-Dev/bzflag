#ifndef _PLAYER_HANDLER_H_
#define _PLAYER_HANDLER_H_

#include "bzfsAPI.h"

class PlayerHandler: public bz_ServerSidePlayerHandler
{
public:
	 virtual void added(int player); // it is required that the bot provide this method

	 virtual void textMessage(int dest, int source, const char *text);

	 virtual void playerSpawned(int player, const float pos[3], float rot);
	 virtual void shotFired(int player, unsigned short shotID);
};

#endif //_PLAYER_HANDLER_H_