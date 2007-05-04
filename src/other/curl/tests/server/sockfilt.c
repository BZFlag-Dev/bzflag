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
 * $Id: sockfilt.c,v 1.29 2007-02-19 02:04:01 yangtse Exp $
 ***************************************************************************/

/* Purpose
 *
 * 1. Accept a TCP connection on a custom port (ipv4 or ipv6), or connect
 *    to a given (localhost) port.
 *
 * 2. Get commands on STDIN. Pass data on to the TCP stream.
 *    Get data from TCP stream and pass on to STDOUT.
 *
 * This program is made to perform all the socket/stream/connection stuff for
 * the test suite's (perl) FTP server. Previously the perl code did all of
 * this by its own, but I decided to let this program do the socket layer
 * because of several things:
 *
 * o We want the perl code to work with rather old perl installations, thus
 *   we cannot use recent perl modules or features.
 *
 * o We want IPv6 support for systems that provide it, and doing optional IPv6
 *   support in perl seems if not impossible so at least awkward.
 *
 * o We want FTP-SSL support, which means that a connection that starts with
 *   plain sockets needs to be able to "go SSL" in the midst. This would also
 *   require some nasty perl stuff I'd rather avoid.
 *
 * (Source originally based on sws.c)
 */
#include "setup.h" /* portability help from the lib directory */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef _XOPEN_SOURCE_EXTENDED
/* This define is "almost" required to build on HPUX 11 */
#include <arpa/inet.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#define ENABLE_CURLX_PRINTF
/* make the curlx header define all printf() functions to use the curlx_*
   versions instead */
#include "curlx.h" /* from the private lib dir */
#include "getpart.h"
#include "inet_pton.h"
#include "util.h"

/* include memdebug.h last */
#include "memdebug.h"

#define DEFAULT_PORT 8999

#ifndef DEFAULT_LOGFILE
#define DEFAULT_LOGFILE "log/sockfilt.log"
#endif

#ifdef SIGPIPE
static volatile int sigpipe;  /* Why? It's not used */
#endif

const char *serverlogfile = (char *)DEFAULT_LOGFILE;

static void lograw(unsigned char *buffer, ssize_t len)
{
  char data[120];
  ssize_t i;
  unsigned char *ptr = buffer;
  char *optr = data;
  ssize_t width=0;

  for(i=0; i<len; i++) {
    switch(ptr[i]) {
    case '\n':
      sprintf(optr, "\\n");
      width += 2;
      optr += 2;
      break;
    case '\r':
      sprintf(optr, "\\r");
      width += 2;
      optr += 2;
      break;
    default:
      sprintf(optr, "%c", (ISGRAPH(ptr[i]) || ptr[i]==0x20) ?ptr[i]:'.');
      width++;
      optr++;
      break;
    }

    if(width>60) {
      logmsg("'%s'", data);
      width = 0;
      optr = data;
    }
  }
  if(width)
    logmsg("'%s'", data);
}

#ifdef SIGPIPE
static void sigpipe_handler(int sig)
{
  (void)sig; /* prevent warning */
  sigpipe = 1;
}
#endif

char use_ipv6=FALSE;
unsigned short port = DEFAULT_PORT;
unsigned short connectport = 0; /* if non-zero, we activate this mode */

enum sockmode {
  PASSIVE_LISTEN,    /* as a server waiting for connections */
  PASSIVE_CONNECT,   /* as a server, connected to a client */
  ACTIVE,            /* as a client, connected to a server */
  ACTIVE_DISCONNECT  /* as a client, disconnected from server */
};

/*
  sockfdp is a pointer to an established stream or CURL_SOCKET_BAD

  if sockfd is CURL_SOCKET_BAD, listendfd is a listening socket we must
  accept()
*/
static int juggle(curl_socket_t *sockfdp,
                  curl_socket_t listenfd,
                  enum sockmode *mode)
{
  struct timeval timeout;
  fd_set fds_read;
  fd_set fds_write;
  fd_set fds_err;
  curl_socket_t sockfd;
  curl_socket_t maxfd;
  ssize_t rc;
  ssize_t nread_stdin;
  ssize_t nread_socket;
  ssize_t bytes_written;
  ssize_t buffer_len;
  unsigned char buffer[256]; /* FIX: bigger buffer */
  char data[256];

  timeout.tv_sec = 120;
  timeout.tv_usec = 0;

  FD_ZERO(&fds_read);
  FD_ZERO(&fds_write);
  FD_ZERO(&fds_err);

  FD_SET(fileno(stdin), &fds_read);

