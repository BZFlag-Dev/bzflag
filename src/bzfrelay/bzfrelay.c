/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * bzfrelay -- firewall tunnel for bzflag servers
 *
 * note that this program does not depend on any other bzflag files.
 */

/* use <> instead of "" cause autoconf recommends it */
#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#if defined(__sgi)
#include <bstring.h>
#endif
/* This happens on suns */
#ifndef FIONBIO
#include <sys/filio.h>
#endif

#if defined(sun)
#define CNCTType		struct sockaddr
#define RecvType		char*
#define SendType		const char*
#endif

#if defined(__BEOS__)
#define socklen_t int
#define O_NDELAY O_NONBLOCK
#endif

#if !defined(INADDR_NONE)
#define INADDR_NONE		((in_addr_t)0xffffffff)
#endif
#if !defined(CNCTType)
#define CNCTType		const struct sockaddr
#endif
#if !defined(RecvType)
#define RecvType		void*
#endif
#if !defined(SendType)
#define SendType		const void*
#endif

#define	RELAY_BUFLEN		4096		/* size of relay buffers */
#define BZFS_PORT		"5154"		/* well known bzfs port */
#define RECONNECT_PORT		"5157"		/* listen for reconnect here */
#define NETWORK_TIMEOUT		15		/* seconds until give up */

#define	DL_ALERT		0		/* always report */
#define DL_IMPORTANT		1		/* major problems */
#define DL_NOTICE		2		/* logging; minor problems */
#define DL_INFO			3		/* extra logging */
#define DL_DEBUG		4		/* debugging */
#define MAX_DEBUG_LEVEL		5		/* ignore higher debug levels */
#define BZFR_LOG_FACILITY	LOG_USER

enum			{ BZFR_LOG_INFO, BZFR_LOG_WARN, BZFR_LOG_ERR };
static const char*	priorityString[] = { "INFO", "WARNING", "ERROR" };

enum	{ DISABLED,		/* shutdown */
	  CONNECTING,		/* first connect() to server in progress */
	  FIRST_CONTACT,	/* waiting for server hello */
	  RECONNECTING,		/* second connect() to server in progress */
	  AWAITING_RECONNECT,	/* waiting for reconnection from client */
	  RELAYING,		/* relaying client<->server */
	  CLOSING };		/* waiting for client to hangup */

typedef struct Relay {
    /* doubly linked list */
    struct Relay*	prev;
    struct Relay*	next;

    /* relay info */
    int			status;
    int			fdSrc;			/* client side socket */
    int			fdDst;			/* server side socket */
    int			fdOldSrc;		/* old client side socket */
    int			fdOldDst;		/* old server side socket */
    unsigned char*	srcToDstBuffer, *srcMark;
    unsigned char*	dstToSrcBuffer, *dstMark;
    int			srcToDstFilled;
    int			dstToSrcFilled;

    /* for timeout */
    struct timeval	startTime;

    /* info for log */
    struct in_addr	srcAddr;		/* client's address */
    char*		srcName;		/* client's name/address */
    int			srcToDstBytes;		/* total traffic */
    int			dstToSrcBytes;		/* total traffic */
} Relay;

typedef struct AddressSet {
    struct AddressSet*	next;
    int			allow;
    struct in_addr	addr;
    struct in_addr	mask;
} AddressSet;

/* srosa/SGI -- have no idea where socklen_t comes from. */
#if defined(sgi)
#define socklen_t int
#endif

static const char*	pname;			/* name of app */
static int		done;			/* != 0 to quit */
static int		fdMax;			/* highest file descriptor */
static int		fdListen;		/* listen for connections */
static int		fdReconnect;		/* listen for reconnections */
static int		usingSyslog;		/* != 0 to use syslog */
static Relay*		relays;			/* doubly-linked list of relays */

static const char	copyright[] = "Copyright (c) 1993 - 2004 Tim Riker";

/*
 * debugging, logging, help
 */

static const char*	usageString = "usage: %s [-f] [-s <address[:port]>] "
				"[-p <reconnect-port>] "
				"[-a <addr> <mask>] "
				"[-r <addr> <mask>] <server-addr[:port]>\n";
static const char*	helpString =
"\t-f:                run in foreground, log to stderr\n"
"\t                   (otherwise background, log to syslog)\n"
"\t-s:                listen on given address and port instead\n"
"\t                   of port " BZFS_PORT " on all interfaces\n"
"\t-p:                listen for reconnections on given port\n"
"\t                   instead of port " RECONNECT_PORT "\n"
"\t-a:                allow clients at given addresses\n"
"\t-r:                reject clients at given addresses\n"
"\tserver-addr:port:  address (and port) of bzfs server\n"
"\n"
"The -s option specifies the address to listen for clients on.  The\n"
"address portion is optional; if omitted, the program listens on the\n"
"given port (which must include the colon prefix) on all interfaces.  If\n"
"the address is supplied, the program listens on only the given\n"
"interface;  the address must specify a local interface address.  If the\n"
"address is supplied, the port is optional;  if no port is given, the\n"
"default port " BZFS_PORT " is used.  The default is to listen on port\n"
BZFS_PORT " on all interfaces.\n"
"\n"
"The -p option specifies an alternative port to listen on for reconnects.\n"
"The default port is " RECONNECT_PORT ".  Both this port and the port\n"
"given by the -s option (or " BZFS_PORT " if not supplied) must be\n"
"accessible to external hosts.  Only reconnects on the same interface\n"
"as the initial connection are accepted.\n"
"\n"
"The -a and -r options define the allowed client addresses:  -a adds a\n"
"set of addresses to the list of allowed addresses and -r removes a set\n"
"and both may appear any number of times.  Only hosts specifically\n"
"allowed and not specifically rejected will have their connection\n"
"relayed to the server.  The default behavior is therefore to reject all\n"
"connections, so you must use at least one -a argument to use the\n"
"relay.\n"
"\n"
"An address `src' matches an address set if (src & mask) == (addr & mask)\n"
"is true.  An address is tested against the -a and -r arguments in the\n"
"order they're listed.  It's accepted as soon as it matches an -a set\n"
"and rejected as soon as it matches a -r set, so you must list more\n"
"specific sets before more general sets.  For example, to allow all\n"
"hosts except hosts in the 192.0.2 subnet use:  `-r 192.0.2.0\n"
"255.255.255.0 -a 0.0.0.0 0.0.0.0'.  To also allow host 192.0.2.1 use:\n"
"`-a 192.0.2.1 255.255.255.255 -r 192.0.2.0 255.255.255.0 -a 0.0.0.0\n"
"0.0.0.0'.  Take care when choosing the arguments to avoid allowing\n"
"unwanted clients to connect.\n"
"\n"
"This program will only connect to the supplied bzflag server and relay\n"
"packets between the server and allowed clients.  The bzflag server does\n"
"not provide any means for escaping to a shell, starting executables,\n"
"using the filesystem, reporting on system resources, etc.\n";

