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

#include "CommandsSearch.h"
#include "CommandManager.h"
#include "StateDatabase.h"
#include "ErrorHandler.h"
#include "network.h"
#include "multicast.h"
#include "PlatformFactory.h"
#include "Protocol.h"
#include "playing.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

//
// command name to function mapping
//

static BzfString	cmdSearch(const BzfString&,
								const CommandManager::ArgList& args)
{
	if (args.size() != 0)
		return "usage: search";
	CommandsSearch::search();
	return BzfString();
}

struct CommandListItem {
	public:
		const char*				name;
		CommandManager::CommandFunction func;
		const char*				help;
};
static const CommandListItem commandList[] = {
	{ "search",		&cmdSearch, "search:  search for available servers" }
};


//
// database entries we muck with
//

static const char* databaseNames[] = {
		"_serverList",
		"_serverCount",
		"_serverName",
		"_serverNumTotal",
		"_serverNumRed",
		"_serverNumGreen",
		"_serverNumBlue",
		"_serverNumPurple",
		"_serverNumRogue",
		"_serverNumShots",
		"_serverStyle",
		"_serverSuperFlags",
		"_serverShakeAntidote",
		"_serverShakeTime",
		"_serverShakeWins",
		"_serverJumping",
		"_serverRicochet",
		"_serverInertia",
		"_serverLimitTime",
		"_serverLimitTeamScore",
		"_serverLimitPlayerScore"
};

// don't erase any DB entry with an index below this in the list above
static const unsigned int firstPerServerName = 3;


//
// CommandsSearch
//

CommandsSearch*			CommandsSearch::instance = NULL;

CommandsSearch::CommandsSearch() : phase(Idle),
								pingInSocket(-1),
								pingOutSocket(-1),
								pingBcastSocket(-1)
{
	// add callback for each frame
	addPlayingCallback(&onFrameCB, this);

	// make database entries read only and non-persistent
	for (unsigned int i = 0; i < countof(databaseNames); ++i) {
		BZDB->setPersistent(databaseNames[i], false);
		BZDB->setPermission(databaseNames[i], StateDatabase::ReadOnly);
	}

	// watch for changes to _serverName
	BZDB->addCallback("_serverName", onServerNameCB, this);
}

CommandsSearch::~CommandsSearch()
{
	// stop watching for changes to _serverName
	BZDB->removeCallback("_serverName", onServerNameCB, NULL);

	// remove frame callback
	removePlayingCallback(&onFrameCB, this);

	// close sockets
	closePingSockets();

	// close list server sockets
	clearListServers();

	// no instance anymore
	instance = NULL;
}

CommandsSearch*			CommandsSearch::getInstance()
{
	if (instance == NULL)
		instance = new CommandsSearch;
	return instance;
}

void					CommandsSearch::add()
{
	unsigned int i;
	for (i = 0; i < countof(commandList); ++i)
		CMDMGR->add(commandList[i].name, commandList[i].func, commandList[i].help);
}

void					CommandsSearch::remove()
{
	unsigned int i;
	for (i = 0; i < countof(commandList); ++i)
		CMDMGR->remove(commandList[i].name);
}

void					CommandsSearch::search()
{
	getInstance()->onSearch();
}

void					CommandsSearch::onFrameCB(void* self)
{
	reinterpret_cast<CommandsSearch*>(self)->onFrame();
}

void					CommandsSearch::onServerNameCB(
								const BzfString& name, void* self)
{
	reinterpret_cast<CommandsSearch*>(self)->onServerName(BZDB->get(name));
}

