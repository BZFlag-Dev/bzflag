/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <signal.h>

/* some platforms don't have a SIG_PF type. */
#ifndef SIG_PF
typedef void (*SIG_PF)(int);
#endif

#ifdef __cplusplus 
extern "C" {
#endif 

SIG_PF bzSignal(int signo, SIG_PF func);

#ifdef __cplusplus 
}
#endif
