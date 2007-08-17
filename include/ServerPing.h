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
 
#ifndef	BZF_SERVERPING_H
#define	BZF_SERVERPING_H

#include <sys/types.h>
#include <vector>
#include "Address.h"
#include "TimeKeeper.h"
//This class represents the act of pinging a server and calculating the lag
//Create one, set the options, call doPings till it's done, then get the results

class ServerPing
{
public:
  ServerPing();
  ServerPing(const Address& addr, int port, int _samples = 4, double interval = 1, double tms = 1);
  ~ServerPing();
  void start();
  int calcLag();
  bool done();
  void setAddress(const Address& addr, int port);
  void setTimeout(double tms);
  void setInterval(double _interval);
  void doPings();
private:
  void openSocket(); //Dirty low-level stuff
  void closeSocket();
  void sendPing(unsigned char tag);
  struct sockaddr_in saddr;
  
  struct pingdesc
  {
    TimeKeeper senttime;
    TimeKeeper recvtime;
  };
  
  std::vector<pingdesc> activepings;
  
  int fd;
  
  unsigned int recieved;
  
  const unsigned int samples;
  
  double timeout;
  double interval;
  
  TimeKeeper starttime;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
