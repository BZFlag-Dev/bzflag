/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "Authentication.h"


// because kebos dosn't asume anyone else but them has defines.
#ifdef MAXHOSTNAMELEN
#undef MAXHOSTNAMELEN
#endif

/* system implementation headers */
#ifdef HAVE_KRB5
#include <com_err.h>
#endif
#include <sys/types.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <assert.h>

#include "DirectoryNames.h"

#ifdef HAVE_KRB5
krb5_context   Authentication::context	= NULL;
krb5_ccache    Authentication::cc	     = NULL;
krb5_principal Authentication::client;
krb5_creds     Authentication::my_creds;
char	   Authentication::ccfile[MAXPATHLEN+6]; // FILE:path+\0
#endif
bool	   Authentication::authentication = false;

Authentication::Authentication() : trusted(false), globallyAuthenticated(false)
{
}

#ifdef HAVE_KRB5
void Authentication::cleanUp()
{
  krb5_error_code retval;

  if (!authentication)
    return;

  if ((retval = krb5_cc_destroy(context, cc)))
    com_err("bzfs:", retval, "while destroying credential cache");
}
#else
void Authentication::cleanUp()
{
}
#endif

#ifdef HAVE_KRB5
void Authentication::init(const char *address, int port, const char *password)
{
  assert(context == NULL);
  assert(cc == NULL);

  krb5_error_code retval;

  if (!address)
    return;

  int i;
  char serverName[128];
  strncpy(serverName, address, 128);
  for (i = 0; i < 127 && serverName[i] != 0 && serverName[i] != ':'; i++);

  // With no public address we cannot do auth
  if (!i)
    return;
  serverName[i] = 0;

  // Initializing kerberos library
  if ((retval = krb5_init_context(&context)))
    com_err("bzfs:", retval, "while initializing krb5");

  // Gettin a default cache different for any bzfs process
  sprintf(ccfile, "FILE:%skrb5cc_p%ld", getTempDirName().c_str(), (long)getpid());
  if (context && (retval = krb5_cc_set_default_name(context, ccfile)))
    com_err("bzfs:", retval, "setting default credential cache");
  unlink(ccfile+strlen("FILE:"));

  // Getting credential cache
  if (!retval && (retval = krb5_cc_default(context, &cc)))
    com_err("bzfs:", retval, "getting credentials cache");

  // Getting principal identifier
  char name[1024];
  snprintf(name, sizeof(name), "%d/%s@BZFLAG.ORG", port, serverName);
  if (cc && (retval = krb5_parse_name(context, name, &client)))
    com_err("bzfs:", retval, "getting principal name");

  // Initing credential cache
  if (!retval && (retval = krb5_cc_initialize(context, cc, client)))
    com_err("bzfs:", retval, "initializing credential cache");

  char intPassword[128];
  strncpy(intPassword, password, 128);
  intPassword[127] = 0;
  // Get credentials for server
  if (!retval && (retval = krb5_get_init_creds_password(context, &my_creds,
							client, intPassword,
							krb5_prompter_posix,
							NULL, 0, NULL, NULL)))
    com_err("bzfs:", retval, "getting credential");

  // Store credentials in cache
  if (!retval && (retval = krb5_cc_store_cred(context, cc, &my_creds)))
    com_err("bzfs:", retval, "storing credential in cache");

  if (!retval)
    authentication = true;
}
#else
void Authentication::init(const char *, int , const char *)
{
}
#endif

void Authentication::setPrincipalName(char *buf, int len)
{
  if (len > 1023)
    return;

  // Saving principal name
  char name[1024];
  memcpy(name, buf, len);
  name[len] = 0;
  principalName = name;

  if (!authentication)
    return;

#ifdef HAVE_KRB5
  krb5_error_code retval;
  char	    remotePrincipal[1024];

  buf[len] = 0;
  snprintf(remotePrincipal, 1024, "%s@BZFLAG.ORG", name);

  if ((retval = krb5_parse_name(context, remotePrincipal, &server)))
    com_err("bzfs", retval, "parsing remote name");
#endif
}

#ifdef HAVE_KRB5
void Authentication::verifyCredential(char *buf, int len)
{
  if (!authentication)
    return;

  krb5_creds	creds;
  krb5_error_code   retval;
  krb5_creds       *new_creds;

  memset((char*)&creds, 0, sizeof(creds));
  memcpy(&creds.client, &client, sizeof(client));
  memcpy(&creds.server, &server, sizeof(server));

  creds.second_ticket.length = len;
  creds.second_ticket.data   = buf;

  // Check authentication info
  if ((retval = krb5_get_credentials(context, KRB5_GC_USER_USER, cc, &creds,
				     &new_creds)))
    com_err("bzfs", retval, "getting user-user ticket");
  else
    trusted = true;
}
#else
void Authentication::verifyCredential(char *, int)
{
}
#endif // HAVE_KRB5

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
