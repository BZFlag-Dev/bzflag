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

#ifndef BZF_AUTHENTICATION_H
	#define BZF_AUTHENTICATION_H

/* bzflag special common - 1st one */
	#include "common.h"

/* system implementation headers */
	#ifdef HAVE_KRB5
		#ifdef MAXHOSTNAMELEN
			#undef MAXHOSTNAMELEN
		#endif 
		#include <krb5.h>
	#endif 
	#include <string>

class Authentication
{
public:
	Authentication();

	static void cleanUp();
	static void init( const char *address, int port, const char *password );
	void setPrincipalName( char *buf, int len );
	void verifyCredential( char *buf, int len );
	bool isTrusted()
	{
		return trusted;
	};
	std::string getPrincipal()
	{
		return principalName;
	};
	bool isGlobal( void )
	{
		return globallyAuthenticated;
	};
	void global( bool set )
	{
		globallyAuthenticated = set;
	}
private:
	#ifdef HAVE_KRB5
	static krb5_context context;
	static krb5_ccache cc;
	static krb5_principal client;
	krb5_principal server;
	static krb5_creds my_creds;
	static char ccfile[MAXPATHLEN + 6]; // FILE:path+\0
	#endif 
	static bool authentication;
	bool trusted;
	std::string principalName;
	bool globallyAuthenticated;
};

#endif 

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
