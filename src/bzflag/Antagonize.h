/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_ANTAGONIZE_H
#define BZF_ANTAGONIZE_H

#include <stdio.h>
#include <stdlib.h>
#include "RemotePlayer.h"
#include "LocalPlayer.h"
#include "World.h"
#include "ServerLink.h"
#include <iostream>

// Messages are build on receive side so i18n can happen (in future)
class Antagonize
{
public:
	inline static void broadcast(int myID);
	inline static std::string Antagonize::display( Player *from, char *msg );
private:
	static std::string antagonizeMessages[];
	static TimeKeeper antagonizeTime;
	static char	antagonizeMessage[2 * PlayerIdPLen + MessageLen];
};

//This assumes that this file is only included once in playing.cxx

std::string Antagonize::antagonizeMessages[] = { 
	std::string("Wow I'm playing bad today, except again you, %s"),
	std::string("Hey %s, why are you always picking on the bad players?"),
	std::string("%s: you usually play better than this, what's the matter?"),
	std::string("Everytime I look to see who I've killed, it always you, %s"),
	std::string("Geez, %s, how'd you get such a good score, you're just driving around!"),
	std::string("Man, %s, I like how you inflated your score before I showed up!"),
	std::string("I may have to look for a better server, if %s is one of the best here!"),
	std::string("Yo %s, you best not try that weak weapon out on me!"),
	std::string("Wow %s, your score is so high, I must never run across you.")
};

TimeKeeper Antagonize::antagonizeTime;
char	Antagonize::antagonizeMessage[2 * PlayerIdPLen + MessageLen];

void Antagonize::broadcast(int myID)
{
	World *world = World::getWorld();
	int maxPlayers = world->getMaxPlayers();
	RemotePlayer **players = world->getPlayers();

	int best = -999;
	std::string bestName = "";

	if (antagonizeTime <= TimeKeeper::getCurrent()) {
		for (int t = 0; t < maxPlayers; t++) {
			if (players[t] && (t != myID)) {
				int delta = players[t]->getWins() - players[t]->getLosses();
				if (delta > best) {
					best = delta;
					bestName = players[t]->getCallSign();
				}
			}
		}

		if (bestName.length() > 0) {
			void* buf = antagonizeMessage;
			buf = nboPackUByte(buf, myID);
			buf = nboPackUByte(buf, 254/*AllPlayers*/);
			std::string msg = "ANTAGONIZE:" + bestName;

			buf = nboPackString(antagonizeMessage + 2 * PlayerIdPLen,msg.c_str(), msg.length());
			ServerLink::getServer()->send(MsgMessage, sizeof(antagonizeMessage), antagonizeMessage);

		}

		antagonizeTime = TimeKeeper::getCurrent();
		antagonizeTime+= 1.5f;
	}
}

std::string Antagonize::display( Player *from, char *msg )
{
	std::string message = msg;

	std::string fullMsg = "[";
	fullMsg += from->getCallSign();
	fullMsg += "->] ";

	char buffer[200];
	sprintf( buffer, antagonizeMessages[(int) (bzfrand() * (sizeof(antagonizeMessages) / sizeof( std::string )))].c_str(),  message.substr( 11 ).c_str());

	fullMsg += buffer;
	return fullMsg;
}

#endif