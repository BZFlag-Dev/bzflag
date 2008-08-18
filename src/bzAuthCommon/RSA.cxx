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
#include <assert.h>
#include "RSA.h"
#include <gcrypt.h>
#include "Log.h"

INSTANTIATE_SINGLETON(RSAManager);

RSAKey::RSAKey()
{
  key = NULL;
}

RSAKey::~RSAKey()
{
  if(key) gcry_ac_key_destroy(key);
}

void RSAKey::_setKey(gcry_ac_key_t k)
{
  key = k;
}

uint8_t * RSAKey::_getValueN(size_t *n_len)
{
  gcry_ac_data_t data = gcry_ac_key_data_get(key);
  gcry_mpi_t mpi;
  if(gcry_ac_data_get_name(data, 0, "n", &mpi)) return NULL;
  uint8_t buf[1024];
  size_t len;
  if(gcry_mpi_print(GCRYMPI_FMT_STD, buf, 1024, &len, mpi)) return NULL;
  // the first byte is left 0 in this case (don't ask..)
  // this makes the returned length one more than it should be (129)
  // reconstructing the mpi doesn't work without this byte
  // TODO: maybe we can save a byte by trimming it here and adding it back there
  uint8_t *ret = (uint8_t*)malloc(len);
  memcpy(ret, buf, len);
  if(n_len) *n_len = len;
  return ret;
}

int RSAKey::_getValueE()
{
  gcry_ac_data_t data = gcry_ac_key_data_get(key);
  gcry_mpi_t mpi;
  if(gcry_ac_data_get_name(data, 0, "e", &mpi)) return 0;
  uint8_t buf[10];
  // the mpi doesn't store the last bytes of the value if they are 0
  // so clear the mem before interpreting the bytes as an integer
  memset(buf, 0, sizeof(buf));
  size_t len;
  if(gcry_mpi_print(GCRYMPI_FMT_STD, buf, 1024, &len, mpi)) return 0;
  return *(int*)buf;
}

bool RSAKey::setValues(uint8_t *n, size_t n_len, uint32_t e)
{
  gcry_ac_handle_t handle = sRSAManager._getHandle();

  gcry_ac_data_t data;
  if(gcry_ac_data_new(&data)) return false;

  // ac_data_destroy tries to destroy the names too, and data_set just stores a pointer
  // so we can't pass it staticly
  char *name_n = (char*)gcry_malloc(2);
  strcpy(name_n, "n");
  char *name_e = (char*)gcry_malloc(2);
  strcpy(name_e, "e");

  // note: when passing an mpi to scan, initialize to NULL (to prevent a memory leak)
  // and when passing to set_ui, use mpi_new (to prevent a crash) .. don't ask
  gcry_mpi_t mpi_n = NULL;
  if(gcry_mpi_scan(&mpi_n, GCRYMPI_FMT_STD, n, n_len, NULL)) return false;
  if(gcry_ac_data_set(data, GCRY_AC_FLAG_DEALLOC, name_n, mpi_n)) return false;

  gcry_mpi_t mpi_e = gcry_mpi_new(0);
  if(!gcry_mpi_set_ui(mpi_e, e)) return false;
  if(gcry_ac_data_set(data, GCRY_AC_FLAG_DEALLOC, name_e, mpi_e)) return false;

  gcry_ac_key_type type = getType() == RSA_KEY_PUBLIC ? GCRY_AC_KEY_PUBLIC : GCRY_AC_KEY_SECRET;

  if(gcry_ac_key_init(&key, handle, type, data)) return false;

  gcry_ac_data_destroy(data);
  return true;
}

RSAPublicKey::RSAPublicKey()
{
}

bool RSAPublicKey::encrypt(uint8_t *message, size_t message_len, uint8_t *&cipher, size_t &cipher_len)
{
  // NOTE: cipher must either be an array that's large enough to hold the output
  //       or a NULL pointer to signal the memory to be allocated inside this function
  //       in that case use RSAFree to free the memory
  assert(key);
  gcry_ac_io_t io_message, io_cipher;
  gcry_ac_io_init(&io_message, GCRY_AC_IO_READABLE, GCRY_AC_IO_STRING, message, message_len);
  gcry_ac_io_init(&io_cipher, GCRY_AC_IO_WRITABLE, GCRY_AC_IO_STRING, &cipher, &cipher_len);

  gcry_ac_handle_t handle = sRSAManager._getHandle();

  return gcry_ac_data_encrypt_scheme(handle, GCRY_AC_ES_PKCS_V1_5, 0, NULL, key, &io_message, &io_cipher) == 0;
}

