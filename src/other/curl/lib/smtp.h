#ifndef __SMTP_H
#define __SMTP_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 2009 - 2010, Daniel Stenberg, <daniel@haxx.se>, et al.
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
 ***************************************************************************/

#include "pingpong.h"

/****************************************************************************
 * SMTP unique setup
 ***************************************************************************/
typedef enum {
  SMTP_STOP,        /* do nothing state, stops the state machine */
  SMTP_SERVERGREET, /* waiting for the initial greeting immediately after
                       a connect */
  SMTP_EHLO,
  SMTP_HELO,
  SMTP_STARTTLS,
  SMTP_UPGRADETLS, /* asynchronously upgrade the connection to SSL/TLS (multi mode only) */
  SMTP_AUTHPLAIN,
  SMTP_AUTHLOGIN,
  SMTP_AUTHPASSWD,
  SMTP_AUTHCRAM,
  SMTP_AUTH,
  SMTP_MAIL, /* MAIL FROM */
  SMTP_RCPT, /* RCPT TO */
  SMTP_DATA,
  SMTP_POSTDATA,
  SMTP_QUIT,
  SMTP_LAST  /* never used */
} smtpstate;

/* smtp_conn is used for struct connection-oriented data in the connectdata
   struct */
struct smtp_conn {
  struct pingpong pp;
  char *domain;    /* what to send in the EHLO */
  size_t eob;         /* number of bytes of the EOB (End Of Body) that has been
                         received thus far */
  unsigned int authmechs;       /* Accepted authentication methods. */
  smtpstate state; /* always use smtp.c:state() to change state! */
  struct curl_slist *rcpt;
  bool ssldone; /* is connect() over SSL done? only relevant in multi mode */
};

/* Authentication mechanism flags. */
#define SMTP_AUTH_LOGIN         0x0001
#define SMTP_AUTH_PLAIN         0x0002
#define SMTP_AUTH_CRAM_MD5      0x0004
#define SMTP_AUTH_DIGEST_MD5    0x0008
#define SMTP_AUTH_GSSAPI        0x0010
#define SMTP_AUTH_EXTERNAL      0x0020

extern const struct Curl_handler Curl_handler_smtp;
extern const struct Curl_handler Curl_handler_smtps;

/* this is the 5-bytes End-Of-Body marker for SMTP */
#define SMTP_EOB "\x0d\x0a\x2e\x0d\x0a"
#define SMTP_EOB_LEN 5

/* if found in data, replace it with this string instead */
#define SMTP_EOB_REPL "\x0d\x0a\x2e\x2e"
#define SMTP_EOB_REPL_LEN 4

CURLcode Curl_smtp_escape_eob(struct connectdata *conn, ssize_t nread);

#endif /* __SMTP_H */
