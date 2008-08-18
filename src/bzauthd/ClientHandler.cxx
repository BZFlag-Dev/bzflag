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

#include <common.h>
#include "NetHandler.h"
#include "Log.h"
#include "RSA.h"
#include "UserStorage.h"
#include "TokenMgr.h"
#include <assert.h>

PacketHandler* PacketHandler::handleHandshake(Packet &packet, ConnectSocket *socket)
{
  uint8_t peerType;
  uint16_t protoVersion;
  if(!(packet >> peerType >> protoVersion)) return NULL;

  PacketHandler *handler = new PacketHandler(socket);
  bool success = true;

  switch (peerType) {
    case BZAUTHD_PEER_CLIENT: {
      sLog.outLog("received %s: client using protocol %d", getOpcodeName(packet), protoVersion);
      uint32_t cliVersion;
      uint8_t commType;
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
    case BZAUTHD_PEER_SERVER: {
      sLog.outLog("received %s: server using protocol %d", getOpcodeName(packet), protoVersion);
    } break;
    case BZAUTHD_PEER_DAEMON: {
      sLog.outLog("received %s: daemon using protocol %d", getOpcodeName(packet), protoVersion);
    } break;
    default: {
      sLog.outError("received %s: unknown peer type %d", getOpcodeName(packet));
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

  uint8_t *key_n;
  uint32_t e;
  size_t n_len;
  sRSAManager.getPublicKey().getValues(key_n, n_len, e);

  Packet challenge(DMSG_AUTH_CHALLENGE, 4+n_len);
  challenge << (uint16_t)n_len;
  challenge.append(key_n, n_len);
  challenge << (uint32_t)e;
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

  uint16_t cipher_len;
  if(!(packet >> cipher_len)) return false;
  uint8_t *cipher = new uint8_t[cipher_len+1];
  if(!packet.read(cipher, cipher_len)) { delete[] cipher; return false; }

  uint8_t *message = NULL;
  size_t message_len;
  sRSAManager.getSecretKey().decrypt(cipher, (size_t)cipher_len, message, message_len);

  if(!message)
  {
    sLog.outLog("AuthResponse: failed to decrypt cipher");
    Packet fail(DMSG_AUTH_FAIL, 4);
    fail << (uint32_t)BZAUTH_INVALID_MESSAGE;
    m_socket->sendData(fail);
    delete[] cipher;
    return true;
  }

  // get callsign and password, make sure the string is valid
  bool valid = false;
  int32_t callsign_len = -1;
  int32_t password_len = -1;

  if(message_len >= MIN_PASSWORD_LEN + MIN_CALLSIGN_LEN + 1 && message_len <= MAX_PASSWORD_LEN + MAX_CALLSIGN_LEN + 1)
  {
	size_t i;
    // it has to contain exactly one space
    for(i = 0; i < message_len; i++)
    {
      // TODO: better checking for characters
      if(!isprint(message[i])) break;
      if(message[i] == ' ')
      {
        if(callsign_len != -1) break;
        callsign_len = (int32_t)i;
      }
    }

    if(i == message_len && callsign_len != -1)
    {
      if(callsign_len >= MIN_CALLSIGN_LEN && callsign_len <= MAX_CALLSIGN_LEN)
      {
        password_len = (int32_t)message_len - callsign_len - 1;
        if(password_len >= MIN_PASSWORD_LEN && password_len <= MAX_PASSWORD_LEN)
          valid = true;
      }
    }
  }

  // TODO: make sure all characters are in range etc .. more thorough checking needed

  if(valid)
  {
    // the password doesn't need hashing for auth
    UserInfo info;
    info.name = std::string ((const char*)message, callsign_len);
    info.password = std::string((const char*)message + callsign_len + 1, password_len);

    if(sUserStore.authUser(info))
    {
      uint32_t token = sTokenMgr.newToken(info.name);
      Packet success(DMSG_AUTH_SUCCESS, 4);
      success << token;
      m_socket->sendData(success);
    } else {
      Packet fail(DMSG_AUTH_FAIL, 4);
      fail << (uint32_t)BZAUTH_INCORRECT_CREDENTIALS;
      m_socket->sendData(fail);
    }

  } else {
    Packet fail(DMSG_AUTH_FAIL, 4);
    fail << (uint32_t)BZAUTH_INVALID_MESSAGE;
    m_socket->sendData(fail);
  }

  sRSAManager.rsaFree(message);
  delete[] cipher;
  return true;
}


bool PacketHandler::handleRegisterGetForm(Packet &packet)
{
  // currently not implemented
  assert(false);
  return true;
}

bool PacketHandler::handleRegisterRequest(Packet &packet)
{
  if(m_regSession)
  {
    sLog.outError("RegisterRequest: register session already in progress");
    return true;
  }
  else
    m_regSession = new RegisterSession;

  uint8_t *key_n;
  uint32_t e;
  size_t n_len;
  sRSAManager.getPublicKey().getValues(key_n, n_len, e);

  Packet challenge(DMSG_REGISTER_CHALLENGE, 4+n_len);
  challenge << (uint16_t)n_len;
  challenge.append(key_n, n_len);
  challenge << (uint32_t)e;
  m_socket->sendData(challenge);

  free(key_n);
  return true;
}

bool PacketHandler::handleRegisterResponse(Packet &packet)
{
  if(!m_regSession)
  {
    sLog.outError("RegisterResponse: no reg session started");
    return true;
  }

  uint16_t cipher_len;
  if(!(packet >> cipher_len)) return false;
  uint8_t *cipher = new uint8_t[cipher_len+1];
  if(!packet.read(cipher, cipher_len)) { delete[] cipher; return false; }

  uint8_t *message = NULL;
  size_t message_len;
  sRSAManager.getSecretKey().decrypt(cipher, (size_t)cipher_len, message, message_len);

  if(!message)
  {
    sLog.outLog("RegisterResponse: failed to decrypt cipher");
    Packet fail(DMSG_REGISTER_FAIL, 4);
    fail << (uint32_t)REG_INVALID_MESSAGE;
    m_socket->sendData(fail);
    delete[] cipher;
    return true;
  }

  // get callsign and password, make sure the string is valid
  bool valid = false;

  int32_t callsign_len = -1;
  int32_t password_len = -1;

  if(message_len >= MIN_PASSWORD_LEN + MIN_CALLSIGN_LEN + 1 && message_len <= MAX_PASSWORD_LEN + MAX_CALLSIGN_LEN + 1)
  {
	size_t i;
    // it has to contain exactly one space
    for(i = 0; i < message_len; i++)
    {
      // TODO: better checking for characters
      if(!isprint(message[i])) break;
      if(message[i] == ' ')
      {
        if(callsign_len != -1) break;
        callsign_len = (int32_t)i;
      }
    }

    if(i == message_len && callsign_len != -1)
    {
      if(callsign_len >= MIN_CALLSIGN_LEN && callsign_len <= MAX_CALLSIGN_LEN)
      {
        password_len = (int32_t)message_len - callsign_len - 1;
        if(password_len >= MIN_PASSWORD_LEN && password_len <= MAX_PASSWORD_LEN)
          valid = true;
      }
    }
  }

  if(valid)
  {
    // hash the password
    size_t digest_len = sUserStore.hashLen();
    uint8_t *digest = new uint8_t[digest_len];
    sUserStore.hash(message + callsign_len + 1, password_len, digest);
    
    UserInfo info;
    info.name = std::string ((const char*)message, callsign_len);
    info.password = std::string((const char*)digest, digest_len);

    sUserStore.registerUser(info);

    Packet success(DMSG_REGISTER_SUCCESS, 0);
    m_socket->sendData(success);
    
    delete[] digest;
  } else {
    Packet fail(DMSG_REGISTER_FAIL, 4);
    fail << (uint32_t)REG_INVALID_MESSAGE;
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