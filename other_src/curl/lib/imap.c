/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2010, Daniel Stenberg, <daniel@haxx.se>, et al.
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
 * RFC3501 IMAPv4 protocol
 * RFC5092 IMAP URL Scheme
 *
 ***************************************************************************/

#include "setup.h"

#ifndef CURL_DISABLE_IMAP
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_UTSNAME_H
#include <sys/utsname.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef __VMS
#include <in.h>
#include <inet.h>
#endif

#if (defined(NETWARE) && defined(__NOVELL_LIBC__))
#undef in_addr_t
#define in_addr_t unsigned long
#endif

#include <curl/curl.h>
#include "urldata.h"
#include "sendf.h"
#include "easyif.h" /* for Curl_convert_... prototypes */

#include "if2ip.h"
#include "hostip.h"
#include "progress.h"
#include "transfer.h"
#include "escape.h"
#include "http.h" /* for HTTP proxy tunnel stuff */
#include "socks.h"
#include "imap.h"

#include "strtoofft.h"
#include "strequal.h"
#include "sslgen.h"
#include "connect.h"
#include "strerror.h"
#include "select.h"
#include "multiif.h"
#include "url.h"
#include "rawstr.h"
#include "strtoofft.h"

#define _MPRINTF_REPLACE /* use our functions only */
#include <curl/mprintf.h>

#include "curl_memory.h"
/* The last #include file should be: */
#include "memdebug.h"

/* Local API functions */
static CURLcode imap_parse_url_path(struct connectdata *conn);
static CURLcode imap_regular_transfer(struct connectdata *conn, bool *done);
static CURLcode imap_do(struct connectdata *conn, bool *done);
static CURLcode imap_done(struct connectdata *conn,
                          CURLcode, bool premature);
static CURLcode imap_connect(struct connectdata *conn, bool *done);
static CURLcode imap_disconnect(struct connectdata *conn);
static CURLcode imap_multi_statemach(struct connectdata *conn, bool *done);
static int imap_getsock(struct connectdata *conn,
                        curl_socket_t *socks,
                        int numsocks);
static CURLcode imap_doing(struct connectdata *conn,
                           bool *dophase_done);
static CURLcode imap_setup_connection(struct connectdata * conn);

/*
 * IMAP protocol handler.
 */

const struct Curl_handler Curl_handler_imap = {
  "IMAP",                           /* scheme */
  imap_setup_connection,            /* setup_connection */
  imap_do,                          /* do_it */
  imap_done,                        /* done */
  ZERO_NULL,                        /* do_more */
  imap_connect,                     /* connect_it */
  imap_multi_statemach,             /* connecting */
  imap_doing,                       /* doing */
  imap_getsock,                     /* proto_getsock */
  imap_getsock,                     /* doing_getsock */
  ZERO_NULL,                        /* perform_getsock */
  imap_disconnect,                  /* disconnect */
  PORT_IMAP,                        /* defport */
  PROT_IMAP                         /* protocol */
};


#ifdef USE_SSL
/*
 * IMAPS protocol handler.
 */

const struct Curl_handler Curl_handler_imaps = {
  "IMAPS",                          /* scheme */
  imap_setup_connection,            /* setup_connection */
  imap_do,                          /* do_it */
  imap_done,                        /* done */
  ZERO_NULL,                        /* do_more */
  imap_connect,                     /* connect_it */
  imap_multi_statemach,             /* connecting */
  imap_doing,                       /* doing */
  imap_getsock,                     /* proto_getsock */
  imap_getsock,                     /* doing_getsock */
  ZERO_NULL,                        /* perform_getsock */
  imap_disconnect,                  /* disconnect */
  PORT_IMAPS,                       /* defport */
  PROT_IMAP | PROT_IMAPS | PROT_SSL  /* protocol */
};
#endif

#ifndef CURL_DISABLE_HTTP
/*
 * HTTP-proxyed IMAP protocol handler.
 */

