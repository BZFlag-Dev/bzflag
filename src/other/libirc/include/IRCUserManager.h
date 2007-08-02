/* libIRC
* Copyright (c) 2004 Christopher Sean Morrison
*
* This package is free software; you can redistribute it and/or
* modify it under the terms of the license found in the file
* named LICENSE that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

// user manager

#ifndef _IRC_USER_MANAGER_
#define _IRC_USER_MANAGER_

#include "IRCTextUtils.h"
#include <vector>
#include <string>
#include <map>

typedef struct 
{
	std::string mask;
	std::string from;
	std::string date;
}trIRCBanListItem;

typedef struct 
{
	std::string mode;
	bool				ircOp;
	bool				identified;
	bool				invisible;
	bool				wallops;
	bool				inviteable;
	bool				messageable;
	bool				ctcpReceipt;
	bool				away;
	bool				idle;
}trIRCUserPermisions;

typedef struct 
{
	std::string mode;
	bool				chanOp;
	bool				voice;
	bool				quieted;
}trIRCChannelUserPermisions;

typedef struct 
{
	std::string mode;
	bool				allowColors;
	bool				forwarded;
	bool				inviteOnly;
	bool				anyInvite;
	bool				juped;
	int					userLimit;
	bool				moderated;
	bool				externalMessages;
	bool				permanent;
	bool				regForVoice;
	bool				regOnly;
	bool				secret;
	bool				reducedModeraton;
}trIRCChannelPermisions;

typedef struct 
{
	int																			id;
	std::string															name;
	trIRCChannelPermisions									perms;
	std::map<int,trIRCChannelUserPermisions>userPerms;
	std::string															lastMessage;
	int																			lastMessageUser;
	std::string															topic;
	std::vector<trIRCBanListItem>						banList;														
}trIRCChannelRecord;

typedef struct 
{
	int											id;
	std::string							nick;
	std::string							host;

	int											lastMessageChannel;
	std::string							lastMessage;

	trIRCUserPermisions			perms;
	std::vector<int>				channels;
}trIRCUserRecord;

class IRCUserManager
{
public:
		IRCUserManager();
		~IRCUserManager();

		// user info API
		bool userExists ( int id );
		bool userExists ( std::string &name );

		int getUserID ( std::string &name );
		std::string getUserNick ( int id );

		std::string getUserHost ( int id );
		std::string getUserHost ( std::string &name );

		std::string getUserLastMessage ( int id );
		std::string getUserLastMessage ( std::string &name );

		int getUserLastMessageChannel ( int id );
		int getUserLastMessageChannel ( std::string &name );
		std::string getUserLastMessageChannelName ( int id );
		std::string getUserLastMessageChannelName ( std::string &name );

		trIRCUserPermisions getUserPerms ( int id );
		trIRCUserPermisions getUserPerms ( std::string &name );

		std::vector<int> listUsers ( void );
		std::vector<std::string> listUserNames ( void );

		// user in channel API
		bool userHasChannels ( int id );
		bool userHasChannels ( std::string &name );

		bool userInChannel ( int id, int channel );
		bool userInChannel ( int id, std::string& channel );
		bool userInChannel ( std::string &name, int channel );
		bool userInChannel ( std::string &name, std::string& channel );

		trIRCChannelUserPermisions getUserChannelPerms ( int id, int channel );
		trIRCChannelUserPermisions getUserChannelPerms ( int id, std::string& channel );
		trIRCChannelUserPermisions getUserChannelPerms ( std::string &name, int channel );
		trIRCChannelUserPermisions getUserChannelPerms ( std::string &name, std::string& channel );

		bool userIsIdentified ( int id );
		bool userIsIdentified ( std::string &name );

		bool userIsOp ( int id, int channel );
		bool userIsOp ( int id, std::string& channel );
		bool userIsOp ( std::string &name, int channel );
		bool userIsOp ( std::string &name, std::string& channel );

		bool userHasVoice ( int id, int channel );
		bool userHasVoice ( int id, std::string& channel );
		bool userHasVoice ( std::string &name, int channel );
		bool userHasVoice ( std::string &name, std::string& channel );

		std::vector<int> listUserChannels ( int id );
		std::vector<int> listUserChannels ( std::string &name );
		std::vector<std::string> listUserChannelNames ( int id );
		std::vector<std::string> listUserChannelNames ( std::string &name );

		// channel API
		bool channelExists ( int id );
		bool channelExists ( std::string &name );

		int getChannelID ( std::string &channel );
		std::string getChannelName ( int id );

		trIRCChannelPermisions getChannelPerms ( int id );
		trIRCChannelPermisions getChannelPerms ( std::string &channel );

		std::string getChannelTopic ( int id );
		std::string getChannelTopic( std::string &channel );

		std::string getChannelLastMessage ( int id );
		std::string getChannelLastMessage ( std::string &channel );

		int getChannelLastMessageUser ( int id );
		int getChannelLastMessageUser ( std::string &channel );
		std::string getChannelLastMessageUserName ( int id );
		std::string getChannelLastMessageUserName ( std::string &channel );

		std::vector<trIRCBanListItem>	getChannelBanList ( int channel );
		std::vector<trIRCBanListItem>	getChannelBanList ( std::string &channel );

		std::vector<int> listChannelUsers ( int id );
		std::vector<int> listChannelsUser ( std::string &name );
		std::vector<std::string> listChanneUserlNames ( int id );
		std::vector<std::string> listChannelUserNames ( std::string &name );

		std::vector<int> listChannels ( void );
		std::vector<std::string> listChannelNames ( void );


		// state update from the IRC data stream
		void userJoinChannel ( int user,  int channel );
		void userJoinChannel ( int user, std::string &channel );
		void userJoinChannel ( std::string &user, int channel );
		void userJoinChannel ( std::string &user, std::string &channel );

		void userPartChannel ( int user,  int channel );
		void userPartChannel ( int user, std::string &channel );
		void userPartChannel ( std::string &user, int channel );
		void userPartChannel ( std::string &user, std::string &channel );

		void nickChange ( std::string &oldNick, std::string &newNick );
		void messageReceved ( std::string &target, std::string &source, std::string &message );
		void modeReceved ( std::string &target, std::string &source, std::string &mode );
		void topicReceved ( std::string &channel, std::string &topic, bool clear = false );

		void removeChannel ( int channel );
		void removeChannel ( std::string channel );

		void addBan ( int channel, std::string &mask, std::string &from, std::string &date );
		void addBan ( std::string channel, std::string &mask, std::string &from, std::string &date );

		void removeBan ( int channel, std::string &mask );
		void removeBan ( std::string channel, std::string &mask );

		void clearBans ( int channel );
		void clearBans ( std::string channel );

		// utilitys
		void purgeNonChannelUsers ( void );
		void purgeLastMessages ( void );
		void setPurgeOnLastPart ( bool purge ){autoPurgeOnLastPart = purge;}
		bool purgeOnLastPart ( void ){return autoPurgeOnLastPart;}

protected:
	trIRCUserRecord& getUserInfo ( int id );
	trIRCUserRecord& getUserInfo ( std::string &name );

	trIRCChannelRecord& getChannelInfo ( int id );
	trIRCChannelRecord& getChannelInfo ( std::string &channel );

	std::string getCleanNick ( std::string &nick );
	std::string getCleanChanName ( std::string &name );

	void setDefaultUserPerms ( trIRCUserPermisions &perms );
	void setDefaultChannelPerms ( trIRCChannelPermisions &perms );
	void setDefaultChannelUserPerms ( trIRCChannelUserPermisions &perms );

	void parseUserPerms ( std::string mode, trIRCUserPermisions &perms );
	void parseChannelPerms ( std::string mode, trIRCChannelPermisions &perms );
	void parseChannelUserPerms ( std::string mode, trIRCChannelUserPermisions &perms );

	void removeChannelFromUser ( int user, int channel );

	void removeUser ( int user );
	void removeUser ( std::string name );

	int findBan (trIRCBanListItem &ban, std::vector<trIRCBanListItem> &banList );

	std::map<int,trIRCUserRecord>	users;
	std::map<int,trIRCChannelRecord> channels;

	std::map<std::string,int>	userNameLookup;
	std::map<std::string,int>	channelNameLookup;

	bool			autoPurgeOnLastPart;
	int				lastUserID;
	int				lastChannelID;
};

#endif//_IRC_USER_MANAGER_