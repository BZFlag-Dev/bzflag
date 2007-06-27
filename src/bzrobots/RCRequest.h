/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
 * Remote Control Request: Encapsulates requests between backend and frontend
 */

#ifndef	BZF_RC_REQUEST_H
#define	BZF_RC_REQUEST_H

#include "common.h"

typedef enum {
		    InvalidRequest,
		    HelloRequest,
                    setSpeed,
                    setTurnRate,
		    setAhead,
                    setTurnLeft,
                    setFire,
                    getGunHeat,
                    getDistanceRemaining,
                    getTurnRemaining,
                    getTickRemaining,
                    getTickDuration,
                    setTickDuration,
                    execute,
		    TeamListRequest,
		    BasesListRequest,
		    ObstacleListRequest,
		    FlagListRequest,
		    ShotListRequest,
		    MyTankListRequest,
		    OtherTankListRequest,
		    ConstListRequest,
                    RequestCount
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

			float distance, turn;
                        float speed, turnRate;
                        float duration;
			bool fail;
			char *failstr;

  private:
			void set_robotindex(char *arg);
                        template <class T>
                        T clamp(T val, T min, T max);

			agent_req_t request_type;
			int robotindex;
			RCRequest *next;
};

#include "RCLink.h"

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
