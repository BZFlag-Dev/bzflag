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

#include "bzsignal.h"

/* Unix systems implemnt signal() differently */
/* modified from "UNIX Network Programming" */
SIG_PF bzSignal(int signo, SIG_PF func)
{
#ifdef _WIN32
  return signal(signo, func);
#else /* _WIN32 */
  struct sigaction act, oact;

  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
#ifdef SA_NODEFER
  act.sa_flags = SA_NODEFER;
#else
  act.sa_flags = 0;
#endif
  if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
    /* SunOS 4.x */
    act.sa_flags |= SA_INTERRUPT;
#endif
  } else {
#ifdef SA_RESTART
    /* SVR4, 4.4BSD */
    act.sa_flags |= SA_RESTART;
#endif
  }
  if (sigaction(signo, &act, &oact) < 0)
    return SIG_ERR;
  return oact.sa_handler;
#endif /* _WIN32 */
}

/*
 * Local Variables: ***
 * mode:C ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */
