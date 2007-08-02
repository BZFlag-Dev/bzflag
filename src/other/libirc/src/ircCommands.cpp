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

#include "ircCommands.h"


IRCCommandParser	ircCommandParser;

// maps IRC commands to IDs and vice/versa
IRCCommandParser::IRCCommandParser()
{
	commandNameList.push_back("NULL");
	commandNameList.push_back(CMD_NICK);
	commandNameList.push_back(CMD_USER);
	commandNameList.push_back(CMD_PING);
	commandNameList.push_back(CMD_PONG);
	commandNameList.push_back(CMD_PRIVMSG);
	commandNameList.push_back(CMD_JOIN);
	commandNameList.push_back(CMD_PART);
	commandNameList.push_back(CMD_NOTICE);
	commandNameList.push_back(CMD_MODE);
	commandNameList.push_back(CMD_TOPIC);
	commandNameList.push_back(CMD_NAMES);
	commandNameList.push_back(CMD_LIST);
	commandNameList.push_back(CMD_INVITE);
	commandNameList.push_back(CMD_KICK);
	commandNameList.push_back(CMD_VERSION);
	commandNameList.push_back(CMD_STATS);
	commandNameList.push_back(CMD_LINKS);
	commandNameList.push_back(CMD_TIME);
	commandNameList.push_back(CMD_CONNECT);
	commandNameList.push_back(CMD_TRACE);
	commandNameList.push_back(CMD_ADMIN);
	commandNameList.push_back(CMD_INFO);
	commandNameList.push_back(CMD_WHO);
	commandNameList.push_back(CMD_WHOIS);
	commandNameList.push_back(CMD_WHOWAS);
	commandNameList.push_back(CMD_QUIT);
	commandNameList.push_back(CMD_OPER);
	commandNameList.push_back(CMD_KILL);
	commandNameList.push_back(CMD_AWAY);
	commandNameList.push_back(CMD_REHASH);
	commandNameList.push_back(CMD_RESTART);
	commandNameList.push_back(CMD_SUMMON);
	commandNameList.push_back(CMD_USERS);
	commandNameList.push_back(CMD_WALLOPS);
	commandNameList.push_back(CMD_USERHOST);
	commandNameList.push_back(CMD_ISON);
	commandNameList.push_back("NULL_LAST");
}

IRCCommandParser::~IRCCommandParser()
{
	commandNameList.clear();
}

std::string IRCCommandParser::getCommandName ( teIRCCommands id )
{
	std::string name;
	if ((int)commandNameList.size() <= id)
		return name;

	return commandNameList[id];
}

teIRCCommands IRCCommandParser::getCommandID ( std::string &name)
{
	int id = 0;

	std::vector<std::string>::iterator	itr = commandNameList.begin();
	while ( itr != commandNameList.end() )
	{
		if (name == *itr)
			return (teIRCCommands)id;
		itr++;
		id++;
	}
	return (teIRCCommands)0;
}


CTCPCommandParser	ctcpCommandParser;

// maps IRC commands to IDs and vice/versa
CTCPCommandParser::CTCPCommandParser()
{
	commandNameList.push_back("NULL");
	commandNameList.push_back(CMD_CTCP_ACTION);
	commandNameList.push_back(CMD_CTCP_VERSION);
	commandNameList.push_back(CMD_CTCP_PING);
	commandNameList.push_back(CMD_CTCP_PONG);
	commandNameList.push_back(CMD_CTCP_CLIENTINFO);
	commandNameList.push_back(CMD_CTCP_USERINFO);
	commandNameList.push_back(CMD_CTCP_TIME);
	commandNameList.push_back(CMD_CTCP_ERRMSG);
	commandNameList.push_back("NULL_LAST");
}

CTCPCommandParser::~CTCPCommandParser()
{
	commandNameList.clear();
}

std::string CTCPCommandParser::getCommandName ( teCTCPCommands id )
{
	return commandNameList[id];
}

teCTCPCommands CTCPCommandParser::getCommandID ( std::string &name)
{
	int id = 0;

	std::vector<std::string>::iterator	itr = commandNameList.begin();
	while ( itr != commandNameList.end() )
	{
		if (name == *itr)
			return (teCTCPCommands)id;
		itr++;
		id++;
	}
	return (teCTCPCommands)0;
}



