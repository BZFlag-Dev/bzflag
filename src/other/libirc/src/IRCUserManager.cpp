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

// user manager
#include "IRCUserManager.h"


std::string mergeModes ( std::string mode, std::string modMode )
{
	if (!modMode.size())
		return mode;

	bool add = modMode[0] == '+';

	std::string newMode = mode;
	if (newMode == "NONE")
		newMode = "";

	std::string::iterator	modModeItr = modMode.begin();
	modModeItr++;

	while (modModeItr != modMode.end())
	{
		char item = *modModeItr;

		if (add)
		{
			if (!string_util::charExists(newMode,item))
				newMode += *modModeItr;
		}
		else
		{
			if (string_util::charExists(newMode,item))
			{
				string_util::eraseFirstOf(newMode,item);
			}
		}
		modModeItr++;
	}
	return newMode;
}

IRCUserManager::IRCUserManager()
{
	//std::map<int,trIRCUserRecord>	users;
	//std::map<int,trIRChannelRecord> channels;

	autoPurgeOnLastPart = true;
	lastUserID = 0;
	lastChannelID = 0;
}

IRCUserManager::~IRCUserManager()
{

}

// user info API

bool IRCUserManager::userExists ( int id )
{
		return users.find(id)!= users.end();
}

bool IRCUserManager::userExists ( std::string &name )
{
	return userNameLookup.find(getCleanNick(name)) != userNameLookup.end();
}

std::string IRCUserManager::getUserNick ( int id )
{
	return users[id].nick;
}

int IRCUserManager::getUserID ( std::string &name )
{
	return getUserInfo(name).id;
}

std::string IRCUserManager::getUserHost ( int id )
{
	return getUserInfo(id).host;
}

std::string IRCUserManager::getUserHost ( std::string &name )
{
	return getUserInfo(name).host;
}

std::string IRCUserManager::getUserLastMessage ( int id )
{
	return getUserInfo(id).lastMessage;
}

std::string IRCUserManager::getUserLastMessage ( std::string &name )
{
	return getUserInfo(name).lastMessage;
}

int IRCUserManager::getUserLastMessageChannel ( int id )
{
	trIRCUserRecord	&info = getUserInfo(id);
	if (info.lastMessage.size())
		return	info.lastMessageChannel;

	return -1;
}

int IRCUserManager::getUserLastMessageChannel ( std::string &name )
{
	trIRCUserRecord	&info = getUserInfo(name);
	if (info.lastMessage.size())
		return	info.lastMessageChannel;

	return -1;
}

std::string IRCUserManager::getUserLastMessageChannelName ( int id )
{
	trIRCUserRecord	&info = getUserInfo(id);
	if (info.lastMessage.size())
		return	getChannelName(info.lastMessageChannel);

	return std::string("");
}

std::string IRCUserManager::getUserLastMessageChannelName ( std::string &name )
{
	trIRCUserRecord	&info = getUserInfo(name);
	if (info.lastMessage.size())
		return	getChannelName(info.lastMessageChannel);

	return std::string("");
}

trIRCUserPermisions IRCUserManager::getUserPerms ( int id )
{
	trIRCUserPermisions	perms;

	trIRCUserRecord	&info = getUserInfo(id);
	if (info.lastMessage.size())
		return info.perms;

	return perms;
}

trIRCUserPermisions IRCUserManager::getUserPerms ( std::string &name )
{
	trIRCUserPermisions	perms;

	trIRCUserRecord	&info = getUserInfo(name);
	if (info.lastMessage.size())
		return info.perms;

	return perms;
}

std::vector<int> IRCUserManager::listUsers ( void )
{
	std::vector<int>	userList;

	std::map<int,trIRCUserRecord>::iterator	itr = users.begin();

	while(itr != users.end())
	{
		userList.push_back(itr->first);
		itr++;
	}
	return userList;
}	

std::vector<std::string> IRCUserManager::listUserNames ( void )
{
	std::vector<std::string>	userList;

	std::map<int,trIRCUserRecord>::iterator	itr = users.begin();

	while(itr != users.end())
	{
		userList.push_back(itr->second.nick);
		itr++;
	}
	return userList;
}

// user in channel API
bool IRCUserManager::userHasChannels ( int id )
{
	trIRCUserRecord	&user = getUserInfo(id);
	return user.channels.size() > 0;
}

