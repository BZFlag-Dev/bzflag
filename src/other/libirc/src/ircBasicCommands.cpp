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

// basic IRC commands

#include "ircBasicCommands.h"
#include "IRCTextUtils.h"
#include "IRCEvents.h"

std::string delim = " ";

// IRC "NICK" command

IRCNickCommand::IRCNickCommand()
{
	name = "NICK";
}

bool IRCNickCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	client.nickCommand(info);
	return true;
}

bool IRCNickCommand::send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	IRCCommandINfo	&ircInfo = (IRCCommandINfo&)info;

	std::string commandLine;

	// NICK
	commandLine = ircInfo.params[0];
	client.sendIRCCommandToServer(eCMD_NICK,commandLine);

	return true;
}

// IRC "USER" command

IRCUserCommand::IRCUserCommand()
{
	name = "USER";
}

bool IRCUserCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	return true;
}

bool IRCUserCommand::send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{	
	IRCCommandINfo	&ircInfo = (IRCCommandINfo&)info;

	std::string commandLine;

	//username host server fullname
	commandLine = ircInfo.params[0] + delim + ircInfo.params[1] + delim + ircInfo.params[2] + delim + std::string(":") + ircInfo.params[3];
	client.sendIRCCommandToServer(eCMD_USER,commandLine);

	return true;
}

// IRC "PING" command
IRCPingCommand::IRCPingCommand()
{
	name = "PING";
}

bool IRCPingCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	IRCCommandINfo	ircInfo;
	ircInfo.command = eCMD_PONG;
	client.sendIRCCommand(eCMD_PONG,ircInfo);
	return true;
}

bool IRCPingCommand::send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	IRCCommandINfo	&ircInfo = (IRCCommandINfo&)info;

	std::string commandLine;
	// PING
	client.sendIRCCommandToServer(eCMD_PING,commandLine);
	return true;
}

// IRC "PONG" command
IRCPongCommand::IRCPongCommand()
{
	name = "PONG";
}

bool IRCPongCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	// we do nothing on a pong
	return true;
}

bool IRCPongCommand::send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	IRCCommandINfo	&ircInfo = (IRCCommandINfo&)info;

	std::string commandLine;
	// PING
	client.sendIRCCommandToServer(eCMD_PONG,commandLine);
	return true;
}

// IRC "NOTICE" command
IRCNoticeCommand::IRCNoticeCommand()
{
	name = "NOTICE";
}

bool IRCNoticeCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	trMessageEventInfo	messageInfo;

	messageInfo.eventType = eIRCNoticeEvent;
	messageInfo.source = info.source;
	messageInfo.params = info.params;
	messageInfo.message = info.getAsString();
	client.noticeMessage(messageInfo);
	return true;
}

bool IRCNoticeCommand::send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	// we do nothing on a pong
	return true;
}

// IRC "JOIN" command
IRCJoinCommand::IRCJoinCommand()
{
	name = "JOIN";
}

bool IRCJoinCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	client.joinMessage(info);
	return true;	
}

bool IRCJoinCommand::send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	std::string commandLine;

	if (info.params.size())
	{
	string_list::iterator itr = info.params.begin();

		while ( itr != info.params.end() )
		{
			commandLine += *itr;
			itr++;

			if (itr != info.params.end())
				commandLine += ",";
		}
	}
	else
		commandLine = info.target;

	// JOIN CHANNEL1,CHANNEL2,....,CHANNELN
	client.sendIRCCommandToServer(eCMD_JOIN,commandLine);
	return true;
}

// IRC "PART" command
IRCPartCommand::IRCPartCommand()
{
	name = "PART";
}

bool IRCPartCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	client.partMessage(info);
	return true;	
}

bool IRCPartCommand::send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	std::string commandLine;

	commandLine = info.target + delim + info.params[0];

	// PART CHANNEL REASON
	client.sendIRCCommandToServer(eCMD_PART,commandLine);
	return true;
}

// IRC "QUIT" command

IRCQuitCommand::IRCQuitCommand()
{
	name = "QUIT";
}

bool IRCQuitCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	client.QuitMessage(info);
	return true;
}

bool IRCQuitCommand::send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	std::string commandLine;

	commandLine = info.params[0];

	// QUIT CHANNEL REASON
	client.sendIRCCommandToServer(eCMD_QUIT,commandLine);
	return true;

}

// IRC "MODE" command
IRCModeCommand::IRCModeCommand()
{
	name = "MODE";
}

bool IRCModeCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	// we got a mode message, see what the deal is
	client.modeCommand(info);
	return true;	
}

bool IRCModeCommand::send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
  // MODE TARGET modes
	std::string modeline = info.target;

	if ( info.params.size())
	{
		modeline+= delim + info.params[0];

		if ( info.params.size() > 1)
			modeline+= delim + info.params[1];
	}
  client.sendIRCCommandToServer(eCMD_MODE, modeline);
  return true;
}

// IRC "PRIVMSG" command
IRCPrivMsgCommand::IRCPrivMsgCommand()
{
	name = "PRIVMSG";
}

