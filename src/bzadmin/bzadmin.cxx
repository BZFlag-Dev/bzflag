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

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <curses.h>

#include "BZAdminUI.h"
#include "CursesUI.h"
#include "ServerLink.h"
#include "StdBothUI.h"
#include "StdInUI.h"
#include "StdOutUI.h"
#include "OptionParser.h"

using namespace std;

// function prototypes
/** Checks for new packets from the server, ignores them or stores a
    text message in str. Tells ui about new or removed players. Returns
    false if no interesting packets have arrived. */
bool getServerString(ServerLink& sLink, string& str, BZAdminUI& ui);

/** Sends the message msg to the server. */
void sendMessage(ServerLink& sLink, const string& msg, PlayerId target);

/** Formats an incoming message. */
string formatMessage(const string& msg, PlayerId src, 
		     PlayerId dst, TeamColor dstTeam, PlayerId me);


// some global variables
map<PlayerId, string> players;
TeamColor myTeam;
struct CLOptions {
  CLOptions() 
    : team("green"), stdin(false), stdout(false) { }
  string team;
  bool stdboth;
  bool stdin;
  bool stdout;
} clOptions;


// Here we go.
int main(int argc, char** argv) {
  
  // parse command line arguments
  OptionParser op;
  op.registerVariable("team", clOptions.team);
  op.registerVariable("stdboth", clOptions.stdboth);
  op.registerVariable("stdin", clOptions.stdin);
  op.registerVariable("stdout", clOptions.stdout);
  if (!op.parse(argc, argv)) {
    cerr<<op.getError()<<endl;
    return 1;
  }
  if (clOptions.team == "rogue")
    myTeam = RogueTeam;
  else if (clOptions.team == "red")
    myTeam = RedTeam;
  else if (clOptions.team == "green")
    myTeam = GreenTeam;
  else if (clOptions.team == "blue")
    myTeam = BlueTeam;
  else if (clOptions.team == "purple")
    myTeam = PurpleTeam;
  else {
    cerr<<'"'<<clOptions.team<<"\" is not a valid team."<<endl;
    return 1;
  }

  // check that we have callsign and host in the right format and extract them
  if (op.getParameters().size() == 0) {
    cerr<<"You have to specify callsign@host."<<endl;
    return 1;
  }
  const string& namehost = op.getParameters()[0];
  int atPos = namehost.find('@');
  if (atPos == -1) {
    cerr<<"You have to specify callsign@host."<<endl;
    return 1;
  }
  string name = namehost.substr(0, atPos);
  name.insert(name.begin(), '@');
  string host = namehost.substr(atPos + 1);
  int port = ServerPort;
  int cPos = host.find(':');
  if (cPos != -1) {
    port = atoi(host.substr(cPos + 1).c_str());
    host = host.substr(0, cPos);
  }
  
  // connect to the server
  Address a(host);
  ServerLink sLink(a, port);
  if (sLink.getState() != ServerLink::Okay) {
    cerr<<"Could not connect to "<<host<<':'<<port<<'.'<<endl;
    return 0;
  }
  sLink.sendEnter(TankPlayer, myTeam, name.c_str(), "");
  if (sLink.getState() != ServerLink::Okay) {
    cerr<<"Rejected."<<endl;
    return 0;
  }
  
  // if we got commands as arguments, send them and exit
  if (op.getParameters().size() > 1) {
    for (unsigned int i = 1; i < op.getParameters().size(); ++i)
      sendMessage(sLink, op.getParameters()[i], AllPlayers);
    return 0;
  }
  
  // choose UI
  BZAdminUI* ui;
  if (clOptions.stdboth)
    ui = new StdBothUI;
  else if (clOptions.stdin)
    ui = new StdInUI;
  else if (clOptions.stdout)
    ui = new StdOutUI;
  else
    ui = new CursesUI(players, sLink.getId());

  // main loop
  string str;
  PlayerId me = sLink.getId();
  while (true) {
    while (getServerString(sLink, str, *ui))
      ui->outputMessage(str);
    if (ui->checkCommand(str)) {
      if (str == "/quit")
	break;
      sendMessage(sLink, str, ui->getTarget());
      // private messages to other players aren't sent back to us, print here
      if (players.count(ui->getTarget()))
	ui->outputMessage(formatMessage(str, me, ui->getTarget(), NoTeam, me));
    }
  }
  delete ui;
  
  return 0;
}