bool IRCUserManager::userHasChannels ( std::string &name )
{
	trIRCUserRecord	&user = getUserInfo(name);
	return user.channels.size() > 0;
}

bool IRCUserManager::userInChannel ( int id, int channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	return channelRecord.userPerms.find(id) != channelRecord.userPerms.end();
}

bool IRCUserManager::userInChannel ( int id, std::string& channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	return channelRecord.userPerms.find(id) != channelRecord.userPerms.end();
}

bool IRCUserManager::userInChannel ( std::string &name, int channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	return channelRecord.userPerms.find(getUserID(name)) != channelRecord.userPerms.end();
}

bool IRCUserManager::userInChannel ( std::string &name, std::string& channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	return channelRecord.userPerms.find(getUserID(name)) != channelRecord.userPerms.end();
}

trIRCChannelUserPermisions IRCUserManager::getUserChannelPerms ( int id, int channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	return channelRecord.userPerms.find(id)->second;
}

trIRCChannelUserPermisions IRCUserManager::getUserChannelPerms ( int id, std::string& channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	return channelRecord.userPerms.find(id)->second;
}

trIRCChannelUserPermisions IRCUserManager::getUserChannelPerms ( std::string &name, int channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	return channelRecord.userPerms.find(getUserID(name))->second;
}

trIRCChannelUserPermisions IRCUserManager::getUserChannelPerms ( std::string &name, std::string& channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	return channelRecord.userPerms.find(getUserID(name))->second;
}

bool IRCUserManager::userIsIdentified ( int id )
{
	return getUserInfo(id).perms.identified;
}

bool IRCUserManager::userIsIdentified ( std::string &name )
{
	return getUserInfo(name).perms.identified;
}

bool IRCUserManager::userIsOp ( int id, int channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	return channelRecord.userPerms.find(id)->second.chanOp;
}

bool IRCUserManager::userIsOp ( int id, std::string& channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	return channelRecord.userPerms.find(id)->second.chanOp;
}

bool IRCUserManager::userIsOp ( std::string &name, int channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	return channelRecord.userPerms.find(getUserID(name))->second.chanOp;
}

bool IRCUserManager::userIsOp ( std::string &name, std::string& channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	return channelRecord.userPerms.find(getUserID(name))->second.chanOp;
}

bool IRCUserManager::userHasVoice ( int id, int channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	return channelRecord.userPerms.find(id)->second.voice;
}

bool IRCUserManager::userHasVoice ( int id, std::string& channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	return channelRecord.userPerms.find(id)->second.voice;
}

bool IRCUserManager::userHasVoice ( std::string &name, int channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	return channelRecord.userPerms.find(getUserID(name))->second.voice;
}

bool IRCUserManager::userHasVoice ( std::string &name, std::string& channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	return channelRecord.userPerms.find(getUserID(name))->second.voice;
}

std::vector<int> IRCUserManager::listUserChannels ( int id )
{
	return getUserInfo(id).channels;
}

std::vector<int> IRCUserManager::listUserChannels ( std::string &name )
{
	return getUserInfo(name).channels;
}

std::vector<std::string> IRCUserManager::listUserChannelNames ( int id )
{
	trIRCUserRecord		&user = getUserInfo(id);
	std::vector<int>::iterator itr = user.channels.begin();
	std::vector<std::string>	chanList;

	while ( itr != user.channels.end())
		chanList.push_back(getChannelName(*(itr++)));

	return chanList;
}

std::vector<std::string> IRCUserManager::listUserChannelNames ( std::string &name )
{
	trIRCUserRecord		&user = getUserInfo(name);
	std::vector<int>::iterator itr = user.channels.begin();
	std::vector<std::string>	chanList;

	while ( itr != user.channels.end())
		chanList.push_back(getChannelName(*(itr++)));

	return chanList;
}

std::vector<int> IRCUserManager::listChannels ( void )
{
	std::vector<int>	chanList;

	std::map<int,trIRCChannelRecord>::iterator itr = channels.begin();

	while (itr != channels.end())
	{
		chanList.push_back(itr->first);
		itr++;
	}
	return chanList;
}

std::vector<std::string> IRCUserManager::listChannelNames ( void )
{
	std::vector<std::string>	chanList;

	std::map<int,trIRCChannelRecord>::iterator itr = channels.begin();

	while (itr != channels.end())
	{
		chanList.push_back(itr->second.name);
		itr++;
	}
	return chanList;
}

