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
char*             ClientAuthentication::principalName;
#endif
bool              ClientAuthentication::authentication = false;

void ClientAuthentication::init()
{
#ifdef HAVE_KRB5
  assert(context == NULL);
  assert(cc == NULL);
  krb5_error_code retval;
  // Initializing kerberos library
  if ((retval = krb5_init_context(&context)))
    err("bzflag:", retval, "while initializing krb5");
  // Getting credential cache 
  if (context && (retval = krb5_cc_default(context, &cc)))
    err("bzflag:", retval, "getting credentials cache");
  if (cc && (retval = krb5_cc_get_principal(context, cc, &client)))
    err("bzflag:", retval, "getting principal name");
  if (!retval && (retval = krb5_unparse_name(context, client, &principalName)))
    err("bzflag:", retval, "unparsing principal name");
  if (context && (retval = krb5_parse_name(context,
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
  assert(principalName != NULL);

  char simpleName[128];
  int i;
  strncpy(simpleName, principalName, 128);
  for (i = 0; i < 127 && simpleName[i] && (simpleName[i] != '@'); i++);
  simpleName[i] = 0;

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
    serverLink.sendKerberosTicket(simpleName, &new_creds->ticket);
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