  switch(*mode) {

  case PASSIVE_LISTEN:

    /* server mode */
    sockfd = listenfd;
    /* there's always a socket to wait for */
    FD_SET(sockfd, &fds_read);
    maxfd = sockfd;
    break;

  case PASSIVE_CONNECT:

    sockfd = *sockfdp;
    if(CURL_SOCKET_BAD == sockfd) {
      /* eeek, we are supposedly connected and then this cannot be -1 ! */
      logmsg("socket is -1! on %s:%d", __FILE__, __LINE__);
      maxfd = 0; /* stdin */
    }
    else {
      /* there's always a socket to wait for */
      FD_SET(sockfd, &fds_read);
      maxfd = sockfd;
    }
    break;

  case ACTIVE:

    sockfd = *sockfdp;
    /* sockfd turns CURL_SOCKET_BAD when our connection has been closed */
    if(CURL_SOCKET_BAD != sockfd) {
      FD_SET(sockfd, &fds_read);
      maxfd = sockfd;
    }
    else {
      logmsg("No socket to read on");
      maxfd = 0;
    }
    break;

  case ACTIVE_DISCONNECT:

    logmsg("disconnected, no socket to read on");
    maxfd = 0;
    sockfd = CURL_SOCKET_BAD;
    break;

  } /* switch(*mode) */

  do {
    rc = select(maxfd + 1, &fds_read, &fds_write, &fds_err, &timeout);
  } while((rc == -1) && (SOCKERRNO == EINTR));

  switch(rc) {
  case -1:
    return FALSE;

  case 0: /* timeout! */
    return TRUE;
  }


  if(FD_ISSET(fileno(stdin), &fds_read)) {
    /* read from stdin, commands/data to be dealt with and possibly passed on
       to the socket

       protocol:

       4 letter command + LF [mandatory]

       4-digit hexadecimal data length + LF [if the command takes data]
       data                       [the data being as long as set above]

       Commands:

       DATA - plain pass-thru data
    */
    nread_stdin = read(fileno(stdin), buffer, 5);
    if(5 == nread_stdin) {

      logmsg("Received %c%c%c%c (on stdin)",
             buffer[0], buffer[1], buffer[2], buffer[3] );

      if(!memcmp("PING", buffer, 4)) {
        /* send reply on stdout, just proving we are alive */
        write(fileno(stdout), "PONG\n", 5);
      }

      else if(!memcmp("PORT", buffer, 4)) {
        /* Question asking us what PORT number we are listening to.
           Replies to PORT with "IPv[num]/[port]" */
        sprintf((char *)buffer, "IPv%d/%d\n", use_ipv6?6:4, (int)port);
        buffer_len = (ssize_t)strlen((char *)buffer);
        sprintf(data, "PORT\n%04x\n", buffer_len);
        write(fileno(stdout), data, 10);
        write(fileno(stdout), buffer, buffer_len);
      }
      else if(!memcmp("QUIT", buffer, 4)) {
        /* just die */
        logmsg("quits");
        return FALSE;
      }
      else if(!memcmp("DATA", buffer, 4)) {
        /* data IN => data OUT */

        if(5 != read(fileno(stdin), buffer, 5))
          return FALSE;
        buffer[5] = '\0';

        buffer_len = (ssize_t)strtol((char *)buffer, NULL, 16);
        if (buffer_len > (ssize_t)sizeof(buffer)) {
          logmsg("Buffer size %d too small for data size %d", 
                   (int)sizeof(buffer), buffer_len);
          return FALSE;
        }
        nread_stdin = read(fileno(stdin), buffer, buffer_len);
        if(nread_stdin != buffer_len)
          return FALSE;

        logmsg("> %d bytes data, server => client", buffer_len);
        lograw(buffer, buffer_len);

        if(*mode == PASSIVE_LISTEN) {
          logmsg("*** We are disconnected!");
          write(fileno(stdout), "DISC\n", 5);
        }
        else {
          /* send away on the socket */
          bytes_written = swrite(sockfd, buffer, buffer_len);
          if(bytes_written != buffer_len) {
            logmsg("Not all data was sent. Bytes to send: %d sent: %d", 
                   buffer_len, bytes_written);
          }
        }
      }
      else if(!memcmp("DISC", buffer, 4)) {
        /* disconnect! */
        write(fileno(stdout), "DISC\n", 5);
        if(sockfd != CURL_SOCKET_BAD) {
          logmsg("====> Client forcibly disconnected");
          sclose(sockfd);
          *sockfdp = CURL_SOCKET_BAD;
          if(*mode == PASSIVE_CONNECT)
            *mode = PASSIVE_LISTEN;
          else
            *mode = ACTIVE_DISCONNECT;
        }
        else
          logmsg("attempt to close already dead connection");
        return TRUE;
      }
    }
    else if(-1 == nread_stdin) {
      logmsg("read %d from stdin, exiting", nread_stdin);
      return FALSE;
    }
  }


