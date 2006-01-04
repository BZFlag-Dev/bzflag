/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZADMINCLIENT_H
#define BZADMINCLIENT_H

/* global interface headers */
#include "common.h"

/* system interface headers */
#include <map>
#include <string>

#include "PlayerInfo.h"
#include "colors.h"
#include "ServerLink.h"
#include "UIMap.h"
#include "StartupInfo.h"


class BZAdminUI;

extern StartupInfo startupInfo;

/** This class is a client that connects to a BZFlag server and has
    functions for sending and receiving messages. If you give it
    a pointer to a BZAdminUI in the constructor it will use that UI
    for communication with the user.
*/
class BZAdminClient {
public:

  /** These values may be returned by getServerString(). */
  enum ServerCode {
    GotMessage,
    NoMessage,
    Superkilled,
    CommError
  };

  /** A default constructor. It tries to connect to the server at host:port.
      If it doesn't succeed, calls to isValid() will return false. */
  BZAdminClient(BZAdminUI* bzInterface = NULL);

  /** Formats an incoming message. */
  std::string formatMessage(const std::string& msg, PlayerId src, PlayerId dst,
		       TeamColor dstTeam, PlayerId me);

  /** Return the PlayerId that this client has been assigned by the server. */
  PlayerId getMyId();

  /** Returns a reference to a @c PlayerIdMap containing the players
      in the game. */
  PlayerIdMap& getPlayers();

  /** Checks for new packets from the server, ignores them or stores a
      text message in @c str. Tells @c ui about new or removed players. Returns
      0 if no interesting packets have arrived, 1 if a message has been stored
      in @c str, negative numbers for errors. A color suggestion will be stored
      in @c colorCode.
  */
  ServerCode checkMessage();

  /** Returns the std::string that the client built from the last received
      message. */
  std::pair<std::string, ColorCode> getLastMessage() const;

  /** This function returns @c true if this object has a valid connection
      to a server, @c false if it doesn't. */
  bool isValid() const;

  /** This functions runs a loop that for each iteration checks if the
      server has sent anything, and if the user has typed anything. It
      sends the servers output to the user interface, and the users
      commands to the server. */
  void runLoop();

  /** Sends the message @c msg to the server with the player or team @c target
      as receiver. */
  void sendMessage(const std::string& msg, PlayerId target);

  /** This function changes the BZAdminUI used by the client to communicate
      with the user. The object pointed to by @c interface will not be
      deallocated when BZAdminClient is done with it, you will have to
      do that yourself. */
  void setUI(BZAdminUI* bzInterface);

  /** Waits until we think the server has processed all our input so far.
      This is done by sending a private message to ourself and waiting until we
      receive it from the server. */
  void waitForServer();

  /** This function tells the client to ignore messages of this type. If
      getServerString() is called and a message of this type is received,
      the function will just ignore that message and wait for the next one.
      Warning: Ignoring some types of messages may cause unexpected behaviour!
      For example, if you ignore MsgSuperKill bzadmin won't know when it has
      been kicked off the server. */
  void ignoreMessageType(uint16_t type);

  /** This function tells the client to show messages of this type
      (return them as strings from getServerString()). Note that the
      message still won't be shown if bzadmin does not know how to handle
      it - for example, player position updates won't be shown. */
  void showMessageType(uint16_t type);

  /** This function tells the client to ignore messages of this type. If
      getServerString() is called and a message of this type is received,
      the function will just ignore that message and wait for the next one.
      Warning: Ignoring some types of messages may cause unexpected behaviour!
      For example, if you ignore MsgSuperKill bzadmin won't know when it has
      been kicked off the server. */
  void ignoreMessageType(std::string type);

  /** This function tells the client to show messages of this type
      (return them as strings from getServerString()). Note that the
      message still won't be shown if bzadmin does not know how to handle
      it - for example, player position updates won't be shown. */
  void showMessageType(std::string type);

  /** This function returns a const reference to the mapping from message type
      names to message type codes. */
  const std::map<std::string, uint16_t>& getMessageTypeMap() const;

  /** This function returns the filter status of the message type
      @c msgType, @c true means "show" and @c false means "hide". */
  bool getFilterStatus(uint16_t msgType) const;

protected:

  /** This function prints a /set command for the BZDB variable with name
      @c name to the current UI. It assumes that there actually is an UI,
      if @c ui is NULL this function could crash the program. It has to be
      static because it is used as a callback for StateDatabase::iterate(). */
  static void listSetVars(const std::string& name, void* thisObject);

  /** Connects to the list server and gets a list of available servers
   */
  void outputServerList() const;


  PlayerIdMap players;
  TeamColor myTeam;
  ServerLink sLink;
  std::pair<std::string, ColorCode> lastMessage;
  bool valid;
  std::map<uint16_t, bool> messageMask;
  std::map<TeamColor, ColorCode> colorMap;
  std::map<std::string, uint16_t> msgTypeMap;
  BZAdminUI* ui;
};


#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
