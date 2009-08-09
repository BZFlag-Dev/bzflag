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

#include "ClientHandler.h"
#include "Log.h"
#include "RSA.h"
#include "UserStorage.h"
#include "TokenMgr.h"
#include <assert.h>

INSTANTIATE_PACKETHANDLER(ClientHandler)

void ClientHandler::initHandlerTable()
{
  for(int i = 0; i < NUM_OPCODES; i++)
    handlerTable[i] = &PacketHandler::handleInvalid;

  handlerTable[CMSG_AUTH_REQUEST]       = &ClientHandler::handleAuthRequest;
  handlerTable[CMSG_AUTH_RESPONSE]      = &ClientHandler::handleAuthResponse;
  handlerTable[CMSG_REGISTER_GET_FORM]  = &ClientHandler::handleRegisterGetForm;
  handlerTable[CMSG_REGISTER_REQUEST]   = &ClientHandler::handleRegisterRequest;
  handlerTable[CMSG_REGISTER_RESPONSE]  = &ClientHandler::handleRegisterResponse;
}

bool ClientHandler::handleAuthRequest(Packet &/*packet*/)
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

bool ClientHandler::handleAuthResponse(Packet &packet)
{
  if(!m_authSession)
  {
    sLog.outError("AuthResponse: no auth session started");
    return true;
  }

  uint16_t cipher_len = 0;
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

  if(valid)
  {
    UserInfo info;
    info.name = std::string ((const char*)message, callsign_len);
    info.password = std::string((const char*)message + callsign_len + 1, password_len);

    uint32_t uid = sUserStore.authUserInGame(info);
    if(uid)
    {
      uint32_t token = sTokenMgr.newToken(info.name, uid, 0);
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


bool ClientHandler::handleRegisterGetForm(Packet &/*packet*/)
{
  // currently not implemented
  assert(false);
  return true;
}

bool ClientHandler::handleRegisterRequest(Packet &/*packet*/)
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

bool ClientHandler::handleRegisterResponse(Packet &packet)
{
  if(!m_regSession)
  {
    sLog.outError("RegisterResponse: no reg session started");
    return true;
  }

  uint16_t cipher_len = 0;
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
  int32_t email_len = -1;

  if(message_len >= MIN_PASSWORD_LEN + MIN_CALLSIGN_LEN + MIN_EMAIL_LEN + 2 && message_len <= MAX_PASSWORD_LEN + MAX_CALLSIGN_LEN + MAX_EMAIL_LEN + 2)
  {
    size_t i;
    // it has to contain exactly two spaces
    for(i = 0; i < message_len; i++)
    {
      // TODO: better checking for characters
      if(!isprint(message[i])) break;
      if(message[i] == ' ')
      {
        if(callsign_len != -1) {
          if(password_len != -1) break;
          password_len = (int32_t)i - callsign_len - 1;
        } else
          callsign_len = (int32_t)i;
      }
    }

    if(i == message_len && callsign_len != -1 &&
      callsign_len >= MIN_CALLSIGN_LEN && callsign_len <= MAX_CALLSIGN_LEN &&
      password_len >= MIN_PASSWORD_LEN && password_len <= MAX_PASSWORD_LEN)
    {
      email_len = (int32_t)message_len - (callsign_len + password_len + 2);
      if(email_len >= MIN_EMAIL_LEN && email_len <= MAX_EMAIL_LEN)
        valid = true;
    }
  }

  // TODO: check for proper email address

  if(valid)
  { 
    UserInfo info;
    info.name = std::string ((const char*)message, callsign_len);
    info.password = std::string((const char*)message + callsign_len + 1, password_len);
    info.email = std::string((const char*)message + callsign_len + password_len + 2, email_len);

    BzRegErrors error = sUserStore.registerUser(info);
    if(error != REG_SUCCESS) {
      Packet fail(DMSG_REGISTER_FAIL, 4);
      fail << (uint32_t)error;
      m_socket->sendData(fail);
    } else {
      Packet success(DMSG_REGISTER_SUCCESS, 0);
      m_socket->sendData(success);
    }

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