// channel API

bool IRCUserManager::channelExists ( int id )
{
	return channels.find(id) != channels.end();
}

bool IRCUserManager::channelExists ( std::string &name )
{
	return channelNameLookup.find(getCleanChanName(name)) != channelNameLookup.end();
}

int IRCUserManager::getChannelID ( std::string &channel )
{
	return getChannelInfo(channel).id;
}

std::string IRCUserManager::getChannelName ( int id )
{
	return getChannelInfo(id).name;
}

trIRCChannelPermisions IRCUserManager::getChannelPerms ( int id )
{
	return getChannelInfo(id).perms;
}

trIRCChannelPermisions IRCUserManager::getChannelPerms ( std::string &channel )
{
	return getChannelInfo(channel).perms;
}

std::string IRCUserManager::getChannelTopic ( int id )
{
	return getChannelInfo(id).topic;
}

std::string IRCUserManager::getChannelTopic ( std::string &channel )
{
	return getChannelInfo(channel).topic;
}

std::string IRCUserManager::getChannelLastMessage ( int id )
{
	return getChannelInfo(id).lastMessage;
}

std::string IRCUserManager::getChannelLastMessage ( std::string &channel )
{
	return getChannelInfo(channel).lastMessage;
}

int IRCUserManager::getChannelLastMessageUser ( int id )
{
	return getChannelInfo(id).lastMessage.size() ? getChannelInfo(id).lastMessageUser : -1;
}

int IRCUserManager::getChannelLastMessageUser ( std::string &channel )
{
	return getChannelInfo(channel).lastMessage.size() ? getChannelInfo(channel).lastMessageUser : -1;
}

std::string IRCUserManager::getChannelLastMessageUserName ( int id )
{
	return getChannelInfo(id).lastMessage.size() ? getUserNick(getChannelInfo(id).lastMessageUser) : std::string("");
}

std::string IRCUserManager::getChannelLastMessageUserName ( std::string &channel )
{
	return getChannelInfo(channel).lastMessage.size() ? getUserNick(getChannelInfo(channel).lastMessageUser) : std::string("");
}

std::vector<int> IRCUserManager::listChannelUsers ( int id )
{
	trIRCChannelRecord	&channel = getChannelInfo(id);

	std::vector<int> userList;
	std::map<int,trIRCChannelUserPermisions>::iterator itr = channel.userPerms.begin();
	while ( itr != channel.userPerms.end() )
		userList.push_back((itr++)->first);

	return userList;
}

std::vector<int> IRCUserManager::listChannelsUser ( std::string &name )
{
	trIRCChannelRecord	&channel = getChannelInfo(name);

	std::vector<int> userList;
	std::map<int,trIRCChannelUserPermisions>::iterator itr = channel.userPerms.begin();
	while ( itr != channel.userPerms.end() )
		userList.push_back((itr++)->first);

	return userList;
}

std::vector<std::string> IRCUserManager::listChanneUserlNames ( int id )
{
	trIRCChannelRecord	&channel = getChannelInfo(id);

	std::vector<std::string> userList;
	std::map<int,trIRCChannelUserPermisions>::iterator itr = channel.userPerms.begin();
	while ( itr != channel.userPerms.end() )
		userList.push_back(getUserNick((itr++)->first));

	return userList;
}

std::vector<std::string> IRCUserManager::listChannelUserNames ( std::string &name )
{
	trIRCChannelRecord	&channel = getChannelInfo(name);

	std::vector<std::string> userList;
	std::map<int,trIRCChannelUserPermisions>::iterator itr = channel.userPerms.begin();
	while ( itr != channel.userPerms.end() )
		userList.push_back(getUserNick((itr++)->first));

	return userList;
}

std::vector<trIRCBanListItem>	IRCUserManager::getChannelBanList ( int channel )
{
	return getChannelInfo(channel).banList;
}

std::vector<trIRCBanListItem>	IRCUserManager::getChannelBanList ( std::string &channel )
{
	return getChannelInfo(channel).banList;
}


