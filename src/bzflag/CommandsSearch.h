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

#ifndef BZF_COMMANDS_SEARCH_H
#define BZF_COMMANDS_SEARCH_H

#include "common.h"
#include <string>
#include "Address.h"
#include "Ping.h"
#include <vector>

class CommandsSearch {
public:
	static void			add();
	static void			remove();
	static void			search();

private:
	enum Phase {
		Idle,
		LookupListServers,
		ConnectListServers,
		SendRequest,
		WaitForReply
	};
	struct GameServer {
	public:
		std::string		name;
		std::string		description;
		PingPacket		ping;
	};
	struct ListServer {
	public:
		Address			address;
		int				port;
		int				socket;
		Phase			phase;
		std::string		buffer;
	};

	CommandsSearch();
	~CommandsSearch();

	static CommandsSearch*	getInstance();

	void				onSearch();

	void				openPingSockets();
	void				closePingSockets();
	void				sendPing();

	void				clearListServers();
	void				lookupListServers();
	void				connectToListServers();

	void				handleReplies();
	void				parseListServerResponse(const ListServer&);

	void				addGameServerWithLookup(GameServer&);
	void				addGameServer(GameServer&);
	static int			getPlayerCount(const GameServer&);

	void				onFrame();
	static void			onFrameCB(void*);

	void				onServerName(const std::string&);
	static void			onServerNameCB(const std::string&, void*);

private:
	typedef std::vector<GameServer> GameServerList;
	typedef std::vector<ListServer> ListServerList;

	Phase				phase;
	GameServerList		gameServers;
	ListServerList		listServers;

	int					pingInSocket;
	int					pingOutSocket;
	int					pingBcastSocket;
	struct sockaddr_in	pingInAddr;
	struct sockaddr_in	pingOutAddr;
	struct sockaddr_in	pingBcastAddr;

	static CommandsSearch* instance;
};

#endif
