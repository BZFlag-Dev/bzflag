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

#ifndef __BZAUTHD_RSA_H__
#define __BZAUTHD_RSA_H__

#include "Platform.h"

typedef struct gcry_ac_key *gcry_ac_key_t;
typedef struct gcry_ac_handle *gcry_ac_handle_t;

enum RSAKeyType
{
  RSA_KEY_PUBLIC,
  RSA_KEY_SECRET
};

class RSAKey
{
public:
  RSAKey();
  virtual ~RSAKey();
  void setValues(uint8 *n, size_t n_len, uint32 e);
  void getValues(uint8 *&n, size_t &n_len, uint32 &e);
  virtual RSAKeyType getType() = 0;
private:
  gcry_ac_key_t key;
};

class RSAPublicKey : public RSAKey
{
public:
  RSAPublicKey();
  void encrypt(uint8 *message, size_t message_len, uint8 *cipher, size_t &cipher_len);
  RSAKeyType getType() { return RSA_KEY_PUBLIC; }
};

class RSASecretKey : public RSAKey
{
public:
  RSASecretKey();
  void decrypt(uint8 *cipher, size_t cipher_len, uint8 *message, size_t &message_len);
  RSAKeyType getType() { return RSA_KEY_SECRET; }
};

class RSAManager : public Singleton<RSAManager>
{
public:
  void initialize();
  void generateKeyPair();
  RSAPublicKey &getPublicKey() { return publicKey; }
  RSASecretKey &getSecretKey() { return secretKey; }

private:
  RSAPublicKey publicKey;
  RSASecretKey secretKey;
  gcry_ac_handle_t rsaHandle;
};

#endif // __BZAUTHD_RSA_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8