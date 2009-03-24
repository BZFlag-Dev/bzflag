#ifndef __CONNECT_H
#define __CONNECT_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2008, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * $Id: connect.h,v 1.26 2008-11-03 16:24:56 bagder Exp $
 ***************************************************************************/

int Curl_nonblock(curl_socket_t sockfd,    /* operate on this */
                  int nonblock   /* TRUE or FALSE */);

CURLcode Curl_is_connected(struct connectdata *conn,
                           int sockindex,
                           bool *connected);

CURLcode Curl_connecthost(struct connectdata *conn,
                          const struct Curl_dns_entry *host, /* connect to
                                                                this */
                          curl_socket_t *sockconn, /* not set if error */
                          Curl_addrinfo **addr, /* the one we used */
                          bool *connected); /* truly connected? */

/* generic function that returns how much time there's left to run, according
   to the timeouts set */
long Curl_timeleft(struct connectdata *conn,
                   struct timeval *nowp,
                   bool duringconnect);

#define DEFAULT_CONNECT_TIMEOUT 300000 /* milliseconds == five minutes */

/*
 * Used to extract socket and connectdata struct for the most recent
 * transfer on the given SessionHandle.
 *
 * The socket 'long' will be -1 in case of failure!
 */
CURLcode Curl_getconnectinfo(struct SessionHandle *data,
                             long *param_longp,
                             struct connectdata **connp);
#endif