void					CommandsSearch::onServerName(
								const BzfString& serverName)
{
	// find the server
	GameServerList::const_iterator index;
	for (index = gameServers.begin(); index != gameServers.end(); ++index)
		if (index->name == serverName)
			break;

	// clear values
	for (unsigned int i = firstPerServerName; i < countof(databaseNames); ++i)
		BZDB->unset(databaseNames[i]);

	// update state database with info on named server
	if (index != gameServers.end()) {
		// player counts
		const PingPacket& ping = index->ping;
		BZDB->set("_serverNumTotal", BzfString::format("%d/%d",
								ping.rogueCount + ping.redCount +
								ping.greenCount + ping.blueCount +
								ping.purpleCount, ping.maxPlayers));
		if (ping.redMax >= ping.maxPlayers)
			BZDB->set("_serverNumRed", BzfString::format("%d", ping.redCount));
		else
			BZDB->set("_serverNumRed", BzfString::format("%d/%d",
								ping.redCount, ping.redMax));
		if (ping.greenMax >= ping.maxPlayers)
			BZDB->set("_serverNumGreen", BzfString::format("%d", ping.greenCount));
		else
			BZDB->set("_serverNumGreen", BzfString::format("%d/%d",
								ping.greenCount, ping.greenMax));
		if (ping.blueMax >= ping.maxPlayers)
			BZDB->set("_serverNumBlue", BzfString::format("%d", ping.blueCount));
		else
			BZDB->set("_serverNumBlue", BzfString::format("%d/%d",
								ping.blueCount, ping.blueMax));
		if (ping.purpleMax >= ping.maxPlayers)
			BZDB->set("_serverNumPurple", BzfString::format("%d", ping.purpleCount));
		else
			BZDB->set("_serverNumPurple", BzfString::format("%d/%d",
								ping.purpleCount, ping.purpleMax));
		if (ping.gameStyle & RoguesGameStyle)
			if (ping.rogueMax >= ping.maxPlayers)
				BZDB->set("_serverNumRogue", BzfString::format("%d", ping.rogueCount));
			else
				BZDB->set("_serverNumRogue", BzfString::format("%d/%d",
								ping.rogueCount, ping.rogueMax));

		// shots
		BZDB->set("_serverNumShots", BzfString::format("%d Shot%s",
								ping.maxShots,
								(ping.maxShots == 1) ? "" : "s"));

		// game style
		if (ping.gameStyle & TeamFlagGameStyle)
			BZDB->set("_serverStyle", "Capture-the-Flag");
		else
			BZDB->set("_serverStyle", "Deathmatch");
		if (ping.gameStyle & SuperFlagGameStyle)
			BZDB->set("_serverSuperFlags", "Super Flags");
		if (ping.gameStyle & JumpingGameStyle)
			BZDB->set("_serverJumping", "Jumping");
		if (ping.gameStyle & RicochetGameStyle)
			BZDB->set("_serverRicochet", "Ricochet");
		if (ping.gameStyle & InertiaGameStyle)
			BZDB->set("_serverInertia", "Inertia");

		// flag shaking
		if (ping.gameStyle & AntidoteGameStyle)
			BZDB->set("_serverShakeAntidote", "Antidote Flags");
		if (ping.gameStyle & ShakableGameStyle)
			BZDB->set("_serverShakeTime",
								BzfString::format("%.1fsec To Drop Bad Flag",
								0.1f * float(ping.shakeTimeout)));
		if (ping.gameStyle & ShakableGameStyle)
			BZDB->set("_serverShakeWins",
								BzfString::format("%d Win%s Drops Bad Flag",
								ping.shakeWins,
								(ping.shakeWins == 1) ? "" : "s"));

		// game over conditions
		if (ping.maxTime != 0) {
			BzfString msg;
			if (ping.maxTime >= 3600)
				msg = BzfString::format("Time limit: %d:%02d:%02d",
								ping.maxTime / 3600,
								(ping.maxTime / 60) % 60,
								ping.maxTime % 60);
			else if (ping.maxTime >= 60)
				msg = BzfString::format("Time limit: %d:%02d",
								ping.maxTime / 60,
								ping.maxTime % 60);
			else
				msg = BzfString::format("Time limit: 0:%02d",
								ping.maxTime);
			BZDB->set("_serverLimitTime", msg);
		}
		if (ping.maxTeamScore != 0)
			BZDB->set("_serverLimitTeamScore",
								BzfString::format("Max team score: %d",
								ping.maxTeamScore));
		if (ping.maxPlayerScore != 0)
			BZDB->set("_serverLimitPlayerScore",
								BzfString::format("Max player score: %d",
								ping.maxPlayerScore));
	}
}

void					CommandsSearch::onSearch()
{
	// clear current list for a fresh start
	clearListServers();
	gameServers.clear();
	onServerName("");
	BZDB->set("_serverCount", "0");
	BZDB->unset("_serverList");

	// get list server URL.  we're done if it's not set.
	BzfString url = BZDB->get("infoListServerURL");
	if (url.empty())
		phase = Idle;
	else
		phase = LookupListServers;

	// open sockets if they're not open yet
	openPingSockets();

	// send pings
	sendPing();
}

