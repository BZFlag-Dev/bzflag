#ifndef __SERVER_UTIL_H
#define __SERVER_UTIL_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2007, Daniel Stenberg, <daniel@haxx.se>, et al.
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
 * $Id: util.h,v 1.13 2007-02-22 02:51:55 yangtse Exp $
 ***************************************************************************/

void logmsg(const char *msg, ...);

#define TEST_DATA_PATH "%s/data/test%ld"

/* global variable, where to find the 'data' dir */
extern const char *path;

#ifdef WIN32
#include <process.h>
#include <fcntl.h>

#define sleep(sec)   Sleep ((sec)*1000)

#undef perror
#define perror(m) win32_perror(m)
void win32_perror (const char *msg);
#endif  /* WIN32 */

#ifdef USE_WINSOCK
void win32_init(void);
void win32_cleanup(void);
#endif  /* USE_WINSOCK */

/* returns the path name to the test case file */
char *test2file(long testno);

#endif  /* __SERVER_UTIL_H */
