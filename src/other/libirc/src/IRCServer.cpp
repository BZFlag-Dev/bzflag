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

// implementation of main libIRC Server classes

//********************************************************************************//

#include "libIRC.h"
#include "ircBasicCommands.h"
#include "IRCTextUtils.h"

#ifndef _WIN32
	#include <unistd.h>
#else
	#include <windows.h>
	#include <time.h>
	#include <stdio.h>
#endif

class DefaultServerIRCLogHandler : public IRCServerLogHandler
{
public:
	virtual ~DefaultServerIRCLogHandler(){return;}
	virtual void log ( IRCServer &client, int level, std::string line )
	{
		printf("log# %d:%s\n",level,line.c_str());

		if (client.getLogfile().size())
		{
			FILE *fp = fopen(client.getLogfile().c_str(),"at");

			if (fp)
			{
				fprintf(fp,"log# %d:%s\n",level,line.c_str());
				fclose(fp);
			}
		}
	}
};

static DefaultServerIRCLogHandler	defaultLogger;


IRCServerConnectedClient::IRCServerConnectedClient ( IRCServer *_server, TCPServerConnectedPeer* _peer )
{
	peer = _peer;
	clientID = peer->getUID();
	server = _server;
}

IRCServerConnectedClient::~IRCServerConnectedClient()
{
}

bool IRCServerConnectedClient::sendText ( const std::string &text )
{ 
	if (!server)
		return false;

	return server->sendTextToPeer(text,peer);
}

std::string IRCServerConnectedClient::getHostMask ( void )
{
	if (!peer)
		return std::string();

	return peer->getHostMask();
}

bool IRCServerConnectedClient::getIP ( unsigned char ip[4] )
{
	if (!peer)
		return false;

	return peer->getIP(ip);
}

IRCServer::IRCServer()
:tcpConnection(TCPConnection::instance())
{
	tcpServer = NULL;

	debugLogLevel = 0;
	logHandler = &defaultLogger;

	ircMessageTerminator = "\r\n";
	ircCommandDelimator	 = " ";

	minCycleTime = 0.1f;
}

IRCServer::~IRCServer()
{
}

std::vector<IRCServerConnectedClient>::iterator IRCServer::getClientItr ( IRCServerConnectedClient *client )
{
	std::vector<IRCServerConnectedClient>::iterator itr = clients.begin();
	while ( itr != clients.end() )
	{	if ( &(*itr) == client )
			return itr;
		itr++;
	}

	return clients.end();
}

void IRCServer::setLogHandler ( IRCServerLogHandler * logger )
{
	if (!logger)
		logHandler = &defaultLogger;
	else
		logHandler = logger;
}

void IRCServer::setLogfile ( std::string file )
{
	logfile = file;
}

std::string  IRCServer::getLogfile ( void )
{
	return logfile;
}

void IRCServer::setDebugLevel ( int level )
{
	debugLogLevel = level;
}

int IRCServer::getDebugLevel ( void )
{
	return debugLogLevel;
}

bool IRCServer::listen ( int maxConnections, int port )
{
	if (tcpServer)
	{
		tcpServer->disconnect();
		tcpConnection.deleteServerConnection(tcpServer);
	}

	if (maxConnections < 0)
		maxConnections = 128;

	ircServerPort = _DEFAULT_IRC_PORT;
	if ( port > 0 )
		ircServerPort = (unsigned short)port;

	tcpServer = tcpConnection.newServerConnection(ircServerPort,maxConnections);
	tcpServer->addListener(this);

	return tcpServer->getLastError() == eTCPNoError;
}

bool IRCServer::disconnect ( std::string reason )
{
	if (tcpServer)
	{
		tcpServer->disconnect();
		tcpConnection.deleteServerConnection(tcpServer);
		tcpServer = NULL;
		return true;
	}

	return false;
}

bool IRCServer::process ( void )
{
	return tcpConnection.update() == eTCPNoError;
}

void IRCServer::log ( std::string text, int level )
{
	if (level <= debugLogLevel && logHandler)
		logHandler->log(*this,level,text);
}

void IRCServer::log ( const char *text, int level )
{
	log(std::string(text),level);
}

void IRCServer::processIRCLine ( std::string line, IRCServerConnectedClient *client )
{

}

bool IRCServer::sendTextToPeer ( const std::string &text, TCPServerConnectedPeer *peer )
{
	if (!peer || !tcpServer->listening())
		return false;

	std::string message = text;
	if (text.size())
	{
		message += ircMessageTerminator;
		teTCPError	error = peer->sendData(message);
		if (error == eTCPNoError)
			log("Send Data:" + text,2);
		else
		{
			switch (error)
			{
			case eTCPNotInit:
				log("Send Data Error: TCP Not Initalised: data=" + text,0);
				break;

			case eTCPSocketNFG:
				log("Send Data Error: Bad Socket: data=" + text,0);
				break;

			case eTCPDataNFG:
				log("Send Data Error: Bad Data",0);
				break;

			case eTCPConnectionFailed:
				log("Send Data Error: TCP Connection failed",0);
				break;

			default:
				log("Send Data Error:Unknown Error",0);
			}
			return false;
		}
	}
	else
		return false;

	// prevent that thar flooding
	IRCOSSleep(minCycleTime);
	return true;
}

bool IRCServer::connect ( TCPServerConnection *connection, TCPServerConnectedPeer *peer )
{
	if (!connection || !peer)
		return false;
	unsigned char ip[4];
	peer->getIP(ip);

	if (!allowConnection(peer->getHostMask().c_str(),ip))
		return false;

	unsigned int index = (unsigned int )clients.size();
	IRCServerConnectedClient	client(this,peer);

	clients.push_back(client);
	peer->setParam(&clients[index]);

	clientConnect(&clients[index]);

	return true;
}

void IRCServer::pending ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, unsigned int count )
{
	IRCServerConnectedClient* client = (IRCServerConnectedClient*)peer->getParam();
		if (!client)	// somehow out of band connection, screw it
			return;

	tvPacketList &packets = peer->getPackets();
	std::string theLine = client->lastData;

	for ( unsigned int p = 0; p < packets.size(); p++ )
	{
		TCPPacket	&packet = packets[p];

		unsigned int size;
		unsigned char*	data = packet.get(size);

		for ( unsigned int i = 0; i < size; i++ )
		{
			if ( data[i] != 13 )
				theLine += data[i];
			else
			{
				processIRCLine(theLine,client);
				theLine = "";
			}
		}
	}

	client->lastData = theLine;
	peer->flushPackets();
}

void IRCServer::disconnect ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, bool forced )
{
	IRCServerConnectedClient* client = (IRCServerConnectedClient*)peer->getParam();
	if (!client)	// somehow out of band connection, screw it
		return;

	clientDisconnect(client);

	std::vector<IRCServerConnectedClient>::iterator clientItr = getClientItr(client);
	if (clientItr != clients.end())
		clients.erase(clientItr);
}

// base IRC event handlers
void IRCServer::clientConnect ( IRCServerConnectedClient *client )
{
}

void IRCServer::clientDisconnect ( IRCServerConnectedClient *client )
{
}

bool IRCServer::allowConnection ( const char* hostmask, unsigned char ip[4] )
{
	return true;
}

void IRCServer::clientIRCCommand ( const std::string &command, IRCServerConnectedClient *client )
{

}




// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
