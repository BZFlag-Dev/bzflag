/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __BZFS_H__
#define __BZFS_H__

#ifdef _MSC_VER
#pragma warning( 4 : 4786 )
#endif

// to dump score info to stdout
#define PRINTSCORE to include code to dump score info to stdout

#define SERVERLOGINMSG true

#include "common.h"

// must be before network.h because that defines a close() macro which
// messes up fstreams.	luckily, we don't need to call the close() method
// on any fstream.
#include <fstream>

// must be before windows.h
#include "network.h"

// common interface headers
#include "Address.h"

// bzfs specific headers
#include "TeamBases.h"

extern void sendMessage(int         playerIndex,
			PlayerId    dstPlayer,
			const char *message);

extern BasesList bases;

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

