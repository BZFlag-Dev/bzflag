/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	__RCREQUEST_H__
#define	__RCREQUEST_H__

#include "common.h"

/* system interface headers */
#include <string>
#include <ostream>

/* local interface headers */
#include "RCMessage.h"
#include "RCRobotPlayer.h"


/**
 * Remote Control Request: Encapsulates requests between backend and frontend
 */
class RCRequest : public RCMessage<RCRequest>
{
public:
  virtual bool process(RCRobotPlayer *) { return true; }
  virtual messageParseStatus parse(char **arguments, int count) = 0;
  virtual std::string getType() const = 0;
  virtual void getParameters(std::ostream &stream) const = 0;
  
  static void initializeLookup(void);
};


#else
class RCRequest;
#endif /* __RCREQUEST_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
