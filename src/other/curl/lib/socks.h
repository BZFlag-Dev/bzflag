#ifndef __SOCKS_H
#define __SOCKS_H
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
 * $Id: socks.h,v 1.6 2008-01-02 21:40:12 bagder Exp $
 ***************************************************************************/

/*
 * This function logs in to a SOCKS4(a) proxy and sends the specifics to the
 * final destination server.
 */
CURLcode Curl_SOCKS4(const char *proxy_name,
                     const char *hostname,
                     int remote_port,
                     int sockindex,
                     struct connectdata *conn,
                     bool protocol4a);

/*
 * This function logs in to a SOCKS5 proxy and sends the specifics to the
 * final destination server.
 */
CURLcode Curl_SOCKS5(const char *proxy_name,
                     const char *proxy_password,
                     const char *hostname,
                     int remote_port,
                     int sockindex,
                     struct connectdata *conn);

#endif