// state update from the IRC data stream
void IRCUserManager::userJoinChannel ( int user,  int channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	trIRCUserRecord	&userRecord = getUserInfo(user);

	trIRCChannelUserPermisions	perms;
	setDefaultChannelUserPerms(perms);

	channelRecord.userPerms[userRecord.id] = perms;
	if ( !userInChannel(userRecord.id,channelRecord.id) )
		userRecord.channels.push_back(channelRecord.id);
}

void IRCUserManager::userJoinChannel ( int user, std::string &channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	std::string nick = getUserNick(user);
	trIRCUserRecord	&userRecord = getUserInfo(nick);

	trIRCChannelUserPermisions	perms;
	setDefaultChannelUserPerms(perms);

	channelRecord.userPerms[userRecord.id] = perms;
	if ( !userInChannel(userRecord.id,channelRecord.id) )
		userRecord.channels.push_back(channelRecord.id);
}

void IRCUserManager::userJoinChannel ( std::string &user, int channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	std::string nick = getCleanNick(user);
	trIRCUserRecord	&userRecord = getUserInfo(nick);

	trIRCChannelUserPermisions	perms;
	setDefaultChannelUserPerms(perms);

	if ( user[0] == '@')
		perms.chanOp = true;

	if ( user[0] == '+')
		perms.voice = true;

	channelRecord.userPerms[userRecord.id] = perms;
	if ( !userInChannel(userRecord.id,channelRecord.id) )
		userRecord.channels.push_back(channelRecord.id);
}

void IRCUserManager::userJoinChannel ( std::string &user, std::string &channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	std::string nick = getCleanNick(user);
	trIRCUserRecord	&userRecord = getUserInfo(nick);

	trIRCChannelUserPermisions	perms;
	setDefaultChannelUserPerms(perms);

	if ( user[0] == '@' )
		perms.chanOp = true;

	if ( user[0] == '+' )
		perms.voice = true;

	if ( !userInChannel(userRecord.id,channelRecord.id) )
		userRecord.channels.push_back(channelRecord.id);

	channelRecord.userPerms[userRecord.id] = perms;
}

void IRCUserManager::userPartChannel ( int user,  int channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	trIRCUserRecord	&userRecord = getUserInfo(user);

	if ( userInChannel(userRecord.id,channelRecord.id) )
	{
		channelRecord.userPerms.erase(channelRecord.userPerms.find(userRecord.id));

		std::vector<int>::iterator	itr = userRecord.channels.begin();
		while ( itr != userRecord.channels.end() )
		{
			if ( *itr == channelRecord.id)
				itr = userRecord.channels.erase(itr);
			else
				itr++;
		}
		if (autoPurgeOnLastPart && userRecord.channels.size() == 0)
			removeUser(user);
	}
}

void IRCUserManager::userPartChannel ( int user, std::string &channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	trIRCUserRecord	&userRecord = getUserInfo(user);

	if ( userInChannel(userRecord.id,channelRecord.id) )
	{
		channelRecord.userPerms.erase(channelRecord.userPerms.find(userRecord.id));

		std::vector<int>::iterator	itr = userRecord.channels.begin();
		while ( itr != userRecord.channels.end() )
		{
			if ( *itr == channelRecord.id)
				itr = userRecord.channels.erase(itr);
			else
				itr++;
		}
		if (autoPurgeOnLastPart && userRecord.channels.size() == 0)
			removeUser(user);
	}
}

void IRCUserManager::userPartChannel ( std::string &user, int channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	trIRCUserRecord	&userRecord = getUserInfo(user);

	if ( userInChannel(userRecord.id,channelRecord.id) )
	{
		channelRecord.userPerms.erase(channelRecord.userPerms.find(userRecord.id));

		std::vector<int>::iterator	itr = userRecord.channels.begin();
		while ( itr != userRecord.channels.end() )
		{
			if ( *itr == channelRecord.id)
				itr = userRecord.channels.erase(itr);
			else
				itr++;
		}
		if (autoPurgeOnLastPart && userRecord.channels.size() == 0)
			removeUser(user);
	}
}

void IRCUserManager::userPartChannel ( std::string &user, std::string &channel )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);
	trIRCUserRecord	&userRecord = getUserInfo(user);

	if ( userInChannel(userRecord.id,channelRecord.id) )
	{
		channelRecord.userPerms.erase(channelRecord.userPerms.find(userRecord.id));

		std::vector<int>::iterator	itr = userRecord.channels.begin();
		while ( itr != userRecord.channels.end() )
		{
			if ( *itr == channelRecord.id)
				itr = userRecord.channels.erase(itr);
			else
				itr++;
		}
		if (autoPurgeOnLastPart && userRecord.channels.size() == 0)
			removeUser(user);
	}
}