static const struct Curl_handler Curl_handler_imap_proxy = {
  "IMAP",                               /* scheme */
  ZERO_NULL,                            /* setup_connection */
  Curl_http,                            /* do_it */
  Curl_http_done,                       /* done */
  ZERO_NULL,                            /* do_more */
  ZERO_NULL,                            /* connect_it */
  ZERO_NULL,                            /* connecting */
  ZERO_NULL,                            /* doing */
  ZERO_NULL,                            /* proto_getsock */
  ZERO_NULL,                            /* doing_getsock */
  ZERO_NULL,                            /* perform_getsock */
  ZERO_NULL,                            /* disconnect */
  PORT_IMAP,                            /* defport */
  PROT_HTTP                             /* protocol */
};


#ifdef USE_SSL
/*
 * HTTP-proxyed IMAPS protocol handler.
 */

static const struct Curl_handler Curl_handler_imaps_proxy = {
  "IMAPS",                              /* scheme */
  ZERO_NULL,                            /* setup_connection */
  Curl_http,                            /* do_it */
  Curl_http_done,                       /* done */
  ZERO_NULL,                            /* do_more */
  ZERO_NULL,                            /* connect_it */
  ZERO_NULL,                            /* connecting */
  ZERO_NULL,                            /* doing */
  ZERO_NULL,                            /* proto_getsock */
  ZERO_NULL,                            /* doing_getsock */
  ZERO_NULL,                            /* perform_getsock */
  ZERO_NULL,                            /* disconnect */
  PORT_IMAPS,                           /* defport */
  PROT_HTTP                             /* protocol */
};
#endif
#endif

/***********************************************************************
 *
 * imapsendf()
 *
 * Sends the formated string as an IMAP command to a server
 *
 * NOTE: we build the command in a fixed-length buffer, which sets length
 * restrictions on the command!
 *
 * Designed to never block.
 */
static CURLcode imapsendf(struct connectdata *conn,
                          const char *idstr, /* id to wait for at the
                                                completion of this command */
                          const char *fmt, ...)
{
  CURLcode res;
  struct imap_conn *imapc = &conn->proto.imapc;
  va_list ap;
  va_start(ap, fmt);

  imapc->idstr = idstr; /* this is the thing */

  res = Curl_pp_vsendf(&imapc->pp, fmt, ap);

  va_end(ap);

  return res;
}

static const char *getcmdid(struct connectdata *conn)
{
  static const char * const ids[]= {
    "A",
    "B",
    "C",
    "D"
  };

  struct imap_conn *imapc = &conn->proto.imapc;

  /* get the next id, but wrap at end of table */
  imapc->cmdid = (int)((imapc->cmdid+1) % (sizeof(ids)/sizeof(ids[0])));

  return ids[imapc->cmdid];
}

/* For the IMAP "protocol connect" and "doing" phases only */
static int imap_getsock(struct connectdata *conn,
                        curl_socket_t *socks,
                        int numsocks)
{
  return Curl_pp_getsock(&conn->proto.imapc.pp, socks, numsocks);
}

/* fucntion that checks for an imap status code at the start of the
   given string */
static int imap_endofresp(struct pingpong *pp, int *resp)
{
  char *line = pp->linestart_resp;
  size_t len = pp->nread_resp;
  struct imap_conn *imapc = &pp->conn->proto.imapc;
  const char *id = imapc->idstr;
  size_t id_len = strlen(id);

  if(len >= id_len + 3) {
    if(!memcmp(id, line, id_len) && (line[id_len] == ' ') ) {
      /* end of response */
      *resp = line[id_len+1]; /* O, N or B */
      return TRUE;
    }
    else if((imapc->state == IMAP_FETCH) &&
            !memcmp("* ", line, 2) ) {
      /* FETCH response we're interested in */
      *resp = '*';
      return TRUE;
    }
  }
  return FALSE; /* nothing for us */
}

/* This is the ONLY way to change IMAP state! */
static void state(struct connectdata *conn,
                  imapstate newstate)
{
#if defined(DEBUGBUILD) && !defined(CURL_DISABLE_VERBOSE_STRINGS)
  /* for debug purposes */
  static const char * const names[]={
    "STOP",
    "SERVERGREET",
    "LOGIN",
    "STARTTLS",
    "SELECT",
    "FETCH",
    "LOGOUT",
    /* LAST */
  };
#endif
  struct imap_conn *imapc = &conn->proto.imapc;
#if defined(DEBUGBUILD) && !defined(CURL_DISABLE_VERBOSE_STRINGS)
  if(imapc->state != newstate)
    infof(conn->data, "IMAP %p state change from %s to %s\n",
          imapc, names[imapc->state], names[newstate]);
#endif
  imapc->state = newstate;
}

