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

#ifndef _BZAUTHD_TEST_COMMON_H_
#define _BZAUTHD_TEST_COMMON_H_

// the net test will use windows threads, disable it when doing builds for other platforms
#define TEST_NET

#include <stdio.h>
#include <conio.h>
#define LDAP_DEPRECATED 1
#include <ldap.h>
#include <gcrypt.h>
#include <net.h>

#ifdef TEST_NET
#include <windows.h>
#include <process.h>
#endif

void test_ldap();
void test_gcrypt();
void test_net();

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