bool IRCPrivMsgCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	client.privMessage(info);
	return true;
}

bool IRCPrivMsgCommand::send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	IRCCommandINfo	&ircInfo = (IRCCommandINfo&)info;

	std::string commandLine;
	ircInfo.target = 
	//username host server fullname
	commandLine = ircInfo.target + delim + std::string(":") + info.getAsString();
	client.sendIRCCommandToServer(eCMD_PRIVMSG,commandLine);
	return true;
}

// IRC "KICK" command

IRCKickCommand::IRCKickCommand()
{
	name = "KICK";
}

bool IRCKickCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	client.kickCommand(info);
	return true;
}

bool IRCKickCommand::send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	IRCCommandINfo	&ircInfo = (IRCCommandINfo&)info;

	std::string commandLine;

	// KICK target user :reason
	commandLine = ircInfo.target + delim + ircInfo.params[0]+ delim + std::string(":") + info.getAsString(1);
	client.sendIRCCommandToServer(eCMD_KICK,commandLine);

	return true;
}

	// special case commands

// Generic handler for ALL
IRCALLCommand::IRCALLCommand()
{
	name = "ALL";
}

bool IRCALLCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	// just log it out
	client.log(string_util::format("ALL::command %s from %s for %s containing %s",info.command.c_str(),info.source.c_str(),info.target.c_str(),info.getAsString().c_str()),4);
	client.log(string_util::format("ALL::raw %s",info.raw.c_str()),6);
	client.log(std::string(" "),6);
	return true;
}

bool IRCALLCommand::send ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	// just log it out
	client.log(string_util::format("ALL::command %s: to server containing %s",command.c_str(),info.getAsString().c_str()),4);
	return true;
}

// numerics

IRCNumericCommand::IRCNumericCommand()
{
	name = "NUMERIC";
}

