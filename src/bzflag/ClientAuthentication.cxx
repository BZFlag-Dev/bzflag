/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "ClientAuthentication.h"

/* system implementation headers */
#include <string>
#include <assert.h>
#ifdef HAVE_KRB5
#include <com_err.h>
#define err(x,y,z) if (debugLevel >= 1) com_err(x,y,z)
#endif

#include "DirectoryNames.h"

#ifdef HAVE_KRB5
krb5_context      ClientAuthentication::context      = NULL;
krb5_ccache       ClientAuthentication::cc           = NULL;
krb5_auth_context ClientAuthentication::auth_context = NULL;
krb5_data         ClientAuthentication::packet;
krb5_data         ClientAuthentication::inbuf;
krb5_creds        ClientAuthentication::creds;
krb5_creds       *ClientAuthentication::new_creds;
krb5_principal    ClientAuthentication::client;
krb5_principal    ClientAuthentication::server;
char              ClientAuthentication::principalName[128];
#endif
bool              ClientAuthentication::authentication = false;

#ifdef HAVE_KRB5
void ClientAuthentication::init(const char *username, const char *password)
#else
void ClientAuthentication::init(const char *username, const char *)
#endif // HAVE_KRB5
{
  strncpy(principalName, username, 128);
  principalName[127] = 0;

#ifdef HAVE_KRB5
  assert(context == NULL);
  assert(cc == NULL);

  krb5_error_code retval;
  krb5_creds      my_creds;

  // Initializing kerberos library
  if ((retval = krb5_init_context(&context)))
    err("bzflag:", retval, "while initializing krb5");
  // Getting a default cache specifically for bzflag
  std::string ccfile = "FILE:" + getConfigDirName() + "krb5_cc";
  if (context && (retval
		  = krb5_cc_set_default_name(context, ccfile.c_str())))
    err("bzflag", retval, "setting default cache");
  // Getting credential cache 
  if (!retval && (retval = krb5_cc_default(context, &cc)))
    err("bzflag:", retval, "getting credentials cache");

  char clientName[139];
  snprintf(clientName, 139, "%s@BZFLAG.ORG", username);
  if (cc && (retval = krb5_parse_name(context, clientName, &client)))
    err("bzflag", retval, "parsing principal name");

  // Initing credential cache
  if (!retval && (retval = krb5_cc_initialize(context, cc, client)))
    err("bzflag", retval, "initializing credential cache");

  char intPassword[128];
  strncpy(intPassword, password, 128);
  intPassword[127] = 0;
  // Get credentials for server
  if (!retval && (retval = krb5_get_init_creds_password(context, &my_creds,
 							client, intPassword,
 							krb5_prompter_posix,
 							NULL, 0, NULL, NULL)))
    err("bzflag", retval, "getting credential");

  // Store credentials in cache
  if (!retval && (retval = krb5_cc_store_cred(context, cc, &my_creds)))
    err("bzflag", retval, "storing credential in cache");

  if (!retval && (retval = krb5_parse_name(context,
					   "krbtgt/BZFLAG.ORG@BZFLAG.ORG",
					   &server)))
    err("bzflag:", retval, "setting up tgt server name");
  authentication = (retval == 0);
#endif
}

#ifdef HAVE_KRB5
void ClientAuthentication::sendCredential(ServerLink &serverLink)
#else
void ClientAuthentication::sendCredential(ServerLink&)
#endif // HAVE_KRB5
{
  if (!authentication)
    return;
#ifdef HAVE_KRB5
  assert(context != NULL);
  assert(cc != NULL);

  krb5_error_code retval;
  /* Get credentials for server */
  memset((char*)&creds, 0, sizeof(creds));
  memcpy(&creds.client, &client, sizeof(client)); 
  memcpy(&creds.server, &server, sizeof(server)); 
  /* Get credentials for server */
  if ((retval = krb5_get_credentials(context, KRB5_GC_CACHED, cc, &creds,
				     &new_creds)))
    err("bzflag:", retval, "getting TGT");
  if (!retval) {
    serverLink.sendKerberosTicket(principalName, &new_creds->ticket);
  }
#endif
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