  if((sockfd != CURL_SOCKET_BAD) && (FD_ISSET(sockfd, &fds_read)) ) {

    if(*mode == PASSIVE_LISTEN) {
      /* there's no stream set up yet, this is an indication that there's a
         client connecting. */
      sockfd = accept(sockfd, NULL, NULL);
      if(CURL_SOCKET_BAD == sockfd)
        logmsg("accept() failed");
      else {
        logmsg("====> Client connect");
        write(fileno(stdout), "CNCT\n", 5);
        *sockfdp = sockfd; /* store the new socket */
        *mode = PASSIVE_CONNECT; /* we have connected */
      }
      return TRUE;
    }

    /* read from socket, pass on data to stdout */
    nread_socket = sread(sockfd, buffer, sizeof(buffer));

    if(nread_socket <= 0) {
      logmsg("====> Client disconnect");
      write(fileno(stdout), "DISC\n", 5);
      sclose(sockfd);
      *sockfdp = CURL_SOCKET_BAD;
      if(*mode == PASSIVE_CONNECT)
        *mode = PASSIVE_LISTEN;
      else
        *mode = ACTIVE_DISCONNECT;
      return TRUE;
    }

    sprintf(data, "DATA\n%04x\n", nread_socket);
    write(fileno(stdout), data, 10);
    write(fileno(stdout), buffer, nread_socket);

    logmsg("< %d bytes data, client => server", nread_socket);
    lograw(buffer, nread_socket);
  }

  return TRUE;
}

static curl_socket_t sockdaemon(curl_socket_t sock,
                                unsigned short *port)
{
  /* passive daemon style */
  struct sockaddr_in me;
#ifdef ENABLE_IPV6
  struct sockaddr_in6 me6;
#endif /* ENABLE_IPV6 */
  int flag = 1;
  int rc;

  if (setsockopt
      (sock, SOL_SOCKET, SO_REUSEADDR, (void *)&flag,
       sizeof(flag)) < 0) {
    perror("setsockopt(SO_REUSEADDR)");
  }

#ifdef ENABLE_IPV6
  if(!use_ipv6) {
#endif
    me.sin_family = AF_INET;
    me.sin_addr.s_addr = INADDR_ANY;
    me.sin_port = htons(*port);
    rc = bind(sock, (struct sockaddr *) &me, sizeof(me));
#ifdef ENABLE_IPV6
  }
  else {
    memset(&me6, 0, sizeof(struct sockaddr_in6));
    me6.sin6_family = AF_INET6;
    me6.sin6_addr = in6addr_any;
    me6.sin6_port = htons(*port);
    rc = bind(sock, (struct sockaddr *) &me6, sizeof(me6));
  }
#endif /* ENABLE_IPV6 */
  if(rc < 0) {
    perror("binding stream socket");
    logmsg("Error binding socket");
    return CURL_SOCKET_BAD;
  }

  if(!*port) {
    /* The system picked a port number, now figure out which port we actually
       got */
    /* we succeeded to bind */
    struct sockaddr_in add;
    socklen_t socksize = sizeof(add);

    if(getsockname(sock, (struct sockaddr *) &add,
                   &socksize)<0) {
      logmsg("getsockname() failed with error: %d", SOCKERRNO);
      return CURL_SOCKET_BAD;
    }
    *port = ntohs(add.sin_port);
  }

  /* start accepting connections */
  rc = listen(sock, 4);
  if(0 != rc) {
    logmsg("listen() failed with error: %d", SOCKERRNO);
    sclose(sock);
    return CURL_SOCKET_BAD;
  }

  return sock;
}

static curl_socket_t mksock(bool use_ipv6)
{
  curl_socket_t sock;
#ifdef ENABLE_IPV6
  if(!use_ipv6)
#else
    (void)use_ipv6;
#endif
  sock = socket(AF_INET, SOCK_STREAM, 0);
#ifdef ENABLE_IPV6
  else
    sock = socket(AF_INET6, SOCK_STREAM, 0);
#endif

  if (CURL_SOCKET_BAD == sock) {
    perror("opening stream socket");
    logmsg("Error opening socket");
  }

  return sock;
}