void					CommandsSearch::onFrame()
{
	// handle list server phases
	switch (phase) {
		case Idle:
			break;

		case LookupListServers:
			lookupListServers();
			break;

		case ConnectListServers:
			connectToListServers();
			break;
	}

	// process echos
	handleReplies();
}

void					CommandsSearch::openPingSockets()
{
	int ttl = atoi(BZDB->get("infoNetworkTTL").c_str());
	Address multicastAddress(BroadcastAddress);
	if (ttl > 0 && pingInSocket == -1)
		pingInSocket  = openMulticast(multicastAddress,
								ServerPort, NULL,
								ttl,
								BZDB->get("infoMulticastInterface").c_str(),
								"r", &pingInAddr);
	if (ttl > 0 && pingOutSocket == -1)
		pingOutSocket = openMulticast(multicastAddress,
								ServerPort, NULL,
								ttl,
								BZDB->get("infoMulticastInterface").c_str(),
								"w", &pingOutAddr);

	// open broadcast
	if (pingBcastSocket == -1)
		pingBcastSocket = openBroadcast(BroadcastPort, NULL, &pingBcastAddr);
}

void					CommandsSearch::closePingSockets()
{
	closeMulticast(pingInSocket);
	closeMulticast(pingOutSocket);
	closeMulticast(pingBcastSocket);
	pingInSocket    = -1;
	pingOutSocket   = -1;
	pingBcastSocket = -1;
}

void					CommandsSearch::sendPing()
{
	int ttl = atoi(BZDB->get("infoNetworkTTL").c_str());
	if (ttl > 1 && pingInSocket != -1 && pingOutSocket != -1) {
		PingPacket::sendRequest(pingOutSocket, &pingOutAddr, ttl);
	}
	if (pingBcastSocket != -1) {
		PingPacket::sendRequest(pingBcastSocket, &pingBcastAddr, 1);
	}
}

void					CommandsSearch::clearListServers()
{
	for (ListServerList::iterator index = listServers.begin();
								index != listServers.end(); ++index)
		if (index->socket != -1)
			closesocket(index->socket);
	listServers.clear();
}

void					CommandsSearch::lookupListServers()
{
	// dereference URL
	BzfNetwork::URLList urls, failedURLs;
	urls.push_back(BZDB->get("infoListServerURL"));
	BzfNetwork::dereferenceURLs(urls, 5, failedURLs);

	// print urls we failed to open
	BzfNetwork::URLList::const_iterator index;
	for (index = failedURLs.begin(); index != failedURLs.end(); ++index)
		printError("Can't open list server: %s", index->c_str());

	// check urls for validity
	for (index = urls.begin(); index != urls.end(); ++index) {
		// parse url
		BzfString protocol, hostname, path;
		int port = ServerPort + 1;
		Address address;
		if (!BzfNetwork::parseURL(*index, protocol, hostname, port, path) ||
		protocol != "bzflist" || port < 1 || port > 65535 ||
		(address = Address::getHostAddress(hostname.c_str())).isAny()) {
		printError("Can't open list server: %s", index->c_str());
		continue;
		}

		// add to list
		ListServer server;
		server.address = address;
		server.port    = port;
		server.socket  = -1;
		server.phase   = SendRequest;
		listServers.push_back(server);
	}

	// do ConnectListServers phase only if we found a valid list server url
	if (!listServers.empty())
		phase = ConnectListServers;
	else
		phase = Idle;
}

void					CommandsSearch::connectToListServers()
{
	// back to idle
	phase = Idle;

	// connect (asynchronously) to each list server
	for (ListServerList::iterator index = listServers.begin();
								index != listServers.end(); ) {
		ListServer& listServer = *index;

		// create socket.  give up on failure.
		int fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd < 0) {
			printError("Can't create list server socket");
			listServers.erase(index);
			continue;
		}

		// set to non-blocking.  we don't want to wait for the connection.
		if (BzfNetwork::setNonBlocking(fd) < 0) {
			printError("Error with list server socket");
			closesocket(fd);
			listServers.erase(index);
			continue;
		}

		// start connection
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port   = htons(listServer.port);
		addr.sin_addr   = listServer.address;
		if (connect(fd, (CNCTType*)&addr, sizeof(addr)) < 0) {
#if defined(_WIN32)
#undef EINPROGRESS
#define EINPROGRESS EWOULDBLOCK
#endif
			if (getErrno() != EINPROGRESS) {
				printError("Can't connect list server socket");
				closesocket(fd);
				listServers.erase(index);
				continue;
			}
		}

		// save socket
		listServer.socket = fd;

		// keep this item
		++index;
	}
}