bool getServerString(ServerLink& sLink, string& str, BZAdminUI& ui) {
  uint16_t code, len;
  char inbuf[MaxPacketLen];
  int e;
  std::string dstName, srcName;
  str = "";
  
  /* read until we have a package that we want, or until there are no more
     packages for 100 ms */
  while ((e = sLink.read(code, len, inbuf, 100)) == 1) {
    void* vbuf = inbuf;
    PlayerId p;
    switch (code) {
      
    case MsgAddPlayer:
      
      vbuf = nboUnpackUByte(vbuf, p);
      
      uint16_t team, type, wins, losses, tks;
      char callsign[CallSignLen];
      char email[EmailLen];
      vbuf = nboUnpackUShort(vbuf, type);
      vbuf = nboUnpackUShort(vbuf, team);
      vbuf = nboUnpackUShort(vbuf, wins);
      vbuf = nboUnpackUShort(vbuf, losses);
      vbuf = nboUnpackUShort(vbuf, tks);
      vbuf = nboUnpackString(vbuf, callsign, CallSignLen);
      vbuf = nboUnpackString(vbuf, email, EmailLen);
      players[p] = callsign;
      if (p != sLink.getId())
	ui.addedPlayer(p);
      str = str + "*** '" + callsign + "' joined the game.";
      return true;
      
    case MsgRemovePlayer:
      vbuf = nboUnpackUByte(vbuf, p);
      str = str + "*** '" + players[p] + "' left the game.";
      ui.removingPlayer(p);
      players.erase(p);
      return true;
      
    case MsgMessage:
      
      // unpack the message header
      PlayerId src;
      PlayerId dst;
      PlayerId me = sLink.getId();
      vbuf = nboUnpackUByte(vbuf, src);
      vbuf = nboUnpackUByte(vbuf, dst);
      
      // is the message for me?
      TeamColor dstTeam = (dst >= 244 && dst <= 250 ? 
			   TeamColor(250 - dst) : NoTeam);
      if (dst == AllPlayers || src == me || dst == me || dstTeam == myTeam) {
	str = formatMessage((char*)vbuf, src, dst, dstTeam, me);
	return true;
      }
    }
    
    if (e == -2) {
      cerr<<"Server communication error!"<<endl;
      exit(0);
    }
  }
  
  return false;
}


void sendMessage(ServerLink& sLink, const string& msg, PlayerId target) {
  char buffer[MessageLen];
  char buffer2[1 + MessageLen];
  void* buf = buffer2;
  
  buf = nboPackUByte(buf, target);
  memset(buffer, 0, MessageLen);
  strncpy(buffer, msg.c_str(), MessageLen);
  nboPackString(buffer2 + 1, buffer, MessageLen);
  sLink.send(MsgMessage, sizeof(buffer2), buffer2);
}


string formatMessage(const string& msg, PlayerId src, 
		     PlayerId dst, TeamColor dstTeam, PlayerId me) {
  string formatted = "    ";
  
  // get sender and receiver
  const string srcName = (src == ServerPlayer ? "SERVER" : 
			  (players.count(src) ? players[src] : "(UNKNOWN)"));
  const string dstName = (players.count(dst) ? players[dst] : "(UNKNOWN)");
  
  // direct message to or from me
  if (dst == me || players.count(dst)) {
    if (!(src == me && dst == me)) {
      formatted += "[";
      if (src == me) {
	formatted += "->";
	formatted += dstName;
      }
      else {
	formatted += srcName;
	formatted += "->";
      }
      formatted += "] ";
    }
    formatted += msg;
  }
  
  // public or team message
  else {
    if (dstTeam != NoTeam)
      formatted += "[Team] ";
    formatted += srcName;
    formatted += ": ";
    formatted += msg;
  }
  
  return formatted;
}
