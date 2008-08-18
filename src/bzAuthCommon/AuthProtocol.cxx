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

#include "Common.h"
#include "AuthProtocol.h"

const char *bzAuthOpcodeNames[NUM_OPCODES] = {
  "MSG_HANDSHAKE",
  "CMSG_AUTH_REQUEST",
  "DMSG_AUTH_FAIL",
  "DMSG_AUTH_CHALLENGE",
  "CMSG_AUTH_RESPONSE",
  "DMSG_AUTH_SUCCESS",
  "CMSG_REGISTER_GET_FORM",
  "DMSG_REGISTER_FAIL",
  "DMSG_REGISTER_SEND_FORM",
  "CMSG_REGISTER_REQUEST",
  "DMSG_REGISTER_CHALLENGE",
  "CMSG_REGISTER_RESPONSE",
  "DMSG_REGISTER_SUCCESS",
  "SMSG_TOKEN_VALIDATE",
  "DMSG_TOKEN_VALIDATE",
};

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8