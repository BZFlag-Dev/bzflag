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

// event trigers from low level messages
// this way the low level events don't need the logic for the high level events.

void IRCClient::noticeMessage ( trMessageEventInfo	&info )
{
	if (info.params[1] == "Looking")
	{
		if (getConnectionState() < eTCPConenct)
			setConnectionState(eTCPConenct);

		callEventHandler(eIRCNoticeEvent,info);
	}
}

void IRCClient::welcomeMessage ( trMessageEventInfo	&info )
{
	setNick(info.target);
	requestedNick = info.target;

	// we know we are conencted here
	if (getConnectionState() < eLoggedIn)
		setConnectionState(eLoggedIn);

	callEventHandler(eIRCWelcomeEvent,info);
}

void IRCClient::endMOTD ( void )
{
	// we know we are conencted here
	if (getConnectionState() < eTCPConenct)
		setConnectionState(eTCPConenct);

	trBaseEventInfo	info;	// no info
	callEventHandler(eIRCEndMOTDEvent,info);
	callEventHandler(eIRCConnectedEvent,info);
}	

void IRCClient::joinMessage ( BaseIRCCommandInfo	&info )
{
	string_list		goodies = string_util::tokenize(info.source,std::string("!"));

	std::string who = goodies[0];

	userManager.userJoinChannel(who,info.target);

	trJoinEventInfo	joinInfo;
	joinInfo.eventType = who == getNick() ? eIRCChannelJoinEvent : eIRCUserJoinEvent;

	joinInfo.channel = info.target;
	joinInfo.user = who;
	callEventHandler(joinInfo.eventType,joinInfo);
}

void IRCClient::partMessage ( BaseIRCCommandInfo	&info )
{
	string_list		goodies = string_util::tokenize(info.source,std::string("!"));

	std::string who = goodies[0];

	trPartEventInfo	partInfo;
	
	userManager.userPartChannel(who,info.target);
	if (who == getNick())
		userManager.removeChannel(info.target);

	partInfo.eventType = who == getNick() ? eIRCChannelPartEvent : eIRCUserPartEvent;
	partInfo.channel = info.target;
	partInfo.user = who;

	callEventHandler(partInfo.eventType,partInfo);
}

void IRCClient::setChannelMode ( std::string channel, std::string mode )
{
	userManager.modeReceved(channel,reportedServerHost,mode);

	trModeEventInfo	info;
	info.eventType = eIRCChannelModeSet;
	info.target = channel;
	info.from = reportedServerHost;
	info.mode = mode;
	callEventHandler(info.eventType,info);
}

void IRCClient::setChannelTopicMessage ( std::string channel, std::string topic, std::string source )
{
	userManager.topicReceved(channel,topic,true);

	trMessageEventInfo	info;
	info.eventType = eIRCTopicChangeEvent;
	info.target = channel;
	info.source = source;
	info.message = topic;
	callEventHandler(info.eventType,info);
}

void IRCClient::modeCommand ( BaseIRCCommandInfo	&info )
{
	std::string who = info.target;

	trModeEventInfo	modeInfo;
	modeInfo.eventType = eIRCNULLEvent;

	trKickBanEventInfo	banInfo;
	banInfo.eventType = eIRCNULLEvent;

	modeInfo.target = who;
	banInfo.channel = who;

	std::string mode = info.params[1];

	// figure out who the message is from, is it form a channel or from a dude
	if (who[0] == '#' )
	{
		if (string_util::charExists(mode,'b'))	// it's a ban!
		{
			bool add = mode[0] == '+';
			banInfo.eventType = eIRCChannelBanEvent;

			std::string banMask = info.params[2];
			if (add)
			{
			  std::string ts = getTimeStamp();
			  userManager.addBan(who,banMask,info.source,ts);
			}
			else	
				userManager.removeBan(who,banMask);

			banInfo.reason = "ban";
			banInfo.user = banMask;
			banInfo.kicker = info.source;
		}
		else	// it's some mode for the channel
		{
			userManager.modeReceved(who,info.source,mode);
			modeInfo.from = info.source;
			modeInfo.mode = mode;

			if (info.params.size() > 1)	// if there is mor then one param then it's a mode change for a user
			{
				modeInfo.eventType = eIRCChannelUserModeSet;
				modeInfo.message = info.getAsString(2);
			}
			else
				modeInfo.eventType = eIRCChannelModeSet;
		}
	}
	else	// it's amode for a user ( like US )
	{
		modeInfo.eventType = eIRCUserModeSet;
		modeInfo.mode = info.params[1];
		std::string infostring = info.getAsString(1);
		userManager.modeReceved(who,info.source, infostring);
	}

	if(modeInfo.eventType != eIRCNULLEvent)
		callEventHandler(modeInfo.eventType,modeInfo);

	if(banInfo.eventType != eIRCNULLEvent)
		callEventHandler(banInfo.eventType,banInfo);
}