static CURLcode imap_state_login(struct connectdata *conn)
{
  CURLcode result;
  struct FTP *imap = conn->data->state.proto.imap;
  const char *str;

  str = getcmdid(conn);

  /* send USER and password */
  result = imapsendf(conn, str, "%s LOGIN %s %s", str,
                     imap->user?imap->user:"",
                     imap->passwd?imap->passwd:"");
  if(result)
    return result;

  state(conn, IMAP_LOGIN);

  return CURLE_OK;
}

/* for STARTTLS responses */
static CURLcode imap_state_starttls_resp(struct connectdata *conn,
                                         int imapcode,
                                         imapstate instate)
{
  CURLcode result = CURLE_OK;
  struct SessionHandle *data = conn->data;
  (void)instate; /* no use for this yet */

  if(imapcode != 'O') {
    failf(data, "STARTTLS denied. %c", imapcode);
    result = CURLE_LOGIN_DENIED;
  }
  else {
    /* Curl_ssl_connect is BLOCKING */
    result = Curl_ssl_connect(conn, FIRSTSOCKET);
    if(CURLE_OK == result) {
      conn->protocol |= PROT_IMAPS;
      result = imap_state_login(conn);
    }
  }
  state(conn, IMAP_STOP);
  return result;
}

/* for LOGIN responses */
static CURLcode imap_state_login_resp(struct connectdata *conn,
                                      int imapcode,
                                      imapstate instate)
{
  CURLcode result = CURLE_OK;
  struct SessionHandle *data = conn->data;
  (void)instate; /* no use for this yet */

  if(imapcode != 'O') {
    failf(data, "Access denied. %c", imapcode);
    result = CURLE_LOGIN_DENIED;
  }

  state(conn, IMAP_STOP);
  return result;
}

/* for the (first line of) FETCH BODY[TEXT] response */
static CURLcode imap_state_fetch_resp(struct connectdata *conn,
                                      int imapcode,
                                      imapstate instate)
{
  CURLcode result = CURLE_OK;
  struct SessionHandle *data = conn->data;
  struct imap_conn *imapc = &conn->proto.imapc;
  struct FTP *imap = data->state.proto.imap;
  struct pingpong *pp = &imapc->pp;
  const char *ptr = data->state.buffer;
  (void)instate; /* no use for this yet */

  if('*' != imapcode) {
    Curl_pgrsSetDownloadSize(data, 0);
    state(conn, IMAP_STOP);
    return CURLE_OK;
  }

  /* Something like this comes "* 1 FETCH (BODY[TEXT] {2021}\r" */
  while(*ptr && (*ptr != '{'))
    ptr++;

  if(*ptr == '{') {
    curl_off_t filesize = curlx_strtoofft(ptr+1, NULL, 10);
    if(filesize)
      Curl_pgrsSetDownloadSize(data, filesize);

    infof(data, "Found %" FORMAT_OFF_TU " bytes to download\n", filesize);

    if(pp->cache) {
      /* At this point there is a bunch of data in the header "cache" that is
         actually body content, send it as body and then skip it. Do note
         that there may even be additional "headers" after the body. */
      size_t chunk = pp->cache_size;

      if(chunk > (size_t)filesize)
        /* the conversion from curl_off_t to size_t is always fine here */
        chunk = (size_t)filesize;

      result = Curl_client_write(conn, CLIENTWRITE_BODY, pp->cache, chunk);
      if(result)
        return result;

      filesize -= chunk;

      /* we've now used parts of or the entire cache */
      if(pp->cache_size > chunk) {
        /* part of, move the trailing data to the start and reduce the size */
        memmove(pp->cache, pp->cache+chunk,
                pp->cache_size - chunk);
        pp->cache_size -= chunk;
      }
      else {
        /* cache is drained */
        free(pp->cache);
        pp->cache = NULL;
        pp->cache_size = 0;
      }
    }

    infof(data, "Filesize left: %" FORMAT_OFF_T "\n", filesize);

    if(!filesize)
      /* the entire data is already transfered! */
      result=Curl_setup_transfer(conn, -1, -1, FALSE, NULL, -1, NULL);
    else
      /* IMAP download */
      result=Curl_setup_transfer(conn, FIRSTSOCKET, filesize, FALSE,
                                 imap->bytecountp,
                                 -1, NULL); /* no upload here */

    data->req.maxdownload = filesize;
  }
  else
    /* We don't know how to parse this line */
    result = CURLE_FTP_WEIRD_SERVER_REPLY; /* TODO: fix this code */

  state(conn, IMAP_STOP);
  return result;
}

