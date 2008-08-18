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
#include <RSA.h>
#include <base64.h>
#include "assert.h"

void nputs(const char *str, size_t len)
{
    for(size_t i = 0; i < len; i++)
        putchar(str[i]);
}

size_t hashLen()
{
  return gcry_md_get_algo_dlen(GCRY_MD_MD5) / 2 * 3;
}

void hash(uint8_t *message, size_t message_len, uint8_t *digest)
{
  int md5len = gcry_md_get_algo_dlen(GCRY_MD_MD5);
  uint8_t *tmpbuf = new uint8_t[md5len];
  gcry_md_hash_buffer(GCRY_MD_MD5, tmpbuf, message, message_len);
  base64::encode(tmpbuf, tmpbuf + md5len, digest);
  delete[] tmpbuf;
}

void md5test()
{
    int digest_len = (int)hashLen();

    uint8_t *digest = new uint8_t[digest_len+1];
    memset(digest, 0, digest_len+1);
    uint8_t buffer[] = "password";
    size_t buffer_len = (int)strlen((const char*)buffer);

    hash(buffer, buffer_len, digest);

    printf("md5 base64 for %s is %s\n", buffer, digest);

    delete[] digest;
}

void test_gcrypt()
{
    //md5test(); return;

    if(!sRSAManager.initialize()) return;
    if(!sRSAManager.generateKeyPair()) return;

    // decompose the key into values that can be sent in packets
    size_t n_len;
    uint8_t *key_n;
    uint32_t e;
    if(!sRSAManager.getPublicKey().getValues(key_n, n_len, e)) return;
    assert(e == 65537);

    // create the key from its components
    RSAPublicKey publicKey;
    if(!publicKey.setValues(key_n, n_len, e)) return;

    free(key_n);

    // encrypt a message
    char message[] = "let's see if this gets encrypted/decrypted properly";
    char *cipher = NULL;
    size_t cipher_len = 0;

    if(!publicKey.encrypt((uint8_t*)message, strlen(message), (uint8_t*&)cipher, cipher_len)) return;
    printf("encrypted: "); nputs((char*)cipher, cipher_len); printf("\n");

    // decrypt the cipher
    char *output = NULL;
    size_t output_len = 0;

    if(!sRSAManager.getSecretKey().decrypt((uint8_t*)cipher, cipher_len, (uint8_t*&)output, output_len)) return;
    printf("decrypted: "); nputs(output, output_len); printf("\n");

    gcry_free(cipher);
    gcry_free(output);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8