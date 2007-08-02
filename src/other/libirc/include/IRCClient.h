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

// libIRC Client header

#ifndef _IRC_CLIENT_H_
#define _IRC_CLIENT_H_

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
class IRCClient;

// base command handler for any command
class IRCClientCommandHandler
{
public:
	IRCClientCommandHandler(){return;}
	virtual ~IRCClientCommandHandler(){return;}

	// called when the system wishes to know the name of this command
	virtual std::string getCommandName ( void ){return name;}

	// the send and receve methods return true if the default handler is to be called
	// it is recomended that the default ALWAYS be called, as it often sets internal data for other mesages

	// called when the client receves a command of this type
	virtual bool receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info ){return true;}

	// called when the user wishes to send a command of this type
	virtual bool send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info ){return true;}
protected:
	std::string name;
};

class IRCClientLogHandler
{
public:
	virtual ~IRCClientLogHandler(){return;}
	virtual void log ( IRCClient &client, int level, std::string line ) = 0;
};

class IRCClient : public TCPClientDataPendingListener, IRCBasicEventCallback
{
public:
	IRCClient();
	virtual ~IRCClient();

	// logging
	void	setLogHandler ( IRCClientLogHandler * logger );

	virtual void setLogfile ( std::string file );
	virtual std::string  getLogfile ( void );

	virtual void setDebugLevel ( int level );
	virtual int getDebugLevel ( void );

	// general connection methods
	virtual bool init ( void );
	virtual bool connect ( std::string server, int port );
	virtual bool disconnect ( std::string reason );

	void setFloodProtectTime ( float time ){minCycleTime = time;}
	float getFloodProtectTime ( void ){return minCycleTime;}

	// update methods
	virtual bool process ( void );

	// basic IRC operations
	virtual bool login ( std::string &nick, std::string &username, std::string &fullname, std::string &host);
	virtual bool join ( std::string channel );
	virtual bool part ( std::string channel, std::string reason );
	virtual bool sendMessage ( std::string target, std::string message, bool isAction = false );

	// CTCP operations
	virtual bool sendCTCPRequest ( std::string target, teCTCPCommands command, std::string &data);
	virtual bool sendCTCPReply ( std::string target, teCTCPCommands command, std::string &data);

	// name operations
	virtual bool changeNick ( std::string &nick );

	// IRC chanops
	virtual bool kick ( std::string user, std::string channel, std::string reason );

	virtual bool mode ( std::string theMode, std::string target, std::string option );
	// user modes in chanel
	virtual bool ban ( std::string mask, std::string channel );
	virtual bool unban ( std::string mask, std::string channel );
	virtual bool voice ( std::string user, std::string channel );
	virtual bool devoice ( std::string user, std::string channel );
	virtual bool op ( std::string user, std::string channel );
	virtual bool deop ( std::string user, std::string channel );
	virtual bool quiet ( std::string user, std::string channel );
	virtual bool unquiet ( std::string user, std::string channel );
	// channel modes
	virtual bool privateChannel ( std::string channel );
	virtual bool unprivateChannel ( std::string channel );
	virtual bool moderateChannel ( std::string channel );
	virtual bool unmoderateChannel ( std::string channel );
	virtual bool secretChannel ( std::string channel );
	virtual bool unsecretChannel ( std::string channel );
	virtual bool messageLockChannel ( std::string channel );
	virtual bool unmessageLockChannel ( std::string channel );
	virtual bool topicLockChannel ( std::string channel );
	virtual bool untopicLockChannel ( std::string channel );
	virtual bool inviteLockChannel ( std::string channel );
	virtual bool uninviteLockChannel ( std::string channel );
	virtual bool setChannelUserLimit ( std::string channel, int limit );
	virtual bool removeChannelUserLimit ( std::string channel );
	virtual bool setChannelKey ( std::string channel, std::string key );
	virtual bool removeChannelKey ( std::string channel );

	// IRC info operations
	virtual string_list listUsers ( std::string channel );
	virtual string_list listChanels ( void );
	virtual string_list listChanOps ( std::string channel );

	virtual trIRCChannelPermisions getChanPerms ( std::string channel );

	IRCUserManager& getUserManager ( void ){return userManager;}

	//event handler methods.... for higher level API
	virtual bool registerEventHandler ( teIRCEventType eventType, IRCBasicEventCallback *handler );
	virtual bool removeEventHandler ( teIRCEventType eventType, IRCBasicEventCallback *handler );
	virtual void callEventHandler ( teIRCEventType eventType, trBaseEventInfo &info );

	//command handler methods... for lower level API
	virtual bool registerCommandHandler ( IRCClientCommandHandler *handler );
	virtual bool removeCommandHandler ( IRCClientCommandHandler *handler );
	virtual int listUserHandledCommands ( std::vector<std::string> &commandList );
	virtual int listDefaultHandledCommands ( std::vector<std::string> &commandList );

