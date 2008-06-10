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

#include "bzfsProto.h"

#include "version.h"

/**
 * This class encapsulates the BZFS protocol.
 *
 */

/**
 * bzfs framing function. Examines the message header to determine
 * the code and the length, and fills the passed buffer with exactly
 * one message
 */
NetHandler::Status bzfsProto::framing(char* src, size_t  src_len, 
				      char* dst, size_t& dst_len)
{
  NetHandler::Status result(NetHandler::NoMsg);
  // Early sanity check. Make sure the passed length is at least 4 bytes.
  if (src_len >= 4) {
    uint16_t msg_len;
    uint16_t code;

    void* buf(src);
    getGeneralMessageInfo(buf, code, msg_len);
  
    // Test if too large of a message was received. The old
    // implementation tested against a constant, but that constant was
    // used to size the buffer. Here, just make sure it fits in the
    // provided destination buffer
    if ( size_t(msg_len + 4) > dst_len) {
      result = NetHandler::EMsgTooLarge;
      // legacy implementation kills the connection. Perhaps that's done
      // in NetHandler
    } else {
      dst_len = msg_len + 4;
      memcpy(dst, src, dst_len);
      result = NetHandler::GoodMsg;
    }
  }
  return result;
}


/**
 * Extract the message code and message length that are present in
 * all bzfs messages
 */
void* bzfsProto::getGeneralMessageInfo(void* buf, uint16_t& code, uint16_t& len)
{
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);

  return buf;
}

/**
 * bzfs framing function. Examines the message header to determine
 * the code and the length, and fills the passed buffer with exactly
 * one message
 */
NetHandler::Status bzfsProto::connectMsg(char* src, size_t  src_len, 
					 char* dst, size_t& dst_len)
{
  NetHandler::Status result(NetHandler::NoMsg);

  // This is very simple. Either the string matches or it doesn't.
  if (src_len >= strlen(BZ_CONNECT_HEADER)) {
    dst_len = strlen(BZ_CONNECT_HEADER);
    memcpy(dst, src, dst_len);
    result = NetHandler::GoodMsg;
  }

  return result;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
