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

void nputs(const char *str, size_t len)
{
    for(size_t i = 0; i < len; i++)
        putchar(str[i]);
}

gcry_error_t test_rsa_init(gcry_ac_handle_t *handle)
{
    gcry_check_version(NULL);
    gcry_control (GCRYCTL_DISABLE_SECMEM, 0);
    // with secmem enabled it crashes with the output
    // "operation is not possible without initialized secure memory"
    // .. but i haven't found any way to initialize it yet
    gcry_error_t ret = gcry_ac_open(handle, GCRY_AC_RSA, 0);
    return ret;
}

gcry_error_t test_rsa_gen_key_pair(gcry_ac_handle_t handle, gcry_ac_key_t *public_key, gcry_ac_key_t *secret_key)
{
    gcry_ac_key_pair_t key_pair;

    gcry_ac_key_spec_rsa_t rsa_spec;
    rsa_spec.e = gcry_mpi_new (0);
    gcry_mpi_set_ui (rsa_spec.e, 1);

    gcry_error_t ret = gcry_ac_key_pair_generate(handle, 1024, (void*) &rsa_spec, &key_pair, NULL);
    gcry_mpi_release(rsa_spec.e);

    if(ret) return ret;
    *public_key = gcry_ac_key_pair_extract(key_pair, GCRY_AC_KEY_PUBLIC);
    *secret_key = gcry_ac_key_pair_extract(key_pair, GCRY_AC_KEY_SECRET);
    // if i destroy key_pair, the the data will become broken in the extracted keys
    return ret;
}

gcry_error_t test_rsa_encrypt(gcry_ac_handle_t handle, gcry_ac_key_t &public_key, const char *message, size_t message_len, char **cipher, size_t *cipher_len)
{
    gcry_ac_io_t io_message, io_cipher;
    gcry_ac_io_init(&io_message, GCRY_AC_IO_READABLE, GCRY_AC_IO_STRING, message, message_len);
    gcry_ac_io_init(&io_cipher, GCRY_AC_IO_WRITABLE, GCRY_AC_IO_STRING, cipher, cipher_len);

    return gcry_ac_data_encrypt_scheme(handle, GCRY_AC_ES_PKCS_V1_5, 0, NULL, public_key, &io_message, &io_cipher);
}

gcry_error_t test_rsa_decrypt(gcry_ac_handle_t handle, gcry_ac_key_t &secret_key, const char *cipher, size_t cipher_len, char **message, size_t *message_len)
{
    gcry_ac_io_t io_cipher, io_message;
    gcry_ac_io_init(&io_cipher, GCRY_AC_IO_READABLE, GCRY_AC_IO_STRING, cipher, cipher_len);
    gcry_ac_io_init(&io_message, GCRY_AC_IO_WRITABLE, GCRY_AC_IO_STRING, message, message_len);

    return gcry_ac_data_decrypt_scheme(handle, GCRY_AC_ES_PKCS_V1_5, 0, NULL, secret_key, &io_cipher, &io_message);
}

void test_gcrypt()
{
    gcry_error_t err;

    gcry_ac_handle_t handle;
    err = test_rsa_init(&handle);

    gcry_ac_key_t public_key, secret_key;
    err = test_rsa_gen_key_pair(handle, &public_key, &secret_key);
    if(err) return;

    char message[] = "let's see if this gets encrypted/decrypted properly";
    char *cipher = NULL;
    size_t cipher_len = 0;

    err = test_rsa_encrypt(handle, public_key, message, strlen(message), &cipher, &cipher_len);
    if(err) return;
    printf("encrypted: "); nputs(cipher, cipher_len); printf("\n");

    char *output = NULL;
    size_t output_len = 0;
 
    err = test_rsa_decrypt(handle, secret_key, cipher, cipher_len, &output, &output_len);
    if(err) return;
    printf("decrypted: "); nputs(output, output_len); printf("\n");

    gcry_free(cipher);
    gcry_free(output);

    gcry_ac_key_destroy(public_key);
    gcry_ac_key_destroy(secret_key);
    // this still leaves leaking memory allocated in the original key_pair
    // but calling destroy the original key_pair after the last two lines crashes
    // and instead of them it doesn't crash but still leaks
    // so far it seems to be a bug in the library
    gcry_ac_close(handle);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8