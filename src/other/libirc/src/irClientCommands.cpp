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

// implementation of main libIRC classes

//********************************************************************************//

#include "libIRC.h"
#include "ircBasicCommands.h"
#include "IRCTextUtils.h"

bool IRCClient::login ( std::string &nick, std::string &username, std::string &fullname, std::string &host )
{
	if (!tcpClient || !tcpClient->connected())
		return false;

	char	someNumber[64];
	sprintf(someNumber,"%d",rand());

	if (!nick.size())
		nick = std::string("SomeLazyUser") + std::string(someNumber);

	if (!username.size())
		username = "libIRCUser";

	if (!fullname.size())
		fullname = "Lazy libIRC programer";

	if (!host.size())
		host = "localhost";

	requestedNick = nick;

	IRCCommandINfo	info;
	info.params.push_back(nick);

	if (!sendIRCCommand(eCMD_NICK,info))
	{
		log("Login Failed: NICK command not sent",0);
		return false;
	}

	info.params.clear();
	info.params.push_back(username);
	info.params.push_back(host);
	info.params.push_back(ircServerName);
	info.params.push_back(fullname);

	if (!sendIRCCommand(eCMD_USER,info))
	{
		log("Login Failed: USER command not sent",0);
		return false;
	}

	if (getConnectionState() < eSentNickAndUSer)
		setConnectionState(eSentNickAndUSer);

	return  true;
}

bool IRCClient::changeNick ( std::string &nick )
{
	requestedNick = nick;

	IRCCommandINfo	info;
	info.params.push_back(nick);

	if (!sendIRCCommand(eCMD_NICK,info))
	{
		log("Login Failed: NICK command not sent",0);
		return false;
	}
	setNick(nick);
	return true;
}

bool IRCClient::join ( std::string channel )
{
	// we need to have at LEAST sent the username and stuff
	if (getConnectionState() < eSentNickAndUSer)
		return false;

	IRCCommandINfo	info;
	info.target = channel;
	if (!sendIRCCommand(eCMD_JOIN,info))
	{
		log("Join Failed: JOIN command not sent",0);
		return false;
	}

	if (!sendIRCCommand(eCMD_MODE,info))
	{
		log("Join Failed: MODE command not sent",0);
		return false;
	}

	return true;
}

bool IRCClient::part ( std::string channel, std::string reason )
{
	// we need to have at LEAST sent the username and stuff
	if (getConnectionState() < eSentNickAndUSer)
		return false;

	IRCCommandINfo	info;
	info.target = channel;
	info.params.push_back(reason);

	if (!sendIRCCommand(eCMD_PART,info))
	{
		log("part Failed: PART command not sent",0);
		return false;
	}

	// notify that we parted the channel
	std::string nick = getNick();
	userManager.userPartChannel(nick, channel);

	trPartEventInfo	eventInfo;

	eventInfo.eventType = eIRCChannelPartEvent;
	eventInfo.reason = reason;
	eventInfo.user = getNick();

	callEventHandler(eventInfo.eventType,eventInfo);

	// todo, we realy should go and remove the channel from our listing and kill any dead users
	return true;
}

bool IRCClient::sendMessage ( std::string target, std::string message, bool isAction )
{
	std::string messageHeader;
	std::string messageFooter;

	int sliceBoundry = 400;

	int headerLen = (int)strlen("PRIVMSG  :") + (int)target.size();
	if(isAction)
		headerLen += (int)strlen("*ACTION **");

	if(isAction)
		messageHeader += (char)0x01 + std::string("ACTION ");

	if(isAction)
		messageFooter +=(char)0x01;

	string_list	messages = string_util::slice(message,sliceBoundry-headerLen,true);

	string_list::iterator	itr = messages.begin();
	while ( itr != messages.end() )
	{
		std::string message = messageHeader+*itr+messageFooter;
		int len = (int)message.size();

		IRCCommandINfo	commandInfo;
		commandInfo.target = target;
		commandInfo.params.clear();
		commandInfo.params.push_back(message);
		sendIRCCommand(eCMD_PRIVMSG,commandInfo);
		itr++;
	}
	return true;
}

bool IRCClient::sendCTCPRequest ( std::string target, teCTCPCommands command, std::string &data)
{
      std::string message = CMD_PRIVMSG;
      message += " " + target + " :";
      message += CTCP_DELIMITER + ctcpCommandParser.getCommandName(command);
      if (data != "")
	    message += " " + data;
      message += CTCP_DELIMITER;
      return sendTextToServer(message);
}