	// command sending and receving methods called by handlers
	virtual bool sendCommand ( std::string &commandName, BaseIRCCommandInfo &info );
	virtual bool sendIRCCommand ( teIRCCommands	command, IRCCommandINfo &info );
	virtual bool sendCTMPCommand ( teCTCPCommands	command, CTCPCommandINfo &info );

	virtual bool receveCommand ( std::string &commandName, BaseIRCCommandInfo &info );
	virtual bool receveIRCCommand ( teIRCCommands	command, IRCCommandINfo &info );
	virtual bool receveCTMPCommand ( teCTCPCommands	command, CTCPCommandINfo &info );

	// --------------------------------------------------------------------------------------
	// generaly not called by the client app

	// called by the TCP/IP connection when we get data
	virtual void pending ( TCPClientConnection *connection, int count );

	// tutilitys generaly used only by command handlers
	// data sending stuff
	virtual bool sendIRCCommandToServer ( teIRCCommands	command, std::string &data);

	// the most RAWEST data transfer
	virtual bool sendTextToServer ( std::string &text );

	// low level log calls
	virtual void log ( std::string text, int level = 0 );
	virtual void log ( const char *text, int level = 0 );

	// info returned from IRC sessions, used to maintain the internal state, and dispatch high level events from low level messages
	void setServerHost ( std::string host ) {host = reportedServerHost;}
	std::string getServerHost ( void ){return reportedServerHost;}
	std::string getMOTD ( void ){return MOTD;}
	void setNick ( std::string text ) {nickname=text;}
	std::string getNick ( void ) {return nickname;}

	// used by the raw IRC command Handlers to update internal states and trigger events
	void noticeMessage ( trMessageEventInfo	&info );
	void welcomeMessage ( trMessageEventInfo	&info );
	void beginMOTD ( void ){MOTD = "";}
	void addMOTD ( std::string line ) {MOTD += line + std::string("\n");}
	void endMOTD ( void );
	void joinMessage ( BaseIRCCommandInfo	&info );
	void partMessage ( BaseIRCCommandInfo	&info );
	void setChannelMode ( std::string channel, std::string mode );
	void setChannelTopicMessage ( std::string channel, std::string topic, std::string source );
	void addChannelUsers ( std::string channel, string_list newUsers );
	void endChannelUsersList ( std::string channel );
	void privMessage ( BaseIRCCommandInfo	&info );
	void modeCommand ( BaseIRCCommandInfo	&info );
	void nickNameError ( int error, std::string message );
	void nickCommand ( BaseIRCCommandInfo	&info );
	void kickCommand ( BaseIRCCommandInfo	&info );
	void QuitMessage ( BaseIRCCommandInfo	&info );

	// used by the defalt event handlers
	bool process ( IRCClient &ircClient, teIRCEventType	eventType, trBaseEventInfo &info );

protected:
	friend class IRCClientCommandHandler;

	// networking
	TCPClientConnection		*tcpClient;	
	TCPConnection					&tcpConnection;

	// irc data
	std::string						ircServerName;
	std::string						reportedServerHost;
	unsigned short				ircServerPort;
	std::string						lastRecevedData;

	// IRC "constants"
	std::string		ircMessageTerminator;
	std::string		ircCommandDelimator;

	// the wonderful connection state
	typedef enum
	{
		eNotConnected = 0,
		eTCPConenct,
		eSentNickAndUSer,
		eLoggedIn,
		eLastState
	}teIRCConnectionState;

	teIRCConnectionState	ircConnectionState;

	bool									registered;

	virtual teIRCConnectionState getConnectionState ( void ){return ircConnectionState;}
	virtual void setConnectionState ( teIRCConnectionState state ){ircConnectionState = state;}

	// receved data processing
	void processIRCLine ( std::string line );

	// the command handlers
	typedef std::map<std::string, IRCClientCommandHandler*>	tmCommandHandlerMap;
	typedef std::map<std::string, std::vector<IRCClientCommandHandler*> >	tmUserCommandHandlersMap;

	tmCommandHandlerMap			defaultCommandHandlers;
	tmUserCommandHandlersMap	userCommandHandlers;

	void addDefaultCommandhandlers ( IRCClientCommandHandler* handler );
	void clearDefaultCommandhandlers ( void );
	void registerDefaultCommandhandlers ( void );

	// event handlers
	tmIRCEventMap							defaultEventHandlers;
	tmIRCEventListMap					userEventHandlers;

	void addDefaultEventHandlers ( teIRCEventType eventType, IRCBasicEventCallback* handler );
	void clearDefaultEventHandlers ( void );
	void registerDefaultEventHandlers ( void );

	// loging
	IRCClientLogHandler				*logHandler;
	std::string								logfile;
	int												debugLogLevel;

	// info from the connection
	std::string								MOTD;
	std::string								requestedNick;
	std::string								nickname;

	IRCUserManager						userManager;
	// flood protection
	float											minCycleTime;
};

#endif //_IRC_CLIENT_H_
