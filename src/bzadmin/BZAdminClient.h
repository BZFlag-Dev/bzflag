/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZADMINCLIENT_H
#define BZADMINCLIENT_H

#include <map>
#include <string>

#include "BZAdminUI.h"
#include "ServerLink.h"

using namespace std;


/** This class is a client that connects to a BZFlag server and has
    functions for sending and receiving messages. */
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
  BZAdminClient(string callsign, string host, int port,
		BZAdminUI* interface = NULL);

  /** Formats an incoming message. */
  string formatMessage(const string& msg, PlayerId src, PlayerId dst,
		       TeamColor dstTeam, PlayerId me);

  /** Return the PlayerId that this client has been assigned by the server. */
  PlayerId getMyId();

  /** Returns a reference to a @c map<PlayerId,string> containing the players
      in the game. */
  map<PlayerId, string>& getPlayers();

  /** Checks for new packets from the server, ignores them or stores a
      text message in @c str. Tells @c ui about new or removed players. Returns
      0 if no interesting packets have arrived, 1 if a message has been stored
      in @c str, negative numbers for errors.
  */
  ServerCode getServerString(string& str);

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
  void sendMessage(const string& msg, PlayerId target);

  /** This function changes the BZAdminUI used by the client to communicate
      with the user. The object pointed to by @c interface will not be
      deallocated when BZAdminClient is done with it, you will have to
      do that yourself. */
  void setUI(BZAdminUI* interface);

  /** Waits until we think the server has processed all our input so far. */
  void waitForServer();

protected:

  map<PlayerId, string> players;
  TeamColor myTeam;
  ServerLink sLink;
  bool valid;
  BZAdminUI* ui;
};


#endif

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
