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
#include "../bzauthd/RSA.h"

void nputs(const char *str, size_t len)
{
    for(size_t i = 0; i < len; i++)
        putchar(str[i]);
}

void test_gcrypt()
{
    if(!sRSAManager.initialize()) return;
    if(!sRSAManager.generateKeyPair()) return;

    // decompose the key into values that can be set in packets
    size_t n_len;
    uint8 *key_n;
    uint32 e;
    sRSAManager.getPublicKey().getValues(key_n, n_len, e);
    if(e != 65537) return;

    // create the key from its components
    RSAPublicKey publicKey;
    publicKey.setValues(key_n, n_len, e);

    free(key_n);

    // encrypt a message
    char message[] = "let's see if this gets encrypted/decrypted properly";
    char *cipher = NULL;
    size_t cipher_len = 0;

    publicKey.encrypt((uint8*)message, strlen(message), (uint8*&)cipher, cipher_len);
    printf("encrypted: "); nputs((char*)cipher, cipher_len); printf("\n");

    // decrypt the cipher
    char *output = NULL;
    size_t output_len = 0;

    sRSAManager.getSecretKey().decrypt((uint8*)cipher, cipher_len, (uint8*&)output, output_len);
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