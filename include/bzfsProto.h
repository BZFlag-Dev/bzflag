/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef bzfsProto_h__
#define bzfsProto_h__

/* common header */
#include "common.h"

#include "NetHandler.h"

/**
 * This class encapsulates the BZFS protocol.
 *
 */

class bzfsProto
{
public:
  /**
   * What I want is to implement a state machine. In response to a
   * particular packet, choose the right action.
   */

  /**
   * bzfs framing function. Examines the message header to determine
   * the code and the length, and fills the passed buffer with exactly
   * one message
   */
  static NetHandler::Status framing(char* src, size_t  src_len, 
				    char* dst, size_t& dst_len);

  /**
   * The protocol requires clients to send the connect header before
   * being recognized. It's the only(?) message that does not conform
   * to the (len, code, ....) structure.
   */
  static NetHandler::Status connectMsg(char* src, size_t  src_len, 
				       char* dst, size_t& dst_len);
private:
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
