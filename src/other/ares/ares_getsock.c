/* $Id: ares_getsock.c,v 1.5 2007-04-03 18:25:18 yangtse Exp $ */

/* Copyright (C) 2005 - 2007, Daniel Stenberg
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

#include "setup.h"

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "ares.h"
#include "ares_private.h"

int ares_getsock(ares_channel channel,
                 int *s,
                 int numsocks) /* size of the 'socks' array */
{
  struct server_state *server;
  int i;
  int sockindex=0;
  int bitmap = 0;
  unsigned int setbits = 0xffffffff;

  ares_socket_t *socks = (ares_socket_t *)s;

  /* No queries, no file descriptors. */
  if (!channel->queries)
    return 0;

  for (i = 0;
       (i < channel->nservers) && (sockindex < ARES_GETSOCK_MAXNUM);
       i++)
    {
      server = &channel->servers[i];
      if (server->udp_socket != ARES_SOCKET_BAD)
        {
          if(sockindex >= numsocks)
            break;
          socks[sockindex] = server->udp_socket;
          bitmap |= ARES_GETSOCK_READABLE(setbits, sockindex);
          sockindex++;
        }
      if (server->tcp_socket != ARES_SOCKET_BAD)
       {
         if(sockindex >= numsocks)
           break;
         socks[sockindex] = server->tcp_socket;
         bitmap |= ARES_GETSOCK_READABLE(setbits, sockindex);

         if (server->qhead)
           /* then the tcp socket is also writable! */
           bitmap |= ARES_GETSOCK_WRITABLE(setbits, sockindex);

         sockindex++;
       }
    }
  return bitmap;
}
