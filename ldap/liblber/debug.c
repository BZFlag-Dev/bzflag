/* $OpenLDAP: pkg/ldap/libraries/liblber/debug.c,v 1.18.2.3 2007/01/02 21:43:48 kurt Exp $ */
/* This work is part of OpenLDAP Software <http://www.openldap.org/>.
 *
 * Copyright 1998-2007 The OpenLDAP Foundation.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted only as authorized by the OpenLDAP
 * Public License.
 *
 * A copy of this license is available in the file LICENSE in the
 * top-level directory of the distribution or, alternatively, at
 * <http://www.OpenLDAP.org/license.html>.
 */

#include "portable.h"

#include <stdio.h>

#include <ac/stdarg.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>
#include <ac/ctype.h>

#ifdef LDAP_SYSLOG
#include <ac/syslog.h>
#endif

#include "ldap_log.h"
#include "ldap_defaults.h"
#include "lber.h"
#include "ldap_pvt.h"

static FILE *log_file = NULL;

#ifdef LDAP_SYSLOG
static int use_syslog = 0;

static int debug2syslog(int l) {
	switch (l) {
	case LDAP_LEVEL_EMERG: return LOG_EMERG;
	case LDAP_LEVEL_ALERT: return LOG_ALERT;
	case LDAP_LEVEL_CRIT: return LOG_CRIT;
	case LDAP_LEVEL_ERR: return LOG_ERR;
	case LDAP_LEVEL_WARNING: return LOG_WARNING;
	case LDAP_LEVEL_NOTICE: return LOG_NOTICE;
	case LDAP_LEVEL_INFO: return LOG_INFO;
	}
	return LOG_DEBUG;
}
#endif

int lutil_debug_file( FILE *file )
{
	log_file = file;
	ber_set_option( NULL, LBER_OPT_LOG_PRINT_FILE, file );

	return 0;
}

void (lutil_debug)( int debug, int level, const char *fmt, ... )
{
	char buffer[4096];
	va_list vl;

	if ( !(level & debug ) ) return;

#ifdef HAVE_WINSOCK
	if( log_file == NULL ) {
		log_file = fopen( LDAP_RUNDIR LDAP_DIRSEP "openldap.log", "w" );

		if ( log_file == NULL ) {
			log_file = fopen( "openldap.log", "w" );
			if ( log_file == NULL ) return;
		}

		ber_set_option( NULL, LBER_OPT_LOG_PRINT_FILE, log_file );
	}
#endif

	va_start( vl, fmt );
	vsnprintf( buffer, sizeof(buffer), fmt, vl );
	buffer[sizeof(buffer)-1] = '\0';
	if( log_file != NULL ) {
		fputs( buffer, log_file );
		fflush( log_file );
	}
	fputs( buffer, stderr );
	va_end( vl );
}

#if defined(HAVE_EBCDIC) && defined(LDAP_SYSLOG)
#undef syslog
void eb_syslog( int pri, const char *fmt, ... )
{
	char buffer[4096];
	va_list vl;

	va_start( vl, fmt );
	vsnprintf( buffer, sizeof(buffer), fmt, vl );
	buffer[sizeof(buffer)-1] = '\0';

	/* The syslog function appears to only work with pure EBCDIC */
	__atoe(buffer);
#pragma convlit(suspend)
	syslog( pri, "%s", buffer );
#pragma convlit(resume)
	va_end( vl );
}
#endif
