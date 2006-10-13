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

/*
 * Remote Control Link: Encapsulates communication between local player and
 * remote agent.
 */

#ifndef	BZF_RC_LINK_H
#define	BZF_RC_LINK_H

#include "common.h"

#include "global.h"
#include "Address.h"
#include "TimeKeeper.h"

#define RC_LINK_RECVBUFLEN 100000
#define RC_LINK_SENDBUFLEN 100000
#define RC_LINK_MAXARGS 50
#define RC_LINK_OVERFLOW_MSG "\nerror Connection Stalled.  RC stopped" \
				" reading data!\n"
#define RC_LINK_NOHELLO_MSG "error agent expected\n"
#define RC_LINK_HELLO_STR "bzrobots 1\n"
typedef enum {
		    InvalidRequest,
		    HelloRequest,
		    Speed,
		    AngularVel,
		    Shoot,
		    TeamListRequest,
		    BasesListRequest,
		    ObstacleListRequest,
		    FlagListRequest,
		    ShotListRequest,
		    MyTankListRequest,
		    OtherTankListRequest,
		    ConstListRequest
} agent_req_t;

class RCLink;

class RCRequest {
  public:
			RCRequest();
			RCRequest(agent_req_t reqtype);
			RCRequest(int argc, char **argv);
			RCRequest *getnext();
			void append(RCRequest *newreq);
			int get_robotindex();
			agent_req_t get_request_type();
			void sendack(RCLink *link);
			void sendfail(RCLink *link);

			float speed_level, angularvel_level;
			bool fail;
			char *failstr;

  private:
			void set_robotindex(int index);

			agent_req_t request_type;
			int robotindex;
			RCRequest *next;
};

class RCLink {
  public:
    enum State {
			Disconnected,
			SocketError,
			Listening,
			Connecting,
			Connected
    };

			RCLink(int port);
			~RCLink();
			void startListening();
			void tryAccept();
			void update();
			int update_read();
			int update_parse(int maxlines=0);
			int update_write();
			bool parsecommand(char *cmd);
			void detach_agents();

			bool respond(char *message);
			bool respondf(const char *format, ...);
			RCRequest *poprequest();

  private:
			enum State status;
			int listenfd, connfd;
			int port;
			char recvbuf[RC_LINK_RECVBUFLEN];
			char sendbuf[RC_LINK_SENDBUFLEN];
			int recv_amount, send_amount;
			RCRequest *requests;
			bool input_toolong, output_overflow;
};


//
// RCLink
//

#endif // BZF_RC_LINK_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