RSASecretKey::RSASecretKey()
{
}

bool RSASecretKey::decrypt(uint8_t *cipher, size_t cipher_len, uint8_t *&message, size_t &message_len)
{
  // NOTE: message must either be an array that's large enough to hold the output
  //       or a NULL pointer to signal the memory to be allocated inside this function
  //       in that case use RSAFree to free the memory
  assert(key);
  gcry_ac_io_t io_cipher, io_message;
  gcry_ac_io_init(&io_cipher, GCRY_AC_IO_READABLE, GCRY_AC_IO_STRING, cipher, cipher_len);
  gcry_ac_io_init(&io_message, GCRY_AC_IO_WRITABLE, GCRY_AC_IO_STRING, &message, &message_len);

  gcry_ac_handle_t handle = sRSAManager._getHandle();

  return gcry_ac_data_decrypt_scheme(handle, GCRY_AC_ES_PKCS_V1_5, 0, NULL, key, &io_cipher, &io_message) == 0;
}

bool RSAKey::getValues(uint8_t *&n, size_t &n_len, uint32_t &e)
{
  assert(key);
  n = _getValueN(&n_len);
  e = _getValueE();
  return n != 0 && e != 0;
}

RSAManager::RSAManager()
{
  rsaHandle = 0;
}

RSAManager::~RSAManager()
{
  gcry_ac_close(rsaHandle);
}

bool RSAManager::initialize()
{
  gcry_check_version(NULL);
  gcry_control (GCRYCTL_DISABLE_SECMEM, 0);
  // with secmem enabled it crashes with the output
  // "operation is not possible without initialized secure memory"
  // .. but i haven't found any way to initialize it yet
  gcry_error_t ret = gcry_ac_open(&rsaHandle, GCRY_AC_RSA, 0);
  if(ret == 0) sLog.outLog("RSAManager: initialized");
  else sLog.outError("RSAManager: failed to open rsa handle, error code %d", ret);

  return ret == 0;
}

bool RSAManager::generateKeyPair()
{
  gcry_ac_key_pair_t key_pair;

  gcry_ac_key_spec_rsa_t rsa_spec;
  rsa_spec.e = NULL; //gcry_mpi_new (0);
  //the following method is recommended by the doc but it doesn't work as it should
  //- the value is stored in binary and it's "converted to an integer" using strtoul
  //- value 1 doesn't mean "default to the secure exponent"
  //gcry_mpi_set_ui (rsa_spec.e, 1);
  //the following seems to work on the other hand
  gcry_mpi_scan(&rsa_spec.e, GCRYMPI_FMT_STD, "65537", 5, NULL);

  gcry_error_t ret = gcry_ac_key_pair_generate(rsaHandle, 1024, (void*) &rsa_spec, &key_pair, NULL);
  gcry_mpi_release(rsa_spec.e);

  if(ret)
  {
    sLog.outError("RSAManager: Failed to generate key pair, error %d", ret);
    return false;
  }

  gcry_ac_key_t key, public_key, secret_key;
  gcry_ac_data_t data;

  key = gcry_ac_key_pair_extract(key_pair, GCRY_AC_KEY_PUBLIC);
  data = gcry_ac_key_data_get(key);
  ret = gcry_ac_key_init(&public_key, rsaHandle, GCRY_AC_KEY_PUBLIC, data);
  if(ret)
  {
    sLog.outError("RSAManager: Failed to initialize public key, error %d", ret);
    return false;
  }

  key = gcry_ac_key_pair_extract(key_pair, GCRY_AC_KEY_SECRET);
  data = gcry_ac_key_data_get(key);
  ret = gcry_ac_key_init(&secret_key, rsaHandle, GCRY_AC_KEY_SECRET, data);
  if(ret)
  {
    sLog.outError("RSAManager: Failed to initialize secret key, error %d", ret);
    return false;
  }

  // the key_init functions copy data, allowing the data in the keypair to be freed
  gcry_ac_key_pair_destroy(key_pair);

  publicKey._setKey(public_key);
  secretKey._setKey(secret_key);
  sLog.outLog("RSAManager: new key pair generated");
  return true;
}

void RSAManager::rsaFree(void *memory)
{
  gcry_free(memory);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8