void IRCUserManager::nickChange ( std::string &oldNick, std::string &newNick )
{
	trIRCUserRecord	&userRecord = getUserInfo(oldNick);
	
	std::map<std::string,int>::iterator itr = userNameLookup.find(oldNick);
	if (itr != userNameLookup.end())
		userNameLookup.erase(itr);
	userNameLookup[newNick] = userRecord.id;
	userRecord.nick = newNick;
}

void IRCUserManager::messageReceved ( std::string &target, std::string &source, std::string &message )
{
	string_list	nameParts = string_util::tokenize(source,std::string("!"));
	std::string sourceUser = getCleanNick(nameParts[0]);

	trIRCUserRecord	&userRecord = getUserInfo(sourceUser);

	if (!userRecord.host.size() && nameParts[1].size())
		userRecord.host = nameParts[1];

	userRecord.lastMessage = message;

	if ( target[0] == '#')
	{
		trIRCChannelRecord	&channelRecord = getChannelInfo(target);
		userRecord.lastMessageChannel = channelRecord.id;
		channelRecord.lastMessage = message;
		channelRecord.lastMessageUser = userRecord.id;
	}
	else
		userRecord.lastMessageChannel = -1;
}

void IRCUserManager::modeReceved ( std::string &target, std::string &source, std::string &mode )
{
	string_list	nameParts;
	std::string sourceUser;
	
	if (string_util::charExists(source,'!'))
	{
		nameParts = string_util::tokenize(source,std::string("!"));
		sourceUser = getCleanNick(nameParts[0]);
	}
	else
		sourceUser = source;

	trIRCUserRecord	&userRecord = getUserInfo(sourceUser);

	if (!userRecord.host.size() && nameParts.size()>1 && nameParts[1].size())
		userRecord.host = nameParts[1];

	if ( target[0] == '#')
	{
		string_list	targets = string_util::tokenize(target,std::string(" "));

		trIRCChannelRecord	&channelRecord = getChannelInfo(targets[0]);

		if (targets.size()>1)
		{
			int targetUser = getUserID(targets[1]);
			
			if (userInChannel(targetUser,channelRecord.id))
				parseChannelUserPerms(mergeModes(channelRecord.userPerms[targetUser].mode,mode),channelRecord.userPerms[targetUser]);
		}
		else
			parseChannelPerms(mergeModes(channelRecord.perms.mode,mode),channelRecord.perms);
	}
	else
	{
		trIRCUserRecord	&userRecord = getUserInfo(target[0]);
		parseUserPerms(mergeModes(userRecord.perms.mode,mode),userRecord.perms);
	}
}

void IRCUserManager::topicReceved ( std::string &channel, std::string &topic, bool clear )
{
	trIRCChannelRecord	&channelRecord = getChannelInfo(channel);

	if (clear)
		channelRecord.topic = "";

	channelRecord.topic += topic;
}

void IRCUserManager::removeChannel ( int channel )
{
	if (!channelExists(channel))
		return;

	trIRCChannelRecord &channelRecord = getChannelInfo(channel);

	std::vector<int> userList = listChannelUsers(channel);

	std::vector<int>::iterator	itr = userList.begin();
	while ( itr != userList.end() )
	{
		removeChannelFromUser(*itr,channel);
		itr++;
	}

	if ( autoPurgeOnLastPart )
		purgeNonChannelUsers();
}

void IRCUserManager::removeChannel ( std::string channel )
{
	if(channelExists(channel))
		removeChannel(getChannelID(channel));
}

void IRCUserManager::addBan ( int channel, std::string &mask, std::string &from, std::string &date )
{
	trIRCChannelRecord &channelRecord = getChannelInfo(channel);

	trIRCBanListItem	ban;
	ban.mask = string_util::tolower(mask);
	ban.from = from;
	ban.date = date;

	if (findBan(ban,channelRecord.banList) == -1)
		channelRecord.banList.push_back(ban);
}