bool IRCNumericCommand::receve ( IRCClient &client, std::string &command, BaseIRCCommandInfo	&info )
{
	int numeric = atoi(info.command.c_str());

	switch(numeric)
	{
		case 0:
			client.log(string_util::format("NUMERIC::Unknown code: %s",info.command.c_str()),2);
		break;

		case RPL_WELCOME:// "Welcome to the Internet Relay Network <nick>!<user>@<host>"
			{
				trMessageEventInfo	messageInfo;

				messageInfo.eventType = eIRCNoticeEvent;
				messageInfo.target = info.target;
				messageInfo.source = info.source;
				messageInfo.params = info.params;
				messageInfo.message = info.getAsString();

				client.welcomeMessage(messageInfo);
			}
			break;

		case RPL_YOURHOST: //"Your host is <servername>, running version <ver>"
			client.setServerHost(info.params[3]);
			break;

		case RPL_CREATED: //"This server was created <date>"
		case RPL_MYINFO: //"<servername> <version> <available user modes> <available channel modes>"
		case RPL_BOUNCE: //	"Try server <server name>, port <port number>"
		case RPL_TRACELINK: //"Link <version & debug level> <destination> <next server> V<protocol version> <link uptime in seconds> <backstream sendq> <upstream sendq>"
		case RPL_TRACECONNECTING: //"Try. <class> <server>"
		case RPL_TRACEHANDSHAKE: //"H.S. <class> <server>"
		case RPL_TRACEUNKNOWN:
		case RPL_TRACEOPERATOR:
		case RPL_TRACEUSER:
		case RPL_TRACESERVER:
		case RPL_TRACESERVICE:
		case RPL_TRACENEWTYPE:
		case RPL_TRACECLASS:
		case RPL_TRACERECONNECT:
		case RPL_STATSLINKINFO:
		case RPL_STATSCOMMANDS:
		case RPL_ENDOFSTATS:
		case RPL_UMODEIS:
		case RPL_SERVLIST:
		case RPL_SERVLISTEND:
		case RPL_STATSUPTIME:
		case RPL_STATSOLINE:
		case RPL_LUSERCLIENT:
		case RPL_LUSEROP:
		case RPL_LUSERUNKNOWN:
		case RPL_LUSERCHANNELS:
		case RPL_LUSERME:
		case RPL_ADMINME:
		case RPL_ADMINLOC1:
		case RPL_ADMINLOC2:
		case RPL_ADMINEMAIL:
		case RPL_TRACELOG:
		case RPL_TRACEEND:
		case RPL_TRYAGAIN:
		case RPL_AWAY:
		case RPL_USERHOST:
		case RPL_ISON:
		case RPL_UNAWAY:
		case RPL_NOWAWAY:
		case RPL_WHOISUSER:
		case RPL_WHOISSERVER:
		case RPL_WHOISOPERATOR:
		case RPL_WHOWASUSER:
		case RPL_ENDOFWHO:
		case RPL_WHOISIDLE:
		case RPL_ENDOFWHOIS:
		case RPL_WHOISCHANNELS:
		case RPL_LISTSTART:
		case RPL_LIST:
		case RPL_LISTEND:
			break;

		case RPL_CHANNELMODEIS:
			{	
				// first string is the channel this is for
				std::string channel = info.params[0];

				// let the big guy know we got some
				client.setChannelMode(channel,info.params[1]);
			}
			break;
		case RPL_UNIQOPIS:
		case RPL_NOTOPIC:
			break;

		case RPL_TOPIC:
			{	
				// first string is the channel this is for
				std::string channel = info.params[0];

				// get that topic
				std::string topic = info.getAsString(1);
				// the first thing is a : we don't want that ether
				topic.erase(topic.begin());

				// let the big guy know we got some
				client.setChannelTopicMessage(channel,topic,info.source);
			}
			break;

		case RPL_INVITING:
		case RPL_SUMMONING:
		case RPL_INVITELIST:
		case RPL_ENDOFINVITELIST:
		case RPL_EXCEPTLIST:
		case RPL_ENDOFEXCEPTLIST:
		case RPL_VERSION:
		case RPL_WHOREPLY:
			break;

		case RPL_NAMREPLY:
			{	
				// first string is the channel this is for
				std::string channel = info.params[1];

				string_list userList = info.params;
				// we don't need the = channel, we saved it
				userList.erase(userList.begin(),userList.begin()+2);

				// the first thing is a : we don't want that ether
				userList[0].erase(userList[0].begin());

				// let the big guy know we got some
				client.addChannelUsers(channel,userList);
			}
			break;
		case RPL_LINKS:
		case RPL_ENDOFLINKS:
			break;

		case RPL_ENDOFNAMES:
			client.endChannelUsersList(info.params[0]);
			break;

		case RPL_BANLIST:
		case RPL_ENDOFBANLIST:
		case RPL_ENDOFWHOWAS:
		case RPL_INFO:
			break;

		case RPL_MOTD:
				client.addMOTD(info.getAsString());
			break;

		case RPL_ENDOFINFO:
			//client.endMOTD();
			break;

		case RPL_MOTDSTART:
			client.beginMOTD();
			break;
		
		case RPL_ENDOFMOTD:
			client.endMOTD();
			break;

		case RPL_YOUREOPER:
		case RPL_REHASHING:
		case RPL_YOURESERVICE:
		case RPL_TIME:
		case RPL_USERSSTART:
		case RPL_USERS:
		case RPL_ENDOFUSERS:
		case RPL_NOUSERS:
		case ERR_NOSUCHNICK:
		case ERR_NOSUCHSERVER:
		case ERR_NOSUCHCHANNEL:
		case ERR_CANNOTSENDTOCHAN:
		case ERR_TOOMANYCHANNELS:
		case ERR_WASNOSUCHNICK:
		case ERR_TOOMANYTARGETS:
		case ERR_NOSUCHSERVICE:
		case ERR_NOORIGIN:
		case ERR_NORECIPIENT:
		case ERR_NOTEXTTOSEND:
		case ERR_NOTOPLEVEL:
		case ERR_WILDTOPLEVEL:
		case ERR_BADMASK:
		case ERR_UNKNOWNCOMMAND:
		case ERR_NOMOTD:
		case ERR_NOADMININFO:
		case ERR_FILEERROR:
			break;

		case ERR_NONICKNAMEGIVEN:
		case ERR_ERRONEUSNICKNAME:
		case ERR_NICKNAMEINUSE:
		case ERR_NICKCOLLISION:
			client.nickNameError(numeric,info.getAsString());
			break;

		case ERR_UNAVAILRESOURCE:
		case ERR_USERNOTINCHANNEL:
		case ERR_NOTONCHANNEL:
		case ERR_USERONCHANNEL:
		case ERR_NOLOGIN:
		case ERR_SUMMONDISABLED:
		case ERR_USERSDISABLED:
		case ERR_NOTREGISTERED:
		case ERR_NEEDMOREPARAMS:
		case ERR_ALREADYREGISTRED:
		case ERR_NOPERMFORHOST:
		case ERR_PASSWDMISMATCH:
		case ERR_YOUREBANNEDCREEP:
		case ERR_YOUWILLBEBANNED:
		case ERR_KEYSET:
		case ERR_CHANNELISFULL:
		case ERR_UNKNOWNMODE:
		case ERR_INVITEONLYCHAN:
		case ERR_BANNEDFROMCHAN:
		case ERR_BADCHANNELKEY:
		case ERR_BADCHANMASK:
		case ERR_NOCHANMODES:
		case ERR_BANLISTFULL:
		case ERR_NOPRIVILEGES:
		case ERR_CHANOPRIVSNEEDED:
		case ERR_CANTKILLSERVER:
		case ERR_RESTRICTED:
		case ERR_UNIQOPPRIVSNEEDED:
		case ERR_NOOPERHOST:
		case ERR_UMODEUNKNOWNFLAG:
		case ERR_USERSDONTMATCH:
		default:
			client.log(string_util::format("NUMERIC::code: %d",numeric),4);
			break;

	}
	return true;
}



