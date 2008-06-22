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
    rsa_spec.e = NULL; //gcry_mpi_new (0);
    //the following method is recommended by the doc but it doesn't work as it should
    //- the value is stored in binary and it's "converted to an integer" using strtoul
    //- value 1 doesn't mean "default to the secure exponent"
    //gcry_mpi_set_ui (rsa_spec.e, 1);
    //the following seems to work on the other hand
    gcry_mpi_scan(&rsa_spec.e, GCRYMPI_FMT_STD, "65537", 5, NULL);

    gcry_error_t ret = gcry_ac_key_pair_generate(handle, 1024, (void*) &rsa_spec, &key_pair, NULL);
    gcry_mpi_release(rsa_spec.e);

    if(ret) return ret;

    gcry_ac_key_t key;
    gcry_ac_data_t data;

    key = gcry_ac_key_pair_extract(key_pair, GCRY_AC_KEY_PUBLIC);
    data = gcry_ac_key_data_get(key);
    ret = gcry_ac_key_init(public_key, handle, GCRY_AC_KEY_PUBLIC, data);

    key = gcry_ac_key_pair_extract(key_pair, GCRY_AC_KEY_SECRET);
    data = gcry_ac_key_data_get(key);
    ret = gcry_ac_key_init(secret_key, handle, GCRY_AC_KEY_SECRET, data);

    // the key_init functions copy data, allowing the data in the keypair to be freed
    gcry_ac_key_pair_destroy(key_pair);
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

char * test_rsa_get_key_n(gcry_ac_key_t &key, size_t *n_len)
{
    gcry_ac_data_t data = gcry_ac_key_data_get(key);
    gcry_mpi_t mpi;
    if(gcry_ac_data_get_name(data, 0, "n", &mpi)) return NULL;
    unsigned char buf[1024];
    size_t len;
    if(gcry_mpi_print(GCRYMPI_FMT_STD, buf, 1024, &len, mpi)) return NULL;
    // the first byte is left 0 in this case (don't ask..)
    // this makes the returned length one more than it should be (129)
    // reconstructing the mpi doesn't work without this byte
    // TODO: maybe we can save a byte by trimming it here and adding it back there
    char *ret = (char*)malloc(len);
    memcpy(ret, buf, len);
    if(n_len) *n_len = len;
    return ret;
}

int test_rsa_get_key_e(gcry_ac_key_t &key)
{
    gcry_ac_data_t data = gcry_ac_key_data_get(key);
    gcry_mpi_t mpi;
    if(gcry_ac_data_get_name(data, 0, "e", &mpi)) return 0;
    unsigned char buf[10];
    // the mpi doesn't store the last bytes of the value if they are 0
    // so clear the mem before interpreting the bytes as an integer
    memset(buf, 0, sizeof(buf));
    size_t len;
    if(gcry_mpi_print(GCRYMPI_FMT_STD, buf, 1024, &len, mpi)) return 0;
    return *(int*)buf;
}

gcry_ac_key_t test_rsa_make_key(gcry_ac_handle_t handle, gcry_ac_key_type_t type, char *n, size_t n_len, int e)
{
    gcry_ac_key_t key;
    gcry_ac_data_t data;
    if(gcry_ac_data_new(&data)) return NULL;

    // ac_data_destroy tries to destroy the names too, and data_set just stores a pointer
    // so we can't pass it staticly
    char *name_n = (char*)gcry_malloc(2);
    strcpy(name_n, "n");
    char *name_e = (char*)gcry_malloc(2);
    strcpy(name_e, "e");

    // note: when passing an mpi to scan, initialize to NULL (to prevent a memory leak)
    // and when passing to set_ui, use mpi_new (to prevent a crash) .. don't ask
    gcry_mpi_t mpi_n = NULL;
    if(gcry_mpi_scan(&mpi_n, GCRYMPI_FMT_STD, n, n_len, NULL)) return NULL;
    if(gcry_ac_data_set(data, GCRY_AC_FLAG_DEALLOC, name_n, mpi_n)) return NULL;

    gcry_mpi_t mpi_e = gcry_mpi_new(0);
    if(!gcry_mpi_set_ui(mpi_e, 65537)) return NULL;
    if(gcry_ac_data_set(data, GCRY_AC_FLAG_DEALLOC, name_e, mpi_e)) return NULL;

    if(gcry_ac_key_init(&key, handle, type, data)) return NULL;

    gcry_ac_data_destroy(data);
    return key;
}

void test_gcrypt()
{
    gcry_error_t err;

    gcry_ac_handle_t handle;
    err = test_rsa_init(&handle);

    gcry_ac_key_t public_key, secret_key;
    err = test_rsa_gen_key_pair(handle, &public_key, &secret_key);
    if(err) return;

    size_t n_len;
    char *key_n = test_rsa_get_key_n(public_key, &n_len);
    if(test_rsa_get_key_e(public_key) != 65537) return;

    gcry_ac_key_t public_key_1 = test_rsa_make_key(handle, GCRY_AC_KEY_PUBLIC, key_n, n_len, 65537);
    if(!public_key_1) return;

    free(key_n);

    char message[] = "let's see if this gets encrypted/decrypted properly";
    char *cipher = NULL;
    size_t cipher_len = 0;

    err = test_rsa_encrypt(handle, public_key_1, message, strlen(message), &cipher, &cipher_len);
    if(err) return;
    printf("encrypted: "); nputs(cipher, cipher_len); printf("\n");

    char *output = NULL;
    size_t output_len = 0;

    err = test_rsa_decrypt(handle, secret_key, cipher, cipher_len, &output, &output_len);
    if(err) return;
    printf("decrypted: "); nputs(output, output_len); printf("\n");

    gcry_free(cipher);
    gcry_free(output);

    gcry_ac_key_destroy(public_key_1);
    gcry_ac_key_destroy(public_key);
    gcry_ac_key_destroy(secret_key);
    gcry_ac_close(handle);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8