static void		printl(int level, int priority, const char* msg, ...)
{
  va_list args;

  /* filter out unwanted messages */
  if (debugLevel < level)
    return;

  /* format.  WARNING -- buffer overrun is possible;  choose msg with care. */
  va_start(args, msg);
  if (usingSyslog) {
    char buffer[1024];
    vsprintf(buffer, msg, args);
    switch (priority) {
      case BZFR_LOG_INFO: priority = LOG_INFO; break;
      case BZFR_LOG_WARN: priority = LOG_WARNING; break;
      case BZFR_LOG_ERR:  priority = LOG_ERR; break;
    }
    syslog(priority, buffer);
  }
  else {
    fprintf(stderr, "%s: ", priorityString[priority]);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
  }
  va_end(args);
}

/*
 * signal installing and handlers
 */

typedef void(*Callback)(int);
static void		setSignal(int sig, Callback fn)
{
  struct sigaction sa;
  sa.sa_flags     = 0;
  sa.sa_handler   = fn;
  sigemptyset(&sa.sa_mask);
  sigaction(sig, &sa, NULL);
}

static void		doQuit(int sig)
{
  (void)sig; /* silence compiler */

  /* drop out of main loop on next iteration */
  done = 1;
}

static void		doIncLogging(int sig)
{
  (void)sig; /* silence compiler */

  /* raise debugging level */
  if (debugLevel < MAX_DEBUG_LEVEL) {
    debugLevel++;
    printl(DL_ALERT, BZFR_LOG_INFO, "logging level now %d", debugLevel);
  }
}

static void		doDecLogging(int sig)
{
  (void)sig; /* silence compiler */

  /* lower debugging level */
  if (debugLevel > 0) {
    debugLevel--;
    printl(DL_ALERT, BZFR_LOG_INFO, "logging level now %d", debugLevel);
  }
}

/*
 * address and port parsing and validating
 */

static int		stringToAddress(struct in_addr* addr, const char* string)
{
  /* convert string to an address;  string may be a hostname or an ip address.
   * return 0 on success, -1 on failure */

  struct hostent* hp;
  if ((hp = gethostbyname(string)) != 0) {
    memcpy(addr, hp->h_addr_list[0], sizeof(*addr));
    return 0;
  }

  addr->s_addr = inet_addr(string);
  if (addr->s_addr == INADDR_NONE)
    return -1;

  addr->s_addr = htonl(addr->s_addr);
  return 0;
}

static int		stringToPort(u_short* port, const char* string)
{
  int tmp;
  struct servent* sp;

  /* try it by name */
  if ((sp = getservbyname(string, "tcp"))) {
    *port = sp->s_port;
    return 0;
  }

  /* try it by number */
  tmp = atoi(string);
  if (tmp <= 0 || tmp > 65535)
    return -1;

  *port = htons((u_short)tmp);
  return 0;
}

static int		parseAddress(struct sockaddr_in* addr, const char* inString,
				const char* defaultPort, int iface)
{
  char* string;
  const char* port;
  int portOnly;

  (void)iface; /* silence compiler */

  /* copy string so we can muss it up */
  string = strdup(inString);
  if (!string) return -1;

  /* find port suffix */
  port = strrchr(string, ':');

  /* note if port suffix is all there is */
  portOnly = (port == string);

  /* chop off port suffix.  if missing use default port. */
  if (port) { *((char*)port) = '\0'; port++; }
  else port = defaultPort ? defaultPort : "0";

  /* parse address */
  if (!portOnly && stringToAddress(&addr->sin_addr, string) < 0) {
    free(string);
    return -2;
  }

  /* parse port */
  if (stringToPort(&addr->sin_port, port) < 0) {
    free(string);
    return -3;
  }

  /* clean up */
  free(string);

  /* fill in rest of address */
  addr->sin_family = AF_INET;

  /* if only port was there return 1, else 0 */
  if (portOnly) return 1;
  return 0;
}

/*
 * AddressSet
 */

static AddressSet*	newAddressSet(int allow, const struct in_addr* addr,
						 const struct in_addr* mask)
{
  /* make a new AddressSet and return it.  return 0 on failure. */
  AddressSet* set;

  /* make AddressSet */
  set = (AddressSet*)malloc(sizeof(AddressSet));
  if (!set) return 0;

  /* fill it in */
  set->next  = 0;
  set->allow = (allow != 0);
  set->addr  = *addr;
  set->mask  = *mask;
  return set;
}

/* not used
static void		deleteAddressSet(AddressSet* set)
{
  if (set) free(set);
}
*/

static int		addressInSet(const AddressSet* set, const struct in_addr* addr)
{
  /* return 1 if addr is in set, 0 if not */
  return ((addr->s_addr & set->mask.s_addr) == (set->addr.s_addr & set->mask.s_addr));
}

static int		testAddress(const AddressSet* list, const struct in_addr* addr)
{
  /* see if addr is allowed by scanning through list in order.  return 0 if no, 1 if yes.
   * default is no. */

  while (list) {
    /* only check matching address */
    if (addressInSet(list, addr))
      return list->allow;

    /* next set */
    list = list->next;
  }

  /* no match */
  return 0;
}

/*
 * common socket stuff
 */

#define FIX_FD_MAX(_fd)	if ((_fd) > fdMax) fdMax = (_fd)

static int		isSocketConnected(int fd)
{
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);
  return (getpeername(fd, (struct sockaddr*)&addr, &addrlen) >= 0);
}

