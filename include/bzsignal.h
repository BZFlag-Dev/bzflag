/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <signal.h>

#if !defined(__sgi)
/* some platforms don't have a SIG_PF type. */
#ifndef SIG_PF
typedef void (*SIG_PF)(int);
#endif
#endif /* defined(__sgi) */

#ifdef __cplusplus
extern "C" {
#endif

SIG_PF bzSignal(int signo, SIG_PF func);

#ifdef __cplusplus
}
#endif

/*
 * Local Variables: ***
 * mode:C++ ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */
