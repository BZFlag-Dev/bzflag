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

#include "common.h"
#include "NetHandler.h"
#include "Log.h"
#include "RSA.h"
#include "UserStorage.h"

PacketHandler* PacketHandler::handleHandshake(Packet &packet, ConnectSocket *socket)
{
  uint8 peerType;
  uint16 protoVersion;
  if(!(packet >> peerType >> protoVersion)) return NULL;

  PacketHandler *handler = new PacketHandler(socket);
  bool success = true;

  switch (peerType) {
    case PEER_CLIENT: {
      sLog.outLog("received %s: client using protocol %d", packet.getOpcodeName(), protoVersion);
      uint32 cliVersion;
      uint8 commType;
      if(!(packet >> cliVersion >> commType)) { success = false; break; }
      sLog.outLog("Handshake: client (%d) connected, requesting comm type %d", cliVersion, commType);
      switch (commType) {
        case 0: success = handler->handleAuthRequest(packet); break;
        case 1: success = handler->handleRegisterGetForm(packet); break;
        case 2: success = handler->handleRegisterRequest(packet); break;
        default:
          sLog.outError("Handshake: invalid commType received : %d", commType);
          success = false;
      }
    } break;
    case PEER_SERVER: {
      sLog.outLog("received %s: server using protocol %d", packet.getOpcodeName(), protoVersion);
    } break;
    case PEER_DAEMON: {
      sLog.outLog("received %s: daemon using protocol %d", packet.getOpcodeName(), protoVersion);
    } break;
    default: {
      sLog.outError("received %s: unknown peer type %d", packet.getOpcodeName());
      success = false;
    }
  }

  if(!success)
  {
    delete handler;
    return NULL;
  }
  else
    return handler;
}

bool PacketHandler::handleAuthRequest(Packet &packet)
{
  if(m_authSession)
  {
    sLog.outError("AuthRequest: auth session already in progress");
    return true;
  }
  else
    m_authSession = new AuthSession;

  uint8 *key_n;
  uint16 e;
  size_t n_len;
  sRSAManager.getPublicKey().getValues(key_n, n_len, e);

  Packet challenge(DMSG_AUTH_CHALLENGE, 4+n_len);
  challenge << (uint16)n_len;
  challenge.append(key_n, n_len);
  challenge << (uint16)e;
  m_socket->sendData(challenge);

  free(key_n);
  return true;
}

bool PacketHandler::handleAuthResponse(Packet &packet)
{
  if(!m_authSession)
  {
    sLog.outError("AuthResponse: no auth session started");
    return true;
  }

  uint16 cipher_len;
  if(!(packet >> cipher_len)) return false;
  uint8 *cipher = new uint8[cipher_len+1];
  if(!packet.read(cipher, cipher_len)) { delete[] cipher; return false; }

  uint8 *message;
  size_t message_len;
  sRSAManager.getSecretKey().decrypt(cipher, (size_t)cipher_len, message, message_len);

  // get callsign and password, make sure the string is valid
  bool valid = true;

  // it has to contain exactly one space
  int32 space_poz = -1;
  for(size_t i = 0; i < message_len && valid; i++)
  {
    if(message[i] == ' ')
    {
      if(space_poz == -1) space_poz = (int32)i;
      else valid = false;
    }
  }

  // TODO: make sure all characters are in range etc .. more thorough checking needed

  if(valid)
  {
    std::string callsign((const char*)message, space_poz);
    std::string password((const char*)(message + space_poz + 1), message_len - space_poz - 1);

  } else {
    Packet fail(DMSG_AUTH_FAIL, 4);
    fail << (uint32)AUTH_INVALID_MESSAGE;
    m_socket->sendData(fail);
  }

  sRSAManager.rsaFree(message);
  delete[] cipher;
  return true;
}


bool PacketHandler::handleRegisterGetForm(Packet &packet)
{
  if(!m_authSession)
  {
    sLog.outError("AuthResponse: no auth session started");
    return true;
  }

  return true;
}

bool PacketHandler::handleRegisterRequest(Packet &packet)
{
  if(m_regSession)
    sLog.outError("RegisterRequest: register session already in progress");
  else
    m_regSession = new RegisterSession;
  return true;
}

bool PacketHandler::handleRegisterResponse(Packet &packet)
{
  if(!m_regSession)
  {
    sLog.outError("RegisterResponse: no reg session started");
    return true;
  }

  uint16 cipher_len;
  if(!(packet >> cipher_len)) return false;
  uint8 *cipher = new uint8[cipher_len+1];
  if(!packet.read(cipher, cipher_len)) { delete[] cipher; return false; }

  uint8 *message;
  size_t message_len;
  sRSAManager.getSecretKey().decrypt(cipher, (size_t)cipher_len, message, message_len);

  // get callsign and password, make sure the string is valid
  bool valid = true;

  // it has to contain exactly one space
  int32 space_poz = -1;
  for(size_t i = 0; i < message_len && valid; i++)
  {
    if(message[i] == ' ')
    {
      if(space_poz == -1) space_poz = (int32)i;
      else valid = false;
    }
  }

  // TODO: make sure all characters are in range etc .. more thorough checking needed

  if(valid)
  {
    // hash the password
    size_t digest_len = sUserStore.hashLen();
    uint8 *digest = new uint8[digest_len];
    sUserStore.hash(message, space_poz, digest);
    
    UserInfo info;
    info.name = std::string ((const char*)message, space_poz);
    info.password = std::string((const char*)digest, digest_len);

    sUserStore.registerUser(info);
    
    delete[] digest;
  } else {
    Packet fail(DMSG_REGISTER_FAIL, 4);
    fail << (uint32)REG_INVALID_MESSAGE;
    m_socket->sendData(fail);
  }

  sRSAManager.rsaFree(message);
  delete[] cipher;
  return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8