/* start the DO phase */
static CURLcode imap_select(struct connectdata *conn)
{
  CURLcode result = CURLE_OK;
  struct imap_conn *imapc = &conn->proto.imapc;
  const char *str;

  str = getcmdid(conn);

  result = imapsendf(conn, str, "%s SELECT %s", str,
                     imapc->mailbox?imapc->mailbox:"");
  if(result)
    return result;

  state(conn, IMAP_SELECT);
  return result;
}

static CURLcode imap_fetch(struct connectdata *conn)
{
  CURLcode result = CURLE_OK;
  const char *str;

  str = getcmdid(conn);

  /* TODO: make this select the correct mail
   * Use "1 body[text]" to get the full mail body of mail 1
   */
  result = imapsendf(conn, str, "%s FETCH 1 BODY[TEXT]", str);
  if(result)
    return result;

  /*
   * When issued, the server will respond with a single line similar to
   * '* 1 FETCH (BODY[TEXT] {2021}'
   *
   * Identifying the fetch and how many bytes of contents we can expect. We
   * must extract that number before continuing to "download as usual".
   */

  state(conn, IMAP_FETCH);
  return result;
}

/* for SELECT responses */
static CURLcode imap_state_select_resp(struct connectdata *conn,
                                       int imapcode,
                                       imapstate instate)
{
  CURLcode result = CURLE_OK;
  struct SessionHandle *data = conn->data;
  (void)instate; /* no use for this yet */

  if(imapcode != 'O') {
    failf(data, "Select failed");
    result = CURLE_LOGIN_DENIED;
  }
  else
    result = imap_fetch(conn);
  return result;
}

static CURLcode imap_statemach_act(struct connectdata *conn)
{
  CURLcode result;
  curl_socket_t sock = conn->sock[FIRSTSOCKET];
  struct SessionHandle *data=conn->data;
  int imapcode;
  struct imap_conn *imapc = &conn->proto.imapc;
  struct pingpong *pp = &imapc->pp;
  size_t nread = 0;

  if(pp->sendleft)
    return Curl_pp_flushsend(pp);

  /* we read a piece of response */
  result = Curl_pp_readresp(sock, pp, &imapcode, &nread);
  if(result)
    return result;

  if(imapcode)
  /* we have now received a full IMAP server response */
  switch(imapc->state) {
  case IMAP_SERVERGREET:
    if(imapcode != 'O') {
      failf(data, "Got unexpected imap-server response");
      return CURLE_FTP_WEIRD_SERVER_REPLY;
    }

    if(data->set.ftp_ssl && !conn->ssl[FIRSTSOCKET].use) {
      /* We don't have a SSL/TLS connection yet, but SSL is requested. Switch
         to TLS connection now */
      const char *str;

      str = getcmdid(conn);
      result = imapsendf(conn, str, "%s STARTTLS", str);
      state(conn, IMAP_STARTTLS);
    }
    else
      result = imap_state_login(conn);
    if(result)
      return result;
    break;

  case IMAP_LOGIN:
    result = imap_state_login_resp(conn, imapcode, imapc->state);
    break;

  case IMAP_STARTTLS:
    result = imap_state_starttls_resp(conn, imapcode, imapc->state);
    break;

  case IMAP_FETCH:
    result = imap_state_fetch_resp(conn, imapcode, imapc->state);
    break;

  case IMAP_SELECT:
    result = imap_state_select_resp(conn, imapcode, imapc->state);
    break;

  case IMAP_LOGOUT:
    /* fallthrough, just stop! */
  default:
    /* internal error */
    state(conn, IMAP_STOP);
    break;
  }

  return result;
}