bool IRCClient::sendCTCPReply ( std::string target, teCTCPCommands command, std::string &data)
{
      std::string message = CMD_NOTICE;
      message += " " + target + " :";
      message += CTCP_DELIMITER + ctcpCommandParser.getCommandName(command);
      if (data != "")
	    message += " " + data;
      message += CTCP_DELIMITER;
      return sendTextToServer(message);
}

bool IRCClient::kick ( std::string user, std::string channel, std::string reason )
{
	// we need to have at LEAST sent the username and stuff
	if (getConnectionState() < eSentNickAndUSer)
		return false;

	if (!userManager.userInChannel(user,channel))
		return false;

	IRCCommandINfo	info;
	info.target = channel;
	info.params.push_back(user);
	info.params.push_back(reason);

	if (!sendIRCCommand(eCMD_KICK,info))
	{
		log("Kick Failed: KICK command not sent",0);
		return false;
	}

	return true;
}

bool IRCClient::ban ( std::string mask, std::string channel )
{
	return mode(std::string("+b"),channel,mask);
}

bool IRCClient::unban ( std::string mask, std::string channel )
{
	return mode(std::string("-b"),channel,mask);
}

bool IRCClient::mode ( std::string theMode, std::string target, std::string option )
{	
	// we need to have at LEAST sent the username and stuff
	if (getConnectionState() < eSentNickAndUSer)
		return false;

	IRCCommandINfo	info;
	info.target = target;
	info.params.push_back(theMode);
	if (option.size())
		info.params.push_back(option);

	sendIRCCommand(eCMD_MODE,info);
	return true;
}

bool IRCClient::voice ( std::string user, std::string channel )
{
	return mode(std::string("+v"),channel,user);
}

bool IRCClient::devoice ( std::string user, std::string channel )
{
	return mode(std::string("-v"),channel,user);
}

bool IRCClient::op ( std::string user, std::string channel )
{
	return mode(std::string("+v"),channel,user);
}

bool IRCClient::deop ( std::string user, std::string channel )
{
	return mode(std::string("-v"),channel,user);
}

bool IRCClient::quiet ( std::string user, std::string channel )
{
	return mode(std::string("+q"),channel,user);
}

bool IRCClient::unquiet ( std::string user, std::string channel )
{
	return mode(std::string("-q"),channel,user);
}

// channel modes
bool IRCClient::privateChannel ( std::string channel )
{
	return mode(std::string("+p"),channel,std::string(""));
}

bool IRCClient::unprivateChannel ( std::string channel )
{
	return mode(std::string("-p"),channel,std::string(""));
}

bool IRCClient::moderateChannel ( std::string channel )
{
	return mode(std::string("+m"),channel,std::string(""));
}

bool IRCClient::unmoderateChannel ( std::string channel )
{
	return mode(std::string("-m"),channel,std::string(""));
}

bool IRCClient::secretChannel ( std::string channel )
{
	return mode(std::string("+s"),channel,std::string(""));
}

bool IRCClient::unsecretChannel ( std::string channel )
{
	return mode(std::string("-s"),channel,std::string(""));
}

bool IRCClient::messageLockChannel ( std::string channel )
{
	return mode(std::string("+n"),channel,std::string(""));
}

bool IRCClient::unmessageLockChannel ( std::string channel )
{
	return mode(std::string("-n"),channel,std::string(""));
}

bool IRCClient::topicLockChannel ( std::string channel )
{
	return mode(std::string("+t"),channel,std::string(""));
}

bool IRCClient::untopicLockChannel ( std::string channel )
{
	return mode(std::string("-t"),channel,std::string(""));
}

bool IRCClient::inviteLockChannel ( std::string channel )
{
	return mode(std::string("+i"),channel,std::string(""));
}

bool IRCClient::uninviteLockChannel ( std::string channel )
{
	return mode(std::string("-i"),channel,std::string(""));
}

bool IRCClient::setChannelUserLimit ( std::string channel, int limit )
{
	return mode(std::string("+l"),channel,string_util::format("%d",limit));
}

bool IRCClient::removeChannelUserLimit ( std::string channel )
{
	return mode(std::string("-l"),channel,std::string(""));
}

bool IRCClient::setChannelKey ( std::string channel, std::string key )
{
	return mode(std::string("+k"),channel,key);
}

bool IRCClient::removeChannelKey ( std::string channel )
{
	return mode(std::string("-k"),channel,std::string(""));
}



