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

#ifndef _BZAUTH_PROTOCOL_H_
#define _BZAUTH_PROTOCOL_H_

/** Constants, enums and structures
  * specific to the auth protocol
  */

enum BzAuthOpcodes
{
  MSG_HANDSHAKE             = 0,
  CMSG_AUTH_REQUEST         = 1,
  DMSG_AUTH_FAIL            = 2,
  DMSG_AUTH_CHALLENGE       = 3,
  CMSG_AUTH_RESPONSE        = 4,
  DMSG_AUTH_SUCCESS         = 5,
  CMSG_REGISTER_GET_FORM    = 6,
  DMSG_REGISTER_FAIL        = 7,
  DMSG_REGISTER_SEND_FORM   = 8,
  CMSG_REGISTER_REQUEST     = 9,
  DMSG_REGISTER_CHALLENGE   = 10,
  CMSG_REGISTER_RESPONSE    = 11,
  DMSG_REGISTER_SUCCESS     = 12,
  SMSG_TOKEN_VALIDATE       = 13,
  DMSG_TOKEN_VALIDATE       = 14,
  NUM_OPCODES
};

extern const char *bzAuthOpcodeNames[NUM_OPCODES];

enum BzAuthCommTypes
{
  BZAUTH_COMM_AUTH = 0,
  BZAUTH_COMM_REGFORM = 1,
  BZAUTH_COMM_REG = 2,
  BZAUTH_NUM_COMM_TYPES
};

enum BzAuthErrors
{
  BZAUTH_INVALID_MESSAGE = 0,
  BZAUTH_INCORRECT_CREDENTIALS = 1
};

enum BzRegErrors
{
  REG_INVALID_MESSAGE = 0
};

enum BzAuthdPeerType
{
  BZAUTHD_PEER_ANY = 0,
  BZAUTHD_PEER_CLIENT = 1,
  BZAUTHD_PEER_SERVER = 2,
  BZAUTHD_PEER_DAEMON = 3,
  BZAUTHD_NUM_PEER_TYPES
};

#endif // _BZAUTH_PROTOCOL_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