static int		createListeningSocket(struct sockaddr_in* addr)
{
  int fd;

  /* create socket listen */
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    printl(DL_NOTICE, BZFR_LOG_ERR, "cannot create socket: %s",
							strerror(errno));
    return -1;
  }

  /* bind to requested address */
  if (bind(fd, (struct sockaddr*)addr, sizeof(*addr)) < 0) {
    printl(DL_NOTICE, BZFR_LOG_ERR, "cannot bind to %s:%d: %s",
		inet_ntoa(addr->sin_addr), ntohs(addr->sin_port),
		strerror(errno));
    close(fd);
    return -1;
  }

  /* start listening */
  if (listen(fd, 5) < 0) {
    printl(DL_NOTICE, BZFR_LOG_ERR, "cannot listen on %s:%d: %s",
		inet_ntoa(addr->sin_addr), ntohs(addr->sin_port),
		strerror(errno));
    close(fd);
    return -1;
  }

  FIX_FD_MAX(fd);
  return fd;
}

static int		createConnectingSocket(struct sockaddr_in* addr)
{
  int fd, flag;

  /* create a new socket */
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    printl(DL_NOTICE, BZFR_LOG_ERR, "cannot create socket: %s",
							strerror(errno));
    return -1;
  }

  /* make non-blocking */
  flag = fcntl(fd, F_GETFL, 0);
  if (flag == -1 || fcntl(fd, F_SETFL, flag | O_NDELAY) < 0) {
    printl(DL_NOTICE, BZFR_LOG_ERR, "cannot set non-blocking: %s",
							strerror(errno));
    close(fd);
    return -1;
  }

  /* start connecting */
  if (connect(fd, (CNCTType*)addr, sizeof(*addr)) < 0) {
    if (errno != EINPROGRESS) {
      printl(DL_NOTICE, BZFR_LOG_ERR, "cannot connect socket: %s",
							strerror(errno));
      close(fd);
      return -1;
    }
    /* EINPROGRESS is expected */
  }

  FIX_FD_MAX(fd);
  return fd;
}

static int		acceptConnection(int listener, struct sockaddr_in* addr)
{
  int fd, flag;
  socklen_t addrlen;

  /* accept connection */
  addrlen = sizeof(*addr);
  fd = accept(listener, (struct sockaddr*)addr, &addrlen);
  if (fd < 0) {
    printl(DL_NOTICE, BZFR_LOG_ERR, "cannot accept connection: %s",
							strerror(errno));
    return -1;
  }

  /* make connection non-blocking */
  flag = 1;
  if (ioctl(fd, FIONBIO, &flag) < 0) {
    printl(DL_NOTICE, BZFR_LOG_ERR, "cannot make non-blocking: %s",
							strerror(errno));
    close(fd);
    return -1;
  }

  FIX_FD_MAX(fd);
  return fd;
}

/*
 * listen port handling
 */

static int		startListening(struct sockaddr_in* addr,
				struct sockaddr_in* reconnect_addr)
{
  /* start listening */
  if ((fdListen = createListeningSocket(addr)) < 0) {
    printl(DL_IMPORTANT, BZFR_LOG_INFO, "cannot open listen socket");
    return -1;
  }

  /* start listening */
  if ((fdReconnect = createListeningSocket(reconnect_addr)) < 0) {
    printl(DL_IMPORTANT, BZFR_LOG_INFO, "cannot open reconnect socket");
    close(fdListen);
    fdListen = -1;
    return -1;
  }

  printl(DL_INFO, BZFR_LOG_INFO, "listening on %s:%d, reconnect on %d",
		inet_ntoa(addr->sin_addr), ntohs(addr->sin_port),
		ntohs(reconnect_addr->sin_port));
  return fdListen;
}

static int		stopListening(void)
{
  /* close down listen socket if it's open */
  if (fdListen != -1) {
    printl(DL_INFO, BZFR_LOG_INFO, "closing listen port");
    if (close(fdListen) < 0)
      printl(DL_NOTICE, BZFR_LOG_ERR, "cannot close listen port: %s",
							strerror(errno));
    fdListen = -1;
  }

  /* close down reconnect listen socket if it's open */
  if (fdReconnect != -1) {
    printl(DL_INFO, BZFR_LOG_INFO, "closing reconnect port");
    if (close(fdReconnect) < 0)
      printl(DL_NOTICE, BZFR_LOG_ERR, "cannot close reconnect port: %s",
							strerror(errno));
    fdReconnect = -1;
  }

  return 0;
}

/*
 * relay handling
 */

static void		closingRelay(Relay* relay, int sendReject)
{
  struct timezone tz;

  /* put relay in closing state (ready for timeout) */
  relay->status = CLOSING;
  gettimeofday(&relay->startTime, &tz);

  /* send client a rejection if requested */
  if (sendReject)
    send(relay->fdSrc, (SendType)"BZFS107e\000\000", 10, 0);
}