void IRCClient::addChannelUsers ( std::string channel, string_list newUsers )
{
	string_list::iterator	itr = newUsers.begin();
	while ( itr != newUsers.end() )
		userManager.userJoinChannel(*(itr++),channel);
}

void IRCClient::endChannelUsersList ( std::string channel )
{
	trBaseEventInfo	info;
	info.eventType = eIRCChanInfoCompleteEvent;
	callEventHandler(info.eventType,info);
}

void IRCClient::privMessage ( BaseIRCCommandInfo	&info )
{
	trMessageEventInfo	msgInfo;
	msgInfo.source = info.source;
	msgInfo.target = info.target;
	msgInfo.message = info.getAsString();
	msgInfo.params = info.params;
	msgInfo.from = string_util::tokenize(msgInfo.source,std::string("!"))[0];

	// lop off the ':'
	msgInfo.message.erase(msgInfo.message.begin());
	msgInfo.params[0].erase(msgInfo.params[0].begin());

	userManager.messageReceved(info.target,info.source,msgInfo.message);

	// figure out who the message is from, is it form a channel or from a dude
	if (info.target.c_str()[0] == '#' )
		msgInfo.eventType = eIRCChannelMessageEvent;
	else
		msgInfo.eventType = eIRCPrivateMessageEvent;

	callEventHandler(msgInfo.eventType,msgInfo);
}

void IRCClient::nickNameError ( int error, std::string message )
{
	trNickErrorEventInfo	info;
	info.error = error;

	if (getConnectionState() < eLoggedIn)
		setConnectionState(eTCPConenct);

	info.error = error;
	info.message = message;
	info.eventType = eIRCNickNameError;
	callEventHandler(info.eventType,info);
}

void IRCClient::nickCommand ( BaseIRCCommandInfo	&info )
{
	string_list	params = string_util::tokenize(info.source,std::string("!"));
	std::string who = params[0];

	userManager.nickChange(who,info.target);

	trNickChangeEventInfo eventInfo;
	eventInfo.eventType = eIRCNickNameChange;
	eventInfo.oldname = who;
	eventInfo.newName = info.target;
	callEventHandler(eventInfo.eventType,eventInfo);
}

void IRCClient::kickCommand ( BaseIRCCommandInfo	&info )
{
	trKickBanEventInfo	eventInfo;

	eventInfo.eventType = eIRCUserKickedEvent;
	eventInfo.channel = info.target;
	eventInfo.kicker = string_util::tokenize(info.source,std::string("!"))[0];
	eventInfo.reason = info.getAsString(1);
	// kill the :
	eventInfo.reason.erase(eventInfo.reason.begin());

	eventInfo.user = info.params[0];
	callEventHandler(eventInfo.eventType,eventInfo);

	userManager.userPartChannel(eventInfo.user,eventInfo.channel);
}

void IRCClient::QuitMessage ( BaseIRCCommandInfo	&info )
{
	string_list		goodies = string_util::tokenize(info.source,std::string("!"));

	std::string who = goodies[0];

	trPartEventInfo	partInfo;

	userManager.userPartChannel(who,info.target);
	if (who == getNick())
		userManager.removeChannel(info.target);

	partInfo.eventType = eIRCQuitEvent;
	partInfo.channel = info.target;
	partInfo.user = who;

	callEventHandler(partInfo.eventType,partInfo);
}