int main(int argc, char *argv[])
{
  struct sockaddr_in me;
#ifdef ENABLE_IPV6
  struct sockaddr_in6 me6;
#endif /* ENABLE_IPV6 */
  curl_socket_t sock;
  curl_socket_t msgsock;
  FILE *pidfile;
  char *pidname= (char *)".sockfilt.pid";
  int rc;
  int error;
  int arg=1;
  enum sockmode mode = PASSIVE_LISTEN; /* default */

  while(argc>arg) {
    if(!strcmp("--version", argv[arg])) {
      printf("sockfilt IPv4%s\n",
#ifdef ENABLE_IPV6
             "/IPv6"
#else
             ""
#endif
             );
      return 0;
    }
    else if(!strcmp("--pidfile", argv[arg])) {
      arg++;
      if(argc>arg)
        pidname = argv[arg++];
    }
    else if(!strcmp("--logfile", argv[arg])) {
      arg++;
      if(argc>arg)
        serverlogfile = argv[arg++];
    }
    else if(!strcmp("--ipv6", argv[arg])) {
#ifdef ENABLE_IPV6
      use_ipv6=TRUE;
#endif
      arg++;
    }
    else if(!strcmp("--ipv4", argv[arg])) {
      /* for completeness, we support this option as well */
      use_ipv6=FALSE;
      arg++;
    }
    else if(!strcmp("--port", argv[arg])) {
      arg++;
      if(argc>arg) {
        port = (unsigned short)atoi(argv[arg]);
        arg++;
      }
    }
    else if(!strcmp("--connect", argv[arg])) {
      /* Asked to actively connect to the specified local port instead of
         doing a passive server-style listening. */
      arg++;
      if(argc>arg) {
        connectport = (unsigned short)atoi(argv[arg]);
        arg++;
      }
    }
    else {
      puts("Usage: sockfilt [option]\n"
           " --version\n"
           " --logfile [file]\n"
           " --pidfile [file]\n"
           " --ipv4\n"
           " --ipv6\n"
           " --port [port]");
      return 0;
    }
  }

#ifdef WIN32
  win32_init();
  atexit(win32_cleanup);
#else

#ifdef SIGPIPE
#ifdef HAVE_SIGNAL
  signal(SIGPIPE, sigpipe_handler);
#endif
#ifdef HAVE_SIGINTERRUPT
  siginterrupt(SIGPIPE, 1);
#endif
#endif
#endif


  sock = mksock(use_ipv6);
  if (CURL_SOCKET_BAD == sock) {
    logmsg("Error opening socket: %d", SOCKERRNO);
    return 1;
  }

  if(connectport) {
    /* Active mode, we should connect to the given port number */
    mode = ACTIVE;
#ifdef ENABLE_IPV6
    if(!use_ipv6) {
#endif
      memset(&me, 0, sizeof(me));
      me.sin_family = AF_INET;
      me.sin_port = htons(connectport);
      me.sin_addr.s_addr = INADDR_ANY;
      Curl_inet_pton(AF_INET, "127.0.0.1", &me.sin_addr);

      rc = connect(sock, (struct sockaddr *) &me, sizeof(me));
#ifdef ENABLE_IPV6
    }
    else {
      memset(&me6, 0, sizeof(me6));
      me6.sin6_family = AF_INET6;
      me6.sin6_port = htons(connectport);
      Curl_inet_pton(AF_INET6, "::1", &me6.sin6_addr);

      rc = connect(sock, (struct sockaddr *) &me6, sizeof(me6));
    }
#endif /* ENABLE_IPV6 */
    if(rc) {
      perror("connecting stream socket");
      logmsg("Error connecting to port %d", port);
      sclose(sock);
      return 1;
    }
    logmsg("====> Client connect");
    msgsock = sock; /* use this as stream */
  }
  else {
    /* passive daemon style */
    sock = sockdaemon(sock, &port);
    if(CURL_SOCKET_BAD == sock)
      return 1;
    msgsock = CURL_SOCKET_BAD; /* no stream socket yet */
  }

  logmsg("Running IPv%d version",
         (use_ipv6?6:4));

  if(connectport)
    logmsg("Connected to port %d", connectport);
  else
    logmsg("Listening on port %d", port);

  pidfile = fopen(pidname, "w");
  if(pidfile) {
    int pid = (int)getpid();
    fprintf(pidfile, "%d\n", pid);
    fclose(pidfile);
    logmsg("Wrote pid %d to %s", pid, pidname);
  }
  else {
    error = ERRNO;
    logmsg("fopen() failed with error: %d %s\n", error, strerror(error));
    logmsg("Error opening file: %s\n", pidname);
    logmsg("Couldn't write pid file\n");
    sclose(sock);
    return 1;
  }

  while(juggle(&msgsock, sock, &mode));

  sclose(sock);

  return 0;
}