/* called repeatedly until done from multi.c */
static CURLcode imap_multi_statemach(struct connectdata *conn,
                                         bool *done)
{
  struct imap_conn *imapc = &conn->proto.imapc;
  CURLcode result = Curl_pp_multi_statemach(&imapc->pp);

  *done = (bool)(imapc->state == IMAP_STOP);

  return result;
}

static CURLcode imap_easy_statemach(struct connectdata *conn)
{
  struct imap_conn *imapc = &conn->proto.imapc;
  struct pingpong *pp = &imapc->pp;
  CURLcode result = CURLE_OK;

  while(imapc->state != IMAP_STOP) {
    result = Curl_pp_easy_statemach(pp);
    if(result)
      break;
  }

  return result;
}

/*
 * Allocate and initialize the struct IMAP for the current SessionHandle.  If
 * need be.
 */
static CURLcode imap_init(struct connectdata *conn)
{
  struct SessionHandle *data = conn->data;
  struct FTP *imap = data->state.proto.imap;
  if(!imap) {
    imap = data->state.proto.imap = calloc(sizeof(struct FTP), 1);
    if(!imap)
      return CURLE_OUT_OF_MEMORY;
  }

  /* get some initial data into the imap struct */
  imap->bytecountp = &data->req.bytecount;

  /* No need to duplicate user+password, the connectdata struct won't change
     during a session, but we re-init them here since on subsequent inits
     since the conn struct may have changed or been replaced.
  */
  imap->user = conn->user;
  imap->passwd = conn->passwd;

  return CURLE_OK;
}

/*
 * imap_connect() should do everything that is to be considered a part of
 * the connection phase.
 *
 * The variable 'done' points to will be TRUE if the protocol-layer connect
 * phase is done when this function returns, or FALSE is not. When called as
 * a part of the easy interface, it will always be TRUE.
 */
static CURLcode imap_connect(struct connectdata *conn,
                                 bool *done) /* see description above */
{
  CURLcode result;
  struct imap_conn *imapc = &conn->proto.imapc;
  struct SessionHandle *data=conn->data;
  struct pingpong *pp = &imapc->pp;

  *done = FALSE; /* default to not done yet */

  /* If there already is a protocol-specific struct allocated for this
     sessionhandle, deal with it */
  Curl_reset_reqproto(conn);

  result = imap_init(conn);
  if(CURLE_OK != result)
    return result;

  /* We always support persistant connections on imap */
  conn->bits.close = FALSE;

  pp->response_time = RESP_TIMEOUT; /* set default response time-out */
  pp->statemach_act = imap_statemach_act;
  pp->endofresp = imap_endofresp;
  pp->conn = conn;

#if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_PROXY)
  if(conn->bits.tunnel_proxy && conn->bits.httpproxy) {
    /* for IMAP over HTTP proxy */
    struct HTTP http_proxy;
    struct FTP *imap_save;

    /* BLOCKING */
    /* We want "seamless" IMAP operations through HTTP proxy tunnel */

    /* Curl_proxyCONNECT is based on a pointer to a struct HTTP at the member
     * conn->proto.http; we want IMAP through HTTP and we have to change the
     * member temporarily for connecting to the HTTP proxy. After
     * Curl_proxyCONNECT we have to set back the member to the original struct
     * IMAP pointer
     */
    imap_save = data->state.proto.imap;
    memset(&http_proxy, 0, sizeof(http_proxy));
    data->state.proto.http = &http_proxy;

    result = Curl_proxyCONNECT(conn, FIRSTSOCKET,
                               conn->host.name, conn->remote_port);

    data->state.proto.imap = imap_save;

    if(CURLE_OK != result)
      return result;
  }
#endif /* !CURL_DISABLE_HTTP && !CURL_DISABLE_PROXY */

  if(conn->protocol & PROT_IMAPS) {
    /* BLOCKING */
    /* IMAPS is simply imap with SSL for the control channel */
    /* now, perform the SSL initialization for this socket */
    result = Curl_ssl_connect(conn, FIRSTSOCKET);
    if(result)
      return result;
  }

  Curl_pp_init(pp); /* init generic pingpong data */

  /* When we connect, we start in the state where we await the server greeting
     response */
  state(conn, IMAP_SERVERGREET);
  imapc->idstr = "*"; /* we start off waiting for a '*' response */

  if(data->state.used_interface == Curl_if_multi)
    result = imap_multi_statemach(conn, done);
  else {
    result = imap_easy_statemach(conn);
    if(!result)
      *done = TRUE;
  }

  return result;
}