void					CommandsSearch::handleReplies()
{
	while (1) {
		// use zero timeout to poll
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		// prepare socket sets
		fd_set read_set, write_set;
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
		if (pingInSocket != -1)
			FD_SET(pingInSocket, &read_set);
		if (pingBcastSocket != -1)
			FD_SET(pingBcastSocket, &read_set);
		int fdMax = (pingInSocket > pingBcastSocket) ?
								pingInSocket : pingBcastSocket;

		// check for list server connection or data
		ListServerList::iterator index;
		for (index = listServers.begin(); index != listServers.end(); ++index) {
			const ListServer& listServer = *index;
			if (listServer.socket != -1) {
				if (listServer.phase == SendRequest)
					FD_SET(listServer.socket, &write_set);
				else if (listServer.phase == WaitForReply)
					FD_SET(listServer.socket, &read_set);
				if (listServer.socket > fdMax)
					fdMax = listServer.socket;
			}
		}

		// poll.  return on error or no sockets ready.
		const int nfound = select(fdMax + 1, &read_set, &write_set, 0, &timeout);
		if (nfound <= 0)
			break;

		// check broadcast and multicast sockets
		GameServer serverInfo;
		if (pingInSocket != -1 && FD_ISSET(pingInSocket, &read_set))
			if (serverInfo.ping.read(pingInSocket, NULL))
				addGameServerWithLookup(serverInfo);
		if (pingBcastSocket != -1 && FD_ISSET(pingBcastSocket, &read_set))
			if (serverInfo.ping.read(pingBcastSocket, NULL))
				addGameServerWithLookup(serverInfo);

		// check list servers
		for (index = listServers.begin(); index != listServers.end(); ++index) {
			ListServer& listServer = *index;
			if (listServer.socket == -1)
				continue;

			// read more data from server
			if (FD_ISSET(listServer.socket, &read_set)) {
				char buffer[1024];
				int n = recv(listServer.socket, buffer, sizeof(buffer), 0);
				if (n > 0) {
					// more data
					listServer.buffer.append(buffer, n);
				}
				else if (n == 0) {
					// server hungup
					parseListServerResponse(listServer);
					closesocket(listServer.socket);
					listServer.socket = -1;
					listServer.buffer = "";
				}
				else {
					// error on socket
					closesocket(listServer.socket);
					listServer.socket = -1;
				}
			}

			// send list request
			else if (FD_ISSET(listServer.socket, &write_set)) {
				static const char* msg = "LIST\n\n";

				// ignore SIGPIPE for this send
				PlatformFactory::SignalHandler oldPipeHandler =
								PLATFORM->signalCatch(kSigPIPE, kSigIGN);

				// send request
				if (send(listServer.socket, msg, strlen(msg), 0) != (int)strlen(msg)) {
					// probably unable to connect to server
					closesocket(listServer.socket);
					listServer.socket = -1;
				}
				else {
					listServer.phase = WaitForReply;
				}

				// restore signal
				PLATFORM->signalCatch(kSigPIPE, oldPipeHandler);
			}
		}
	}
}