void IRCUserManager::addBan ( std::string channel, std::string &mask, std::string &from, std::string &date )
{
	trIRCChannelRecord &channelRecord = getChannelInfo(channel);

	trIRCBanListItem	ban;
	ban.mask = string_util::tolower(mask);
	ban.from = from;
	ban.date = date;

	if (findBan(ban,channelRecord.banList) == -1)
		channelRecord.banList.push_back(ban);
}

void IRCUserManager::removeBan ( int channel, std::string &mask )
{
	trIRCChannelRecord &channelRecord = getChannelInfo(channel);

	trIRCBanListItem	ban;
	ban.mask = mask;
	
	int banFind = findBan(ban,channelRecord.banList);
	if ( banFind != -1)
		channelRecord.banList.erase(channelRecord.banList.begin()+banFind);
}

void IRCUserManager::removeBan ( std::string channel, std::string &mask )
{
	trIRCChannelRecord &channelRecord = getChannelInfo(channel);

	trIRCBanListItem	ban;
	ban.mask = mask;

	int banFind = findBan(ban,channelRecord.banList);
	if ( banFind != -1)
		channelRecord.banList.erase(channelRecord.banList.begin()+banFind);
}

void IRCUserManager::clearBans ( int channel )
{
	trIRCChannelRecord &channelRecord = getChannelInfo(channel);
	channelRecord.banList.clear();
}

void IRCUserManager::clearBans ( std::string channel )
{
	trIRCChannelRecord &channelRecord = getChannelInfo(channel);
	channelRecord.banList.clear();
}


// utilitys
void IRCUserManager::purgeNonChannelUsers ( void )
{
	std::map<int,trIRCUserRecord>::iterator userItr = users.begin();
	while (userItr != users.end())
	{
		if ( !userItr->second.channels.size() )
		{
			userNameLookup.erase(userNameLookup.find(userItr->second.nick));
			std::map<int,trIRCUserRecord>::iterator nextItr = userItr++;
			users.erase(userItr);
			userItr = nextItr;
		}
		else
			userItr++;
	}
}

void IRCUserManager::purgeLastMessages ( void )
{
	std::map<int,trIRCUserRecord>::iterator userItr = users.begin();
	while (userItr != users.end())
		(userItr++)->second.lastMessage = "";

	std::map<int,trIRCChannelRecord>::iterator chanItr = channels.begin();
	while (chanItr != channels.end())
		(chanItr++)->second.lastMessage = "";
}

trIRCUserRecord& IRCUserManager::getUserInfo ( int id )
{
	return users[id];
}

trIRCUserRecord& IRCUserManager::getUserInfo ( std::string &name )
{
	std::map<std::string,int>::iterator itr = userNameLookup.find(getCleanNick(name));

	if ( itr == userNameLookup.end())
	{
		trIRCUserRecord	user;
		user.nick = getCleanNick(name);
		user.id = lastUserID;
		setDefaultUserPerms(user.perms);
		users[lastUserID] = user;
		userNameLookup[user.nick] = lastUserID;
		return getUserInfo(lastUserID++);
	}
	return getUserInfo(itr->second);
}

trIRCChannelRecord& IRCUserManager::getChannelInfo ( int id )
{
	return channels[id];
}

trIRCChannelRecord& IRCUserManager::getChannelInfo ( std::string &channel )
{
	std::map<std::string,int>::iterator itr = channelNameLookup.find(getCleanChanName(channel));

	if ( itr == channelNameLookup.end())
	{
		trIRCChannelRecord	channelRecord;
		channelRecord.name = getCleanChanName(channel);
		setDefaultChannelPerms(channelRecord.perms);
		channelRecord.id = lastChannelID;
		channels[lastChannelID] = channelRecord;
		channelNameLookup[getCleanChanName(channel)] = lastChannelID;
		return getChannelInfo(lastChannelID++);
	}
	return getChannelInfo(itr->second);
}

std::string IRCUserManager::getCleanNick ( std::string &nick )
{
	if (nick.size() < 2)
		return string_util::tolower(nick);

	if (nick[0] == '@' || nick[0] == '+')
	{
		std::string	temp = nick;
		temp.erase(temp.begin());
		return string_util::tolower(temp);
	}
	return string_util::tolower(nick);
}

std::string IRCUserManager::getCleanChanName ( std::string &name )
{
	return string_util::tolower(name);
}

