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
#include "Factory.h"

class RCLink;
class RCRobotPlayer;
class RCRequest;

class RCRequest {
  public:
    typedef enum {
      ParseError,
      ParseOk,
      InvalidArgumentCount,
      InvalidArguments
    } parseStatus;

    static Factory<RCRequest, std::string> requestHandlerFactory;
    /* These are static functions to allow for instantiation
     * of classes based on a string (the request command name) */
    static void initializeLookup();
    static RCRequest *getRequestInstance(std::string request, RCLink *_link);

    RCRequest(RCLink *_link);
    virtual ~RCRequest();

    /* This is for the linked-list aspect of RCRequest. */
    RCRequest *getNext();
    void append(RCRequest *newreq);

    int getRobotIndex();
    void sendFail();

    virtual void sendAck(bool newline = false);
    virtual bool process(RCRobotPlayer *rrp);
    virtual parseStatus parse(char **arguments, int count) = 0;
    virtual std::string getType() = 0;

    bool fail;
    char *failstr;

  private:
    /* These are static data and functions to allow for instantiation 
     * of classes based on a string (the request command name) */
    static std::map<std::string, RCRequest* (*)(RCLink *)> requestLookup;
    template <typename T>
     static RCRequest* instantiate(RCLink *_link) { return new T(_link); }
    RCRequest *next;
    int robotIndex;

  protected:
    RCLink *link;
    /* Utility functions for subclasses. */
    int setRobotIndex(char *arg);
    template <class T>
      T clamp(T val, T min, T max);
    bool parseFloat(char *string, float &dest);
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
