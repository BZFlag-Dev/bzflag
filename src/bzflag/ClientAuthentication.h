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

#ifndef BZF_CLIENT_AUTHENTICATION_H
#define BZF_CLIENT_AUTHENTICATION_H

/* bzflag special common - 1st one */
#include "common.h"


/* system implementation headers */
#ifdef HAVE_KRB5
	// because kebos dosn't asume anyone else but them has defines.
	#ifdef MAXHOSTNAMELEN
		#undef MAXHOSTNAMELEN
	#endif 
#include <krb5.h>
#endif

/* local implementation headers */
#include "ServerLink.h"

class ClientAuthentication {
 public:
  static void init();
  static void sendCredential(ServerLink &serverLink);
private:
#ifdef HAVE_KRB5
  static krb5_context      context;
  static krb5_ccache       cc;
  static krb5_auth_context auth_context;
  static krb5_data         packet;
  static krb5_data         inbuf;
  static krb5_creds        creds;
  static krb5_creds       *new_creds;
  static krb5_principal    client;
  static krb5_principal    server;
  static char             *principalName;
#endif
  static bool              authentication;
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