/***********************************************************************
 *
 * imap_done()
 *
 * The DONE function. This does what needs to be done after a single DO has
 * performed.
 *
 * Input argument is already checked for validity.
 */
static CURLcode imap_done(struct connectdata *conn, CURLcode status,
                          bool premature)
{
  struct SessionHandle *data = conn->data;
  struct FTP *imap = data->state.proto.imap;
  CURLcode result=CURLE_OK;
  (void)premature;

  if(!imap)
    /* When the easy handle is removed from the multi while libcurl is still
     * trying to resolve the host name, it seems that the imap struct is not
     * yet initialized, but the removal action calls Curl_done() which calls
     * this function. So we simply return success if no imap pointer is set.
     */
    return CURLE_OK;

  if(status) {
    conn->bits.close = TRUE; /* marked for closure */
    result = status;      /* use the already set error code */
  }

  /* clear these for next connection */
  imap->transfer = FTPTRANSFER_BODY;

  return result;
}

/***********************************************************************
 *
 * imap_perform()
 *
 * This is the actual DO function for IMAP. Get a file/directory according to
 * the options previously setup.
 */

static
CURLcode imap_perform(struct connectdata *conn,
                     bool *connected,  /* connect status after PASV / PORT */
                     bool *dophase_done)
{
  /* this is IMAP and no proxy */
  CURLcode result=CURLE_OK;

  DEBUGF(infof(conn->data, "DO phase starts\n"));

  if(conn->data->set.opt_no_body) {
    /* requested no body means no transfer... */
    struct FTP *imap = conn->data->state.proto.imap;
    imap->transfer = FTPTRANSFER_INFO;
  }

  *dophase_done = FALSE; /* not done yet */

  /* start the first command in the DO phase */
  result = imap_select(conn);
  if(result)
    return result;

  /* run the state-machine */
  if(conn->data->state.used_interface == Curl_if_multi)
    result = imap_multi_statemach(conn, dophase_done);
  else {
    result = imap_easy_statemach(conn);
    *dophase_done = TRUE; /* with the easy interface we are done here */
  }
  *connected = conn->bits.tcpconnect;

  if(*dophase_done)
    DEBUGF(infof(conn->data, "DO phase is complete\n"));

  return result;
}

/***********************************************************************
 *
 * imap_do()
 *
 * This function is registered as 'curl_do' function. It decodes the path
 * parts etc as a wrapper to the actual DO function (imap_perform).
 *
 * The input argument is already checked for validity.
 */
static CURLcode imap_do(struct connectdata *conn, bool *done)
{
  CURLcode retcode = CURLE_OK;

  *done = FALSE; /* default to false */

  /*
    Since connections can be re-used between SessionHandles, this might be a
    connection already existing but on a fresh SessionHandle struct so we must
    make sure we have a good 'struct IMAP' to play with. For new connections,
    the struct IMAP is allocated and setup in the imap_connect() function.
  */
  Curl_reset_reqproto(conn);
  retcode = imap_init(conn);
  if(retcode)
    return retcode;

  retcode = imap_parse_url_path(conn);
  if(retcode)
    return retcode;

  retcode = imap_regular_transfer(conn, done);

  return retcode;
}

/***********************************************************************
 *
 * imap_logout()
 *
 * This should be called before calling sclose().  We should then wait for the
 * response from the server before returning. The calling code should then try
 * to close the connection.
 *
 */
static CURLcode imap_logout(struct connectdata *conn)
{
  CURLcode result = CURLE_OK;
  const char *str;

  str = getcmdid(conn);

  result = imapsendf(conn, str, "%s LOGOUT", str, NULL);
  if(result)
    return result;
  state(conn, IMAP_LOGOUT);

  result = imap_easy_statemach(conn);

  return result;
}

/***********************************************************************
 *
 * imap_disconnect()
 *
 * Disconnect from an IMAP server. Cleanup protocol-specific per-connection
 * resources. BLOCKING.
 */
