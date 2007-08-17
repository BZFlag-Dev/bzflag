/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "ServerPing.h"

#include <sys/socket.h>
#include <sys/select.h>
#include <limits.h>
#include "Pack.h"
#include "Protocol.h"

ServerPing::ServerPing() : fd(-1), recieved(0), samples(4),timeout(1), interval(1)
{
  
}

ServerPing::ServerPing(const Address& addr, int port, int _samples, double _interval, double tms) :
     fd(-1), recieved(0), samples(_samples), timeout(tms), interval(_interval)
{
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(port);
  saddr.sin_addr = addr;
}

ServerPing::~ServerPing()
{
  closeSocket();
}

void ServerPing::start()
{ 
  closeSocket();
  activepings.clear();
  openSocket();
}

int ServerPing::calcLag()
{
  if (done()) {
    TimeKeeper total;
    int packetslost = 0;
    for (std::vector<pingdesc>::iterator i = activepings.begin(); i != activepings.end(); ++i) {
      if ((*i).recvtime.getSeconds()) {
	total += i->recvtime - i->senttime;
      } else {
	++packetslost;
      }
    }

    if (packetslost == samples) {
      return INT_MAX;		    // This is bad
    }

    return (int)((total.getSeconds() * 1000.0) / (double)samples - (double)packetslost);
  }

  return 0;
} 

bool ServerPing::done()
{
  return (recieved == samples || (activepings.size() == samples && (TimeKeeper::getCurrent() - activepings.back().senttime) > timeout));
}

void ServerPing::setAddress(const Address& addr, int port)
{
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(port);
  saddr.sin_addr = addr;
}

void ServerPing::setTimeout(double tms)
{
  timeout = tms;
}

void ServerPing::setInterval(double _interval)
{
  interval = _interval;
}

void ServerPing::doPings()
{ 
  if ( activepings.size() < samples && (activepings.empty() || TimeKeeper::getCurrent() - activepings.back().senttime > interval) ) {
    pingdesc pd;
    pd.senttime = TimeKeeper::getCurrent();
    sendPing(activepings.size());
    activepings.push_back(pd);
  }
  
   if (recieved < samples) {
    timeval timeo = { 0, 0 }; //is this what I want to do?
    fd_set readset;
    
    FD_ZERO(&readset);
    FD_SET(fd, &readset);
     if (select(fd+1, (fd_set*)&readset, NULL, NULL, &timeo) > 0) {
      unsigned char tag;
      uint16_t len, code;
      char buffer[1 + 4];
      void *buf = buffer;
      int n = recvfrom(fd, buffer, 1 + 4, 0, 0, 0);
      
      if (n < 4)
        return;
      
      buf = nboUnpackUShort(buf, len);
      buf = nboUnpackUShort(buf, code);
      
      if (code == MsgEchoResponse && len == 1) {
        buf = nboUnpackUByte(buf, tag);
        activepings.at(tag).recvtime = TimeKeeper::getCurrent();
        ++recieved;
       }
    }
  } else {
    closeSocket();
  }
}

void ServerPing::sendPing(unsigned char tag)
{
  char *buffer[1 + 4];
  void *buf = buffer;
  buf = nboPackUShort(buf, 1); //len
  buf = nboPackUShort(buf, MsgEchoRequest);
  buf = nboPackUByte(buf, tag);
  sendto(fd, buffer, 1 + 4, 0, (struct sockaddr*)&saddr, sizeof(saddr));
} 

void ServerPing::openSocket()
{
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0)
    ; //How exactly should I crap out here
}

void ServerPing::closeSocket()
{
  if (fd > 0)
     close(fd);
  fd = -1;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