void					CommandsSearch::parseListServerResponse(
								const ListServer& listServer)
{
	// copy buffer to scratch space
	char* buffer = new char[listServer.buffer.size() + 1];
	memcpy(buffer, listServer.buffer.c_str(), listServer.buffer.size() + 1);

	// parse
	char* base = buffer;
	while (*base) {
		// find next newline
		char* scan = base;
		while (*scan && *scan != '\n')
			scan++;

		// if no newline then no more complete replies
		if (*scan != '\n')
			break;
		*scan++ = '\0';

		// parse server info
		char *scan2, *name, *version, *info, *address, *title;
		name = base;
		version = name;
		while (*version && !isspace(*version))  version++;
		while (*version &&  isspace(*version)) *version++ = 0;
		info = version;
		while (*info && !isspace(*info))  info++;
		while (*info &&  isspace(*info)) *info++ = 0;
		address = info;
		while (*address && !isspace(*address))  address++;
		while (*address &&  isspace(*address)) *address++ = 0;
		title = address;
		while (*title && !isspace(*title))  title++;
		while (*title &&  isspace(*title)) *title++ = 0;

		// extract port number from address
		int port = ServerPort;
		scan2 = strchr(name, ':');
		if (scan2) {
			port = atoi(scan2 + 1);
			*scan2 = 0;
		}

		// check info
		if (strncmp(version, ServerVersion, 7) == 0 &&
		(int)strlen(info) == PingPacketHexPackedSize &&
		port >= 1 && port <= 65535) {
			// store info
			GameServer serverInfo;
			serverInfo.ping.unpackHex(info);
			int dot[4] = {127,0,0,1};
			if (sscanf(address, "%d.%d.%d.%d", dot+0, dot+1, dot+2, dot+3) == 4) {
				if (dot[0] >= 0 && dot[0] <= 255 &&
		    dot[1] >= 0 && dot[1] <= 255 &&
		    dot[2] >= 0 && dot[2] <= 255 &&
		    dot[3] >= 0 && dot[3] <= 255) {
					InAddr addr;
					unsigned char* paddr = (unsigned char*)&addr.s_addr;
					paddr[0] = (unsigned char)dot[0];
					paddr[1] = (unsigned char)dot[1];
					paddr[2] = (unsigned char)dot[2];
					paddr[3] = (unsigned char)dot[3];
					serverInfo.ping.serverId.serverHost = addr;
				}
			}
			serverInfo.ping.serverId.port = htons((int16_t)port);
			serverInfo.name  = name;
			serverInfo.name += BzfString::format(":%d", port);

			// construct description
			serverInfo.description = serverInfo.name;
			if (strlen(title) > 0) {
				serverInfo.description += "; ";
				serverInfo.description += title;
			}

			// add to list
			addGameServer(serverInfo);
		}

		// next reply
		base = scan;
	}

	delete[] buffer;
}

void					CommandsSearch::addGameServerWithLookup(
								GameServer& info)
{
	// get host name
	info.name = Address::getHostByAddress(info.ping.serverId.serverHost);

	// tack on port number
	int port = (int)ntohs(info.ping.serverId.port);
	if (port == 0)
		port = ServerPort;
	info.name += BzfString::format(":%d", port);

	// the description is the name
	info.description = info.name;

	addGameServer(info);
}

void					CommandsSearch::addGameServer(GameServer& info)
{
	// update list if we already have the server
	GameServerList::iterator index;
	for (index = gameServers.begin(); index != gameServers.end(); ++index) {
		GameServer& server = *index;
		if (server.ping.serverId.serverHost.s_addr ==
								info.ping.serverId.serverHost.s_addr &&
		server.ping.serverId.port == info.ping.serverId.port) {

			// update description if new description is longer.  this
			// allows servers found via broadcast/multicast to get the
			// list server description should it arrive afterwards.
			if (server.description.size() < info.description.size())
				server.description = info.description;

			// update
			if (BZDB->get("_serverName") == server.name)
				onServerName(server.name);

			break;
		}
	}

	// add server if we don't already have it
	if (index == gameServers.end()) {
		gameServers.push_back(info);
		BZDB->set("_serverCount", BzfString::format("%u", gameServers.size()));
	}

	// sort by number of players
	const unsigned int n = gameServers.size();
	for (unsigned int i = 0; i < n - 1; ++i) {
		// find server that should be at position i
		unsigned int indexWithMin = i;
		for (unsigned int j = i + 1; j < n; ++j)
			if (getPlayerCount(gameServers[j]) >
		  getPlayerCount(gameServers[indexWithMin]))
				indexWithMin = j;

		// swap
		GameServer tmp            = gameServers[i];
		gameServers[i]            = gameServers[indexWithMin];
		gameServers[indexWithMin] = tmp;
	}

	// set server list
	BzfString servers;
	for (index = gameServers.begin(); index != gameServers.end(); ++index) {
		GameServer& server = *index;
		servers += server.description;
		servers += "\n";
		servers += server.name;
		servers += "\n";
	}
	BZDB->set("_serverList", servers);
}

int						CommandsSearch::getPlayerCount(const GameServer& info)
{
	const PingPacket& item = info.ping;
	return item.rogueCount +
		item.redCount +
		item.greenCount +
		item.blueCount +
		item.purpleCount;
}
