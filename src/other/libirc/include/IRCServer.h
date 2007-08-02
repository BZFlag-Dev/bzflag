/* libIRC
* Copyright (c) 2004 Christopher Sean Morrison
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named LICENSE that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

// libIRC Server header

#ifndef _IRC_SERVER_H_
#define _IRC_SERVER_H_

// IRC includes
#include "libIRC.h"
#include "ircCommands.h"
#include "IRCEvents.h"
#include "TCPConnection.h"
#include "IRCUserManager.h"

// global includes
#include <string>
#include <vector>
#include <map>

// need this later
class IRCServer;

// info that is passed to a command handler
// handles standard commands and CTCP

class IRCServerLogHandler
{
public:
	virtual ~IRCServerLogHandler(){return;}
	virtual void log ( IRCServer &server, int level, std::string line ) = 0;
};

class IRCServerConnectedClient
{
public:
	IRCServerConnectedClient ( IRCServer *_server, TCPServerConnectedPeer* _peer );
	~IRCServerConnectedClient();

	unsigned int getClientID ( void ) { return clientID;}
	bool sendText ( const std::string &text );
	std::string lastData;

	std::string getHostMask ( void );
	bool getIP ( unsigned char ip[4] );

protected:
	unsigned int clientID;
	TCPServerConnectedPeer	*peer;
	IRCServer *server;
};

class IRCServer : public TCPServerDataPendingListener
{
public:
	IRCServer();
	virtual ~IRCServer();

	// loging
	void	setLogHandler ( IRCServerLogHandler * logger );

	virtual void setLogfile ( std::string file );
	virtual std::string  getLogfile ( void );

	virtual void setDebugLevel ( int level );
	virtual int getDebugLevel ( void );

	// general connection methods
	virtual bool listen ( int maxConnections = 32, int port = -1 );
	virtual bool disconnect ( std::string reason );

	void setFloodProtectTime ( float time ){minCycleTime = time;}
	float getFloodProtectTime ( void ){return minCycleTime;}

	// update methods
	virtual bool process ( void );

	// low level log calls
	virtual void log ( std::string text, int level = 0 );
	virtual void log ( const char *text, int level = 0 );

	virtual bool connect ( TCPServerConnection *connection, TCPServerConnectedPeer *peer );
	virtual void pending ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, unsigned int count );
	virtual void disconnect ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, bool forced = false );

	// virtual methods for basic IRC functions
	virtual bool allowConnection ( const char* hostmask, unsigned char ip[4] );
	virtual void clientConnect ( IRCServerConnectedClient *client );
	virtual void clientDisconnect ( IRCServerConnectedClient *client );
	virtual void clientIRCCommand ( const std::string &command, IRCServerConnectedClient *client );

protected:
	friend class IRCServerConnectedClient;

	bool sendTextToPeer ( const std::string &text, TCPServerConnectedPeer *peer );

	void processIRCLine ( std::string line, IRCServerConnectedClient *client );

	// networking
	TCPServerConnection		*tcpServer;	
	TCPConnection			&tcpConnection;
	int						ircServerPort;

	// loging
	IRCServerLogHandler				*logHandler;
	std::string						logfile;
	int								debugLogLevel;

	// helpers
	std::string ircMessageTerminator;
	std::string ircCommandDelimator;

	// flood protection
	float							minCycleTime;

	// users
	std::vector<IRCServerConnectedClient>	clients;

	std::vector<IRCServerConnectedClient>::iterator getClientItr ( IRCServerConnectedClient *client );
};

#endif //_IRC_SERVER_H_