static CURLcode imap_disconnect(struct connectdata *conn)
{
  struct imap_conn *imapc= &conn->proto.imapc;

  /* The IMAP session may or may not have been allocated/setup at this
     point! */
  if (imapc->pp.conn)
    (void)imap_logout(conn); /* ignore errors on the LOGOUT */

  Curl_pp_disconnect(&imapc->pp);

  Curl_safefree(imapc->mailbox);

  return CURLE_OK;
}

/***********************************************************************
 *
 * imap_parse_url_path()
 *
 * Parse the URL path into separate path components.
 *
 */
static CURLcode imap_parse_url_path(struct connectdata *conn)
{
  /* the imap struct is already inited in imap_connect() */
  struct imap_conn *imapc = &conn->proto.imapc;
  struct SessionHandle *data = conn->data;
  const char *path = data->state.path;
  int len;

  if(!*path)
    path = "INBOX";

  /* url decode the path and use this mailbox */
  imapc->mailbox = curl_easy_unescape(data, path, 0, &len);
  if(!imapc->mailbox)
    return CURLE_OUT_OF_MEMORY;

  return CURLE_OK;
}

/* call this when the DO phase has completed */
static CURLcode imap_dophase_done(struct connectdata *conn,
                                  bool connected)
{
  CURLcode result = CURLE_OK;
  struct FTP *imap = conn->data->state.proto.imap;
  (void)connected;

  if(imap->transfer != FTPTRANSFER_BODY)
    /* no data to transfer */
    result=Curl_setup_transfer(conn, -1, -1, FALSE, NULL, -1, NULL);

  return result;
}

/* called from multi.c while DOing */
static CURLcode imap_doing(struct connectdata *conn,
                               bool *dophase_done)
{
  CURLcode result;
  result = imap_multi_statemach(conn, dophase_done);

  if(*dophase_done) {
    result = imap_dophase_done(conn, FALSE /* not connected */);

    DEBUGF(infof(conn->data, "DO phase is complete\n"));
  }
  return result;
}

/***********************************************************************
 *
 * imap_regular_transfer()
 *
 * The input argument is already checked for validity.
 *
 * Performs all commands done before a regular transfer between a local and a
 * remote host.
 *
 */
static
CURLcode imap_regular_transfer(struct connectdata *conn,
                              bool *dophase_done)
{
  CURLcode result=CURLE_OK;
  bool connected=FALSE;
  struct SessionHandle *data = conn->data;
  data->req.size = -1; /* make sure this is unknown at this point */

  Curl_pgrsSetUploadCounter(data, 0);
  Curl_pgrsSetDownloadCounter(data, 0);
  Curl_pgrsSetUploadSize(data, 0);
  Curl_pgrsSetDownloadSize(data, 0);

  result = imap_perform(conn,
                        &connected, /* have we connected after PASV/PORT */
                        dophase_done); /* all commands in the DO-phase done? */

  if(CURLE_OK == result) {

    if(!*dophase_done)
      /* the DO phase has not completed yet */
      return CURLE_OK;

    result = imap_dophase_done(conn, connected);
    if(result)
      return result;
  }

  return result;
}

static CURLcode imap_setup_connection(struct connectdata * conn)
{
  struct SessionHandle *data = conn->data;

  if(conn->bits.httpproxy && !data->set.tunnel_thru_httpproxy) {
    /* Unless we have asked to tunnel imap operations through the proxy, we
       switch and use HTTP operations only */
#ifndef CURL_DISABLE_HTTP
    if(conn->handler == &Curl_handler_imap)
      conn->handler = &Curl_handler_imap_proxy;
    else {
#ifdef USE_SSL
      conn->handler = &Curl_handler_imaps_proxy;
#else
      failf(data, "IMAPS not supported!");
      return CURLE_UNSUPPORTED_PROTOCOL;
#endif
    }
    /*
     * We explicitly mark this connection as persistent here as we're doing
     * IMAP over HTTP and thus we accidentally avoid setting this value
     * otherwise.
     */
    conn->bits.close = FALSE;
#else
    failf(data, "IMAP over http proxy requires HTTP support built-in!");
    return CURLE_UNSUPPORTED_PROTOCOL;
#endif
  }

  data->state.path++;   /* don't include the initial slash */

  return CURLE_OK;
}

#endif /* CURL_DISABLE_IMAP */
