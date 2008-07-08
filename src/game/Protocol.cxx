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

// interface header
#include "Protocol.h"

// implementation-specific system headers
#include <iostream>
// #include <algorithm>
// #include <assert.h>
// #include <errno.h>
// #include <vector>
// #include <string>
// #include <time.h>
// #include <sstream>

// implementation-specific bzflag headers
// #include "NetHandler.h"
// #include "VotingArbiter.h"
// #include "version.h"
// #include "bz_md5.h"
// #include "BZDBCache.h"
// #include "ShotUpdate.h"
// #include "PhysicsDriver.h"
// #include "CommandManager.h"
// #include "TimeBomb.h"
// #include "ConfigFileManager.h"
// #include "bzsignal.h"

// implementation-specific bzfs-specific headers
#include "Pack.h"
// #include "RejoinList.h"
// #include "ListServerConnection.h"
// #include "WorldInfo.h"
// #include "WorldWeapons.h"
// #include "BZWReader.h"
// #include "SpawnPosition.h"
// #include "DropGeometry.h"
// #include "commands.h"
// #include "MasterBanList.h"
// #include "Filter.h"
// #include "WorldEventManager.h"
// #include "WorldGenerators.h"
// #include "bzfsProto.h"
// #include "bzfsMessages.h"
// #include "bzfsClientMessages.h"
// #include "bzfsPlayerStateVerify.h"
// #include "AutoAllowTimer.h"
// #include "ServerIntangibilityManager.h"

// common implementation headers
// #include "Obstacle.h"
// #include "ObstacleMgr.h"
// #include "BaseBuilding.h"
// #include "AnsiCodes.h"
// #include "GameTime.h"
// #include "bzfsAPI.h"
// #include "BufferedNetworkMessage.h"

  /// Destructor
  ///
  /// Although this class doesn't allocate the memory, free it here so
  /// no one else has to think about it.
BZProtocolMsg::~BZProtocolMsg()
{
  delete[] static_cast<char*>(buf);
}

/// Return the total size of the packet
size_t BZProtocolMsg::size() const
{
  return sizeof(msgLen) + sizeof(msgCode) + msgLen;
}

/// construct a network protocol object
void* BZProtocolMsg::pack()
{
  // It doesn't make sense, but let multiple calls to pack deal with memory coherently
  delete[] static_cast<char*>(buf);

  buf = new char[ size() ];
  void* tmpbuf = buf;

  tmpbuf = nboPackUShort(tmpbuf, msgLen);
  tmpbuf = nboPackUShort(tmpbuf, msgCode);
  tmpbuf = doPack(tmpbuf);

  return buf;
}

/// Construct for an explicit code (before sending)
BZProtocolMsg::BZProtocolMsg(uint16_t code_, uint16_t len_)
  : msgCode(code_)
  , msgLen(len_)
  , buf(0)
{
}

/// Construct from a memory buffer (after receiving)
BZProtocolMsg::BZProtocolMsg(void*& buf_)
  : buf(0)
{
  std::cout << "BZProtocolMsg ctor: buf " << buf_ << "\n";
  buf_ = nboUnpackUShort(buf_, msgLen);
  buf_ = nboUnpackUShort(buf_, msgCode);
}

/// construct a network protocol object --- implemented by base classes to pack their junk
void* BZProtocolMsg::doPack(void* buf_)
{
  return buf_;
}

EchoRequest::EchoRequest(unsigned char tag_)
  : BZProtocolMsg(MsgEchoRequest, sizeof(msgTag))
  , msgTag(tag_)
{
}

EchoRequest::EchoRequest(void* buf_)
  : BZProtocolMsg(buf_)
{
  std::cout << "EchoRequest ctor: buf " << buf_ << "\n";
  buf_ = nboUnpackUByte(buf_, msgTag);
}

unsigned char EchoRequest::tag()
{
  return msgTag;
}

void* EchoRequest::doPack(void* buf_)
{
  buf_ = nboPackUByte(buf_, msgTag);
  return buf_;
}

EchoResponse::EchoResponse(unsigned char tag_)
  : BZProtocolMsg(MsgEchoResponse, sizeof(msgTag))
  , msgTag(tag_)
{
}

EchoResponse::EchoResponse(void* buf_)
  : BZProtocolMsg(buf_)
{
  std::cout << "EchoResponse ctor: buf " << buf_ << "\n";
  buf_ = nboUnpackUByte(buf_, msgTag);
}

void* EchoResponse::doPack(void* buf_)
{
  buf_ = nboPackUByte(buf_, msgTag);
  return buf_;
}