void IRCUserManager::setDefaultUserPerms ( trIRCUserPermisions &perms )
{
	perms.mode = "";
	perms.ircOp = false;
	perms.identified = false;
	perms.invisible = false;
	perms.wallops = false;
	perms.inviteable = true;
	perms.messageable = true;
	perms.ctcpReceipt = true;
	perms.away = false;
	perms.idle = false;
}

void IRCUserManager::setDefaultChannelPerms ( trIRCChannelPermisions &perms )
{
	perms.mode = "";
	perms.allowColors = true;
	perms.forwarded = false;
	perms.inviteOnly = false;
	perms.anyInvite = false;
	perms.juped = false;
	perms.userLimit = -1;
	perms.moderated = false;
	perms.externalMessages = true;
	perms.permanent = false;
	perms.regForVoice = false;
	perms.regOnly = false;
	perms.secret = false;
	perms.reducedModeraton = false;
}

void IRCUserManager::setDefaultChannelUserPerms ( trIRCChannelUserPermisions &perms )
{
	perms.mode = "";
	perms.chanOp = false;
	perms.voice= false;
	perms.quieted= false;
}

void IRCUserManager::parseUserPerms ( std::string mode, trIRCUserPermisions &perms )
{
	perms.mode = mode;
	perms.ircOp = false;
	perms.identified = string_util::charExists(mode,'e');
	perms.invisible = string_util::charExists(mode,'i');
	perms.wallops = string_util::charExists(mode,'w');
	perms.inviteable = !string_util::charExists(mode,'I');
}

void IRCUserManager::parseChannelPerms ( std::string mode, trIRCChannelPermisions &perms )
{
	perms.mode = mode;
	perms.allowColors = string_util::charExists(mode,'c');
	perms.forwarded = string_util::charExists(mode,'f');
	perms.inviteOnly = string_util::charExists(mode,'I');
	perms.anyInvite = string_util::charExists(mode,'g');
	perms.juped = string_util::charExists(mode,'j');
	perms.moderated = string_util::charExists(mode,'m');
	perms.externalMessages = !string_util::charExists(mode,'n');
	perms.permanent = string_util::charExists(mode,'P');
	perms.regForVoice = string_util::charExists(mode,'R');
	perms.regOnly = string_util::charExists(mode,'r');
	perms.secret = string_util::charExists(mode,'s');
	perms.reducedModeraton = string_util::charExists(mode,'z');
}

void IRCUserManager::parseChannelUserPerms ( std::string mode, trIRCChannelUserPermisions &perms )
{
	perms.mode = mode;
	perms.chanOp = string_util::charExists(mode,'o');
	perms.voice = string_util::charExists(mode,'v');
	perms.quieted = string_util::charExists(mode,'q');

}

void IRCUserManager::removeChannelFromUser ( int user, int channel )
{
	trIRCUserRecord &userRecord = getUserInfo(user);

	std::vector<int>::iterator chanItr = userRecord.channels.begin();

	while ( chanItr != userRecord.channels.begin() )
	{
		if ( *chanItr == channel)
			chanItr = userRecord.channels.erase(chanItr);
		else
			chanItr++;
	}
}

void IRCUserManager::removeUser ( int user )
{
	std::map<int,trIRCUserRecord>::iterator userItr = users.find(user);
	if (userItr != users.end())
	{
		if ( userItr->second.channels.size() )
		{
			std::vector<int>::iterator itr = userItr->second.channels.begin();
			{
				trIRCChannelRecord	&channelRecord = getChannelInfo(*itr);
				channelRecord.userPerms.erase(channelRecord.userPerms.find(user));
				itr++;
			}
		}
		userItr->second.channels.clear();
		userNameLookup.erase(userNameLookup.find(userItr->second.nick));
		std::map<int,trIRCUserRecord>::iterator nextItr = userItr++;
		users.erase(userItr);
		userItr = nextItr;
	}
}

void IRCUserManager::removeUser ( std::string name )
{
	if (userExists(name))
		removeUser(getUserID(name));
}

int IRCUserManager::findBan (trIRCBanListItem &ban, std::vector<trIRCBanListItem> &banList )
{
	std::vector<trIRCBanListItem>::iterator	itr = banList.begin();

	int item = 0;
	while ( itr != banList.end() )
	{
		if ( itr->mask == string_util::tolower(ban.mask) )
			return item;
		item++;
		itr++;
	}
	return -1;
}