static void		createRelay(struct sockaddr_in* serverAddr,
				AddressSet* addresses)
{
  Relay* relay;
  int fdSrc;
  struct sockaddr_in addr;
  struct timezone tz;

  /* accept client connection */
  printl(DL_DEBUG, BZFR_LOG_INFO, "accepting client connection");
  fdSrc = acceptConnection(fdListen, &addr);
  if (fdSrc < 0) {
    printl(DL_IMPORTANT, BZFR_LOG_INFO, "cannot accept client connection");
    return;
  }

  /* allocate space for relay */
  relay = (Relay*)malloc(sizeof(Relay));
  if (relay) {
    relay->srcToDstBuffer = (unsigned char*)malloc(RELAY_BUFLEN);
    relay->dstToSrcBuffer = (unsigned char*)malloc(RELAY_BUFLEN);
    relay->srcName = (char*)malloc(strlen(inet_ntoa(addr.sin_addr)) + 16);
  }
  if (!relay || !relay->srcName ||
	!relay->srcToDstBuffer ||
	!relay->dstToSrcBuffer) {
    printl(DL_IMPORTANT, BZFR_LOG_ERR, "out of memory allocating relay");
    close(fdSrc);
    if (relay->srcToDstBuffer) free(relay->srcToDstBuffer);
    if (relay->dstToSrcBuffer) free(relay->dstToSrcBuffer);
    if (relay->srcName) free(relay->srcName);
    return;
  }

  /* initialize relay object */
  relay->status         = CONNECTING;
  relay->fdSrc          = fdSrc;
  relay->fdDst          = -1;
  relay->fdOldSrc       = -1;
  relay->fdOldDst       = -1;
  relay->srcMark        = relay->srcToDstBuffer;
  relay->dstMark        = relay->dstToSrcBuffer;
  relay->srcToDstFilled = 0;
  relay->dstToSrcFilled = 0;
  relay->srcToDstBytes  = 0;
  relay->dstToSrcBytes  = 0;
  relay->srcAddr        = addr.sin_addr;
  sprintf(relay->srcName, "%s:%d",
		inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
  gettimeofday(&relay->startTime, &tz);

  /* add to list */
  relay->prev = NULL;
  relay->next = relays;
  if (relay->next) relay->next->prev = relay;
  relays = relay;

  /* validate client address */
  if (!testAddress(addresses, &addr.sin_addr)) {
    /* connections from unwanted hosts is important to report */
    printl(DL_IMPORTANT, BZFR_LOG_WARN,
		"rejecting connection from %s", relay->srcName);
    closingRelay(relay, 1);
    return;
  }

  /* make a new socket and begin connecting to server */
  printl(DL_NOTICE, BZFR_LOG_INFO,
		"connection from %s", relay->srcName);
  printl(DL_DEBUG, BZFR_LOG_INFO,
		"connecting to server for %s", relay->srcName);
  relay->fdDst = createConnectingSocket(serverAddr);
  if (relay->fdDst < 0) {
    printl(DL_IMPORTANT, BZFR_LOG_INFO,
			"cannot create server connection socket");
    closingRelay(relay, 1);
    return;
  }
}

static int		shutdownRelay(Relay* relay)
{
  /* close connections */
  if (relay->fdSrc != -1) close(relay->fdSrc);
  if (relay->fdDst != -1) close(relay->fdDst);
  if (relay->fdOldSrc != -1) close(relay->fdOldSrc);
  if (relay->fdOldDst != -1) close(relay->fdOldDst);
  relay->fdSrc    = -1;
  relay->fdDst    = -1;
  relay->fdOldSrc = -1;
  relay->fdOldDst = -1;
  relay->status   = DISABLED;
  return 0;
}

static int		closeRelay(Relay* relay)
{
  /* close connections */
  shutdownRelay(relay);

  /* log it */
  printl(DL_NOTICE, BZFR_LOG_INFO, "closed connection from %s", relay->srcName);
  printl(DL_INFO, BZFR_LOG_INFO,
		"%d bytes client to server, %d bytes server to client",
		relay->srcToDstBytes, relay->dstToSrcBytes);

  /* remove from list */
  if (relay->prev) relay->prev->next = relay->next;
  else relays = relay->next;
  if (relay->next) relay->next->prev = relay->prev;

  /* free memory */
  free(relay->srcToDstBuffer);
  free(relay->dstToSrcBuffer);
  free(relay->srcName);
  free(relay);
  return 0;
}

static int		reapRelays(void)
{
  Relay* scan;

  /* find relays that are shutdown and remove them */
  for (scan = relays; scan; ) {
    Relay* next = scan->next;
    if (scan->fdSrc == -1 && scan->fdDst == -1)
      closeRelay(scan);
    scan = next;
  }
  return 0;
}

static int		stopRelays(void)
{
  /* close and remove all relays */
  while (relays) {
    Relay* next = relays->next;
    closeRelay(relays);
    relays = next;
  }
  return 0;
}

static void		getRelayTimeout(Relay* relay, struct timeval* t)
{
  t->tv_sec  = relay->startTime.tv_sec + NETWORK_TIMEOUT;
  t->tv_usec = relay->startTime.tv_usec;
}

static void		connectToServer(Relay* relay)
{
  struct timezone tz;

  /* now connected to server.  make sure we're really connected */
  if (!isSocketConnected(relay->fdDst)) {
    printl(DL_IMPORTANT, BZFR_LOG_ERR,
		"failed to connect to server for %s", relay->srcName);
    closingRelay(relay, 1);
    return;
  }

  /* next stage */
  printl(DL_DEBUG, BZFR_LOG_INFO,
		"established server connection for %s", relay->srcName);
  relay->status = FIRST_CONTACT;
  gettimeofday(&relay->startTime, &tz);
}

static void		readHello(Relay* relay, struct sockaddr_in* serverAddr)
{
  int n;

  /* read data from server */
  n = recv(relay->fdDst, (RecvType)relay->dstMark, 10, 0);
  if (n < 0) {
    printl(DL_NOTICE, BZFR_LOG_WARN, "error reading from server: %s",
							strerror(errno));
    return;
  }
  if (n == 0) {
    printl(DL_NOTICE, BZFR_LOG_ERR, "server hungup on %s", relay->srcName);
    closingRelay(relay, 1);
    return;
  }
  relay->dstMark += n;

  /* when we've got the entire hello message, close the connection to the
   * server and connect again at the new port.  if the server is rejecting
   * us then relay the hello (and goodbye) message and shutdown. */
  if (relay->dstMark - relay->dstToSrcBuffer >= 10) {
    unsigned short port;

    /* check header */
    if (relay->dstToSrcBuffer[0] != 'B' ||
	relay->dstToSrcBuffer[1] != 'Z' ||
	relay->dstToSrcBuffer[2] != 'F' ||
	relay->dstToSrcBuffer[3] != 'S') {
      /* not a bzflag server.  pass on whatever gibberish we got */
      send(relay->fdSrc, (SendType)relay->dstToSrcBuffer, 10, 0);
      closingRelay(relay, 0);

      for (n = 0; n < 10; n++)
	if (!isprint(relay->dstToSrcBuffer[n]))
	  relay->dstToSrcBuffer[n] = ' ';
      printl(DL_NOTICE, BZFR_LOG_WARN, "not a bzflag server: %10.10s",
						relay->dstToSrcBuffer);
      return;
    }

    /* port is in network byte order */
    port = ((unsigned short)relay->dstToSrcBuffer[8] << 8) +
			   (unsigned short)relay->dstToSrcBuffer[9];

    /* if port is zero, server is rejecting connection */
    if (port == 0) {
      printl(DL_NOTICE, BZFR_LOG_INFO, "server rejected %s", relay->srcName);
      send(relay->fdSrc, (SendType)relay->dstToSrcBuffer, 10, 0);
      closingRelay(relay, 0);
      return;
    }

    /* if port is not zero, server is allowing connection */
    else {
      int fdDst;
      struct sockaddr_in addr;
      struct timezone tz;

      /* begin reconnect to server */
      printl(DL_INFO, BZFR_LOG_INFO, "client %s accepted;  reconnecting",
							relay->srcName);
      printl(DL_DEBUG, BZFR_LOG_INFO, "server accepted %s on port %d",
							relay->srcName, port);
      addr          = *serverAddr;
      addr.sin_port = htons(port);
      fdDst = createConnectingSocket(&addr);
      if (fdDst < 0) {
	printl(DL_IMPORTANT, BZFR_LOG_ERR,
			"cannot create server reconnection socket");
	closingRelay(relay, 1);
	return;
      }

      /* start using new socket.  don't close old connection yet because
       * server will tear down the new socket before we have a chance
       * to use it if we do. */
      relay->fdOldDst = relay->fdDst;
      relay->fdDst    = fdDst;

      /* next stage */
      relay->status = RECONNECTING;
      gettimeofday(&relay->startTime, &tz);
    }
  }
}

static void		connectToServerAgain(Relay* relay,
				struct sockaddr_in* listenAddr)
{
  socklen_t addrlen;
  struct sockaddr_in addr;
  struct timezone tz;

  (void)listenAddr; /* silence compiler */

  /* now reconnected to server.  get peer name to make sure we're connected. */
  if (!isSocketConnected(relay->fdDst)) {
    printl(DL_IMPORTANT, BZFR_LOG_ERR,
		"failed to reconnect to server for %s", relay->srcName);
    closingRelay(relay, 1);
    return;
  }
  printl(DL_DEBUG, BZFR_LOG_INFO,
		"reconnected to server for %s", relay->srcName);

  /* close old server connection */
  close(relay->fdOldDst);
  relay->fdOldDst = -1;

  /* get the port number of the reconnect socket */
  addrlen = sizeof(addr);
  if (getsockname(fdReconnect, (struct sockaddr*)&addr, &addrlen) < 0) {
    printl(DL_IMPORTANT, BZFR_LOG_ERR,
		"cannot get port number of server connection for %s: %s",
		relay->srcName, strerror(errno));
    closingRelay(relay, 1);
    return;
  }
  printl(DL_DEBUG, BZFR_LOG_INFO,
		"listening for client %s reconnect on port %d",
		relay->srcName, ntohs(addr.sin_port));

  /* modify hello message to provide new port (in network byte order) */
  relay->dstToSrcBuffer[8] = (unsigned char)((ntohs(addr.sin_port) >> 8) & 0xff);
  relay->dstToSrcBuffer[9] = (unsigned char)(ntohs(addr.sin_port) & 0xff);

  /* send hello message to client to cause client to reconnect */
  if (send(relay->fdSrc, (SendType)relay->dstToSrcBuffer, 10, 0) != 10) {
    printl(DL_NOTICE, BZFR_LOG_ERR, "cannot send hello to client %s: %s",
		relay->srcName, strerror(errno));
    closingRelay(relay, 0);
    return;
  }

  /* wait for client to close old connection */
  relay->fdOldSrc = relay->fdSrc;

  /* no active connection to client */
  relay->fdSrc = -1;

  /* next stage */
  relay->status  = AWAITING_RECONNECT;
  relay->dstMark = relay->dstToSrcBuffer;
  gettimeofday(&relay->startTime, &tz);
}

static void		acceptClientReconnect(Relay* relays)
{
  int fdSrc;
  struct sockaddr_in addr;
  Relay* scan;

  /* accept client reconnection */
  fdSrc = acceptConnection(fdReconnect, &addr);
  if (fdSrc < 0) {
    printl(DL_IMPORTANT, BZFR_LOG_INFO, "failed to accept reconnect");
    return;
  }

  /* figure out which client */
  for (scan = relays; scan; scan = scan->next)
    if (scan->status == AWAITING_RECONNECT &&
	addr.sin_addr.s_addr == scan->srcAddr.s_addr)
      break;

  /* if no client found then some unexpected client tried our
     reconnect port.  this is suspicious. */
  if (!scan) {
    printl(DL_IMPORTANT, BZFR_LOG_WARN,
		"rejecting unexpected connection from %s",
		inet_ntoa(addr.sin_addr));
    return;
  }

  /* stop listening on listen socket and start using connection */
  scan->fdSrc = fdSrc;
  printl(DL_DEBUG, BZFR_LOG_INFO,
		"accepting reconnect from client %s", scan->srcName);

  /* next stage */
  scan->status = RELAYING;
  printl(DL_NOTICE, BZFR_LOG_INFO, "relaying for %s", scan->srcName);
}

static void		readFromServer(Relay* relay)
{
  /* read from server */
  int n = recv(relay->fdDst, (RecvType)relay->dstToSrcBuffer, RELAY_BUFLEN, 0);

  /* put relay in closing state if server hungup */
  if (n == 0) {
    closingRelay(relay, 0);
  }

  /* prepare to send data to client */
  else if (n > 0) {
    relay->dstMark        = relay->dstToSrcBuffer;
    relay->dstToSrcFilled = n;
  }
}

static void		readFromClient(Relay* relay)
{
  /* read from client */
  int n = recv(relay->fdSrc, (RecvType)relay->srcToDstBuffer, RELAY_BUFLEN, 0);

  /* shutdown relay if client hungup */
  if (n == 0) {
    shutdownRelay(relay);
  }

  /* prepare to send data to server */
  else if (n > 0) {
    relay->srcMark        = relay->srcToDstBuffer;
    relay->srcToDstFilled = n;
  }
}

static void		writeToServer(Relay* relay)
{
  /* write data to server */
  int n = send(relay->fdDst, (SendType)relay->srcMark, relay->srcToDstFilled, 0);

  /* update output queue and record bytes sent */
  if (n > 0) {
    relay->srcMark        += n;
    relay->srcToDstFilled -= n;
    relay->srcToDstBytes  += n;
  }
}

static void		writeToClient(Relay* relay)
{
  /* write data to client */
  int n = send(relay->fdSrc, (SendType)relay->dstMark, relay->dstToSrcFilled, 0);

  /* update output queue and record bytes sent */
  if (n > 0) {
    relay->dstMark        += n;
    relay->dstToSrcFilled -= n;
    relay->dstToSrcBytes  += n;
  }
}


/*
 * daemonize a process
 */

static int		daemonize(void)
{
  int i;
  pid_t pid;
#ifdef HAVE_RLIMIT
  struct rlimit rl;
#endif
  int rlim_max = 1024;

  umask(0);

  /* get the maximum number of file descriptors.  we really want the
   * highest open file descriptor but i don't know how to get that. */
#ifdef HAVE_RLIMIT
  if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
    return -1;
#endif

  /* become session leader to detach from controlling terminal */
  if ((pid = fork()) < 0)
    return -1;
  else if (pid != 0)
    exit(0);
  setsid();

  /* become not a session leader so we won't open a controlling terminal */
  setSignal(SIGHUP, SIG_IGN);
  if ((pid = fork()) < 0)
    return -1;
  else if (pid != 0)
    exit(0);

#ifdef HAVE_RLIMIT
  /* if infinity then set to a lower limit.  this is bad but we don't want
   * to try closing all fd's up to RLIM_INFINITY. */
  if (rl.rlim_max == RLIM_INFINITY)
    rl.rlim_max = 1024;
  rlim_max = (int)rl.rlim_max;
#endif

  /* close all files except stderr */
  for (i = 0; i < rlim_max; i++)
    if (i != 2)
      if (close(i) < 0 && errno != EBADF)
	return -1;

  /* reset errno */
  errno = 0;

  /* change to root directory */
  if (chdir("/") < 0)
    return -1;

  /* attach 0, 1, and 2 to /dev/null just in case something expects these
   * to be attached to stdin, stdout, and stderr */
  if ((i = open("/dev/null", O_RDWR)) != 0) {
    if (i < 0) return -1;
    else return -1;		/* unexpected fd */
  }

  if ((i = dup(0)) != 1) {
    if (i < 0) return -1;
    else return -1;		/* unexpected fd */
  }

  if (close(2) < 0 && errno != EBADF)
    return -1;
  if ((i = dup(0)) != 2) {
    if (i < 0) return -1;
    else return -1;		/* unexpected fd */
  }

  return 0;
}

/*
 * main app
 */

static void		usage(void)
{
  strlen(copyright); /* force compiler to keep the copyright string */
  fprintf(stderr, usageString, pname);
  exit(1);
}

static void		help(void)
{
  fprintf(stdout, usageString, pname);
  fprintf(stdout, helpString);
  exit(0);
}

int			main(int argc, char** argv)
{
  int i, foreground = 0, gotServer = 0, gotListen = 0, gotReconnect = 0;
  AddressSet* addresses, *lastAddress;
  struct sockaddr_in listenOn, reconnectOn, relayTo;
  fd_set read_set, write_set;
  struct timeval currentTime, timeout, *timeoutp;
  struct timezone tz;
  Relay* scan;

  /* initialize */
  pname = argv[0];
  relays = 0;
  addresses = 0;
  lastAddress = 0;
  done = 0;
  debugLevel = 0;
  fdMax = 0;
  fdListen = -1;
  fdReconnect = -1;
  usingSyslog = 1;
  if (parseAddress(&listenOn, "0.0.0.0", BZFS_PORT, 1) != 0) {
    fprintf(stderr, "internal error:  invalid default address\n");
    exit(1);
  }
  if (parseAddress(&reconnectOn, "0.0.0.0", RECONNECT_PORT, 1) != 0) {
    fprintf(stderr, "internal error:  invalid default address\n");
    exit(1);
  }

  /* parse arguments */
  for (i = 1; i < argc; i++) {
    if (gotServer) {
      /* already parsed the server address -- no arguments after server allowed */
      fprintf(stderr, "illegal argument %s after server address\n", argv[i]);
      usage();
    }

    else if (strcmp(argv[i], "-h") == 0) {
      help();
    }

    else if (strncmp(argv[i], "-d", 2) == 0) {
      /* could be debugging argument.  must be all d's */
      const char* scan = argv[i] + 1;
      while (*scan == 'd') scan++;
      if (*scan) goto parseServerArg;
      debugLevel += (scan - argv[i]) - 1;
      if (debugLevel > MAX_DEBUG_LEVEL)
	debugLevel = MAX_DEBUG_LEVEL;
    }

    else if (strcmp(argv[i], "-f") == 0) {
      foreground = 1;
      usingSyslog = 0;
    }

    else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "-r") == 0) {
      AddressSet* newSet;
      struct in_addr addr, mask;

      /* verify we have enough arguments */
      if (i + 2 >= argc) {
	fprintf(stderr, "missing argument for %s\n", argv[i]);
	usage();
      }

      /* parse addresses */
      if (stringToAddress(&addr, argv[i + 1]) < 0) {
	fprintf(stderr, "unknown host %s for %s\n", argv[i + 1], argv[i]);
	usage();
      }
      if (stringToAddress(&mask, argv[i + 1]) < 0) {
	fprintf(stderr, "invalid mask %s for %s\n", argv[i + 1], argv[i]);
	usage();
      }

      /* make address set */
      newSet = newAddressSet(strcmp(argv[i], "-a") == 0, &addr, &mask);
      if (!newSet) {
	fprintf(stderr, "out of memory\n");
	return 2;
      }

      /* add to tail of linked list */
      if (lastAddress)
	lastAddress->next = newSet;
      else
	addresses = newSet;
      lastAddress = newSet;

      /* skip arguments */
      i += 2;
    }

    else if (strcmp(argv[i], "-p") == 0) {
      /* can't have this argument twice */
      if (gotReconnect) {
	fprintf(stderr, "argument %s cannot appear more than once\n", argv[i]);
	usage();
      }

      /* verify we have enough arguments */
      if (i + 1 >= argc) {
	fprintf(stderr, "missing argument for %s\n", argv[i]);
	usage();
      }

      /* parse port */
      if (stringToPort(&reconnectOn.sin_port, argv[i + 1]) < 0) {
	fprintf(stderr, "invalid port: %s\n", argv[i + 1]);
	return 1;
      }

      gotReconnect = 1;
      i += 1;
    }
    else if (strcmp(argv[i], "-s") == 0) {
      u_short savedPort = reconnectOn.sin_port;

      /* can't have this argument twice */
      if (gotListen) {
	fprintf(stderr, "argument %s cannot appear more than once\n", argv[i]);
	usage();
      }

      /* verify we have enough arguments */
      if (i + 1 >= argc) {
	fprintf(stderr, "missing argument for %s\n", argv[i]);
	usage();
      }

      /* parse address */
      switch (parseAddress(&listenOn, argv[i + 1], BZFS_PORT, 1)) {
	case 0:
	case 1:
	  break;

	case -1:
	  fprintf(stderr, "out of memory\n");
	  return 2;

	case -2:
	  fprintf(stderr, "unknown interface: %s\n", argv[i + 1]);
	  return 1;

	case -3:
	  fprintf(stderr, "invalid port: %s\n", argv[i + 1]);
	  return 1;

	default:
	  fprintf(stderr, "invalid argument: %s\n", argv[i + 1]);
	  return 1;
      }
      parseAddress(&reconnectOn, argv[i + 1], RECONNECT_PORT, 1);
      reconnectOn.sin_port = savedPort;
      /* note -- bind() will fail later if listenOn is not a local address */

      gotListen = 1;
      i += 1;
    }

    else {
parseServerArg:
      /* should be server address with optional port.  parse server address. */
      switch (parseAddress(&relayTo, argv[i], BZFS_PORT, 0)) {
	case 0:
	  break;

	case -1:
	  fprintf(stderr, "out of memory\n");
	  return 2;

	case 1:
	  /* have to have a host address */
	case -2:
	  fprintf(stderr, "unknown host: %s\n", argv[i]);
	  return 1;

	case -3:
	  fprintf(stderr, "invalid port: %s\n", argv[i]);
	  return 1;

	default:
	  fprintf(stderr, "invalid argument: %s\n", argv[i]);
	  return 1;
      }

      gotServer = 1;
    }
  }

  /* must have server by now */
  if (!gotServer) {
    fprintf(stderr, "missing server argument\n");
    usage();
  }

  /* daemonize.  note -- stderr may be closed even on error. */
  if (!foreground && daemonize() < 0) {
    fprintf(stderr, "failed to daemonize.  exiting.\n");
    return 2;
  }

  /* open syslog (after daemonize(), which closes all file descriptors) */
  if (usingSyslog)
    openlog("bzfrelay", LOG_PID | LOG_CONS, BZFR_LOG_FACILITY);

  /* install signal handlers */
  setSignal(SIGINT, doQuit);
  setSignal(SIGTERM, doQuit);
  setSignal(SIGUSR1, doIncLogging);
  setSignal(SIGUSR2, doDecLogging);
  setSignal(SIGPIPE, SIG_IGN);		/* allow writes on closed sockets */

  printl(DL_ALERT, BZFR_LOG_INFO, "starting");
  printl(DL_INFO, BZFR_LOG_INFO, "server is %s:%d",
		inet_ntoa(relayTo.sin_addr), ntohs(relayTo.sin_port));

  /* start listening */
  if (startListening(&listenOn, &reconnectOn) < 0)
    done = 1;

  /* main loop */
  while (!done) {
    /* prepare select bitfields */
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);

    /* listen for new connections from clients */
    FD_SET(fdListen, &read_set);

    timeout.tv_sec  = 0;
    timeout.tv_usec = 0;
    for (scan = relays; scan; scan = scan->next) {
      switch (scan->status) {
	case CONNECTING: {
	  /* we've accepted the client connection and now we're waiting
	   * for the server to accept our connection to it.  ignore the
	   * client. */
	  struct timeval t;
	  getRelayTimeout(scan, &t);
	  if ((t.tv_sec < timeout.tv_sec) ||
	      (t.tv_sec == timeout.tv_sec && t.tv_usec < timeout.tv_usec))
	    timeout = t;
	  FD_SET(scan->fdDst, &write_set);
	  break;
	}

	case FIRST_CONTACT: {
	  /* we're now connected on both sides waiting for the server
	   * to send back a hello.  ignore the client. */
	  struct timeval t;
	  getRelayTimeout(scan, &t);
	  if ((t.tv_sec < timeout.tv_sec) ||
	      (t.tv_sec == timeout.tv_sec && t.tv_usec < timeout.tv_usec))
	    timeout = t;
	  FD_SET(scan->fdDst, &read_set);
	  break;
	}

	case RECONNECTING: {
	  /* we've received the server's hello and now we're waiting for
	   * the server to accept our new connection to it.  we haven't
	   * relayed the server's hello yet so we still ignore the client. */
	  struct timeval t;
	  getRelayTimeout(scan, &t);
	  if ((t.tv_sec < timeout.tv_sec) ||
	      (t.tv_sec == timeout.tv_sec && t.tv_usec < timeout.tv_usec))
	    timeout = t;
	  FD_SET(scan->fdDst, &write_set);
	  break;
	}

	case AWAITING_RECONNECT: {
	  /* we've reconnected to the server, relayed the hello to the
	   * client, hungup on the client, and now we're waiting for the
	   * client to reconnect.  ignore the server until then. */
	  struct timeval t;
	  getRelayTimeout(scan, &t);
	  if ((t.tv_sec < timeout.tv_sec) ||
	      (t.tv_sec == timeout.tv_sec && t.tv_usec < timeout.tv_usec))
	    timeout = t;
	  FD_SET(fdReconnect, &read_set);
	  break;
	}

	case RELAYING:
	  /* okay, we've connected to both the client and the server.
	   * listen for data on both ends (if they're still open) and
	   * wait for writability on the ends that have data waiting to
	   * be written.  remember that we don't want to buffer data
	   * for long because it must be delivered quickly. */
	  if (scan->fdDst != -1) {
	    if (scan->dstToSrcFilled == 0)
	      FD_SET(scan->fdDst, &read_set);
	    if (scan->srcToDstFilled != 0)
	      FD_SET(scan->fdDst, &write_set);
	  }
	  if (scan->fdSrc != -1) {
	    if (scan->srcToDstFilled == 0)
	      FD_SET(scan->fdSrc, &read_set);
	    if (scan->dstToSrcFilled != 0)
	      FD_SET(scan->fdSrc, &write_set);
	  }
	  break;

	case CLOSING: {
	  /* waiting for the client to close its end */
	  struct timeval t;
	  getRelayTimeout(scan, &t);
	  if ((t.tv_sec < timeout.tv_sec) ||
	      (t.tv_sec == timeout.tv_sec && t.tv_usec < timeout.tv_usec))
	    timeout = t;
	  FD_SET(scan->fdSrc, &read_set);
	  break;
	}
      }

      /* listen for old client and old server connection hangup */
      if (scan->fdOldSrc != -1)
	FD_SET(scan->fdOldSrc, &read_set);
      if (scan->fdOldDst != -1)
	FD_SET(scan->fdOldDst, &read_set);
    }

    /* see if there's a connection to timeout */
    timeoutp = NULL;
    if (timeout.tv_sec != 0 || timeout.tv_usec) {
      /* compute time until timeout */
      gettimeofday(&currentTime, &tz);
      if ((timeout.tv_sec < currentTime.tv_sec) ||
	  (timeout.tv_sec == currentTime.tv_sec &&
	   timeout.tv_usec <= currentTime.tv_usec)) {
	timeout.tv_sec  = 0;
	timeout.tv_usec = 0;
      }
      else {
	timeout.tv_sec  -= currentTime.tv_sec;
	timeout.tv_usec -= currentTime.tv_usec;
      }
      timeoutp = &timeout;
    }

    /* wait for an event */
    i = select(fdMax + 1, &read_set, &write_set, 0, timeoutp);

    /* handle timeouts */
    gettimeofday(&currentTime, &tz);
    for (scan = relays; scan; scan = scan->next) {
      if (scan->status == CONNECTING ||
	  scan->status == RECONNECTING ||
	  scan->status == AWAITING_RECONNECT ||
	  scan->status == CLOSING) {
	getRelayTimeout(scan, &timeout);
	if ((timeout.tv_sec < currentTime.tv_sec) ||
	    (timeout.tv_sec == currentTime.tv_sec &&
	     timeout.tv_usec <= currentTime.tv_usec)) {
	  const char* msg;
	  if (scan->status == CONNECTING)
	    msg = "server connection";
	  else if (scan->status == RECONNECTING)
	    msg = "server reconnection";
	  else if (scan->status == AWAITING_RECONNECT)
	    msg = "client reconnection";
	  else
	    msg = "client hangup";
	  printl(DL_NOTICE, BZFR_LOG_WARN,
		"%s timed out for %s", msg, scan->srcName);
	  shutdownRelay(scan);
	}
      }
    }

    /* handle errors */
    if (i < 0) {
      if (errno != EINTR) {
	/* this is bad -- quit */
	printl(DL_ALERT, BZFR_LOG_ERR,
		"select() returned %500s", strerror(errno));
	break;
      }

      /* EINTR is okay -- we returned from select() because of a signal */
      continue;
    }

    /* handle events -- first handle new connections on the well known port */
    if (FD_ISSET(fdListen, &read_set))
      createRelay(&relayTo, addresses);

    /* now handle reconnections */
    if (FD_ISSET(fdReconnect, &read_set))
      acceptClientReconnect(relays);

    /* handle relay sockets */
    for (scan = relays; scan; scan = scan->next) {
      switch (scan->status) {
	case CONNECTING:
	  if (FD_ISSET(scan->fdDst, &write_set))
	    connectToServer(scan);
	  break;

	case FIRST_CONTACT:
	  if (FD_ISSET(scan->fdDst, &read_set))
	    readHello(scan, &relayTo);
	  break;

	case RECONNECTING:
	  if (FD_ISSET(scan->fdDst, &write_set))
	    connectToServerAgain(scan, &listenOn);
	  break;

	case RELAYING:
	  if (scan->fdDst != -1)
	    if (FD_ISSET(scan->fdDst, &read_set))
	      readFromServer(scan);
	  if (scan->fdDst != -1)
	    if (FD_ISSET(scan->fdDst, &write_set))
	      writeToServer(scan);
	  if (scan->fdSrc != -1)
	    if (FD_ISSET(scan->fdSrc, &read_set))
	      readFromClient(scan);
	  if (scan->fdSrc != -1)
	    if (FD_ISSET(scan->fdSrc, &write_set))
	      writeToClient(scan);
	  break;

	case CLOSING:
	  if (FD_ISSET(scan->fdSrc, &read_set))
	    shutdownRelay(scan);
	  break;
      }

      /* close down old sockets.  we don't expect to receive anything
       * on these sockets so just throw away anything that comes in. */
      if (scan->fdOldSrc != -1 && FD_ISSET(scan->fdOldSrc, &read_set)) {
	char buffer[256];
	if (recv(scan->fdOldSrc, (RecvType)buffer, sizeof(buffer), 0) == 0) {
	  close(scan->fdOldSrc);
	  scan->fdOldSrc = -1;
	}
      }
      if (scan->fdOldDst != -1 && FD_ISSET(scan->fdOldDst, &read_set)) {
	char buffer[256];
	if (recv(scan->fdOldDst, (RecvType)buffer, sizeof(buffer), 0) == 0) {
	  close(scan->fdOldDst);
	  scan->fdOldDst = -1;
	}
      }
    }

    /* reap fully closed connections */
    reapRelays();
  }

  stopListening();
  stopRelays();

  printl(DL_ALERT, BZFR_LOG_INFO, "terminating");
  if (usingSyslog)
    closelog();

  return 0;
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
