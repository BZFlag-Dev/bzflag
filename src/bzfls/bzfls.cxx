/* bzfls
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

static const char	copyright[] = "Copyright (c) 1993 - 2001 Tim Riker";

// must be before windows.h
#include "network.h"

#if defined(_WIN32)
#include <windows.h>
#define	strcasecmp	_stricmp
#define sleep(_x)	Sleep(1000 * (_x))
#endif /* defined(_WIN32) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include "common.h"
#include "global.h"
#include "Protocol.h"
#include "Ping.h"
#include "TimeKeeper.h"
#include "TimeBomb.h"

// give up on connections that have been idle DisconnectTimeout seconds.
static const float DisconnectTimeout = 10.0f;

// purge list every CheckListInterval seconds of servers that haven't
// talked to us in ServerExpiration seconds.  CheckListInterval can be
// fairly small since it's only local computation.  ServerExpiration
// should be high enough to allow servers to send updates without
// being purged and without bugging us too much.  however, dead servers
// can remain in the list for up to ServerExpiration seconds, so it
// should be kept reasonably low.
static const float CheckListInterval = 10.0f;
static const float ServerExpiration = 45.0f * 60.0f;

// expect messages to be no longer than this
static const int MaxInputMsgSize = 1024;

// handle this many clients at once.  since we do not hold lengthly
// dialogues with clients this doesn't have to be too large.
static const int MaxClients = 20;

static const int NotConnected = -1;	// do NOT change

class Server {
  public:
    Server(const char *name, const char* version, const char* build,
				const char* gameInfo, const char* title);
    ~Server();

    void		addAfter(Server*);
    void		ref();
    void		unref();
    void		setStale();
    void		setFresh();
    void		setNumPlayers(int* players);
    void		setGameInfo(const char* gameInfo);
    void		setTitle(const char* title);

    Server*		getNext() const;
    boolean		isReferenced() const;
    boolean		isStale(const TimeKeeper&) const;
    boolean		isFresh() const;
    const char*		getTitle() const;
    const char*		getAddress() const;
    const char*		getVersion() const;
    const char*		getBuild() const;
    const char*		getGameInfo() const;
    const char*		getName() const;
    const char*		getPort() const;

  public:
    char		address[16]; // enough space for "AAA.BBB.CCC.DDD" including null

  private:
    Server*		prev;
    Server*		next;
    boolean		fresh;
    int			refCount;
    char*		title;
    char		version[9];
    char*		build;
    char*		gameInfo;
    char*		name;
    char*		port;
    TimeKeeper		time;
};

struct Client {
  public:
    int			fd;
    boolean		sendingReply;
    char		buffer[MaxInputMsgSize];
    int			offset;
    int			length;
    Server*		nextServer;
    TimeKeeper		time;
};

class TestServer {
  public:
    TestServer(int fd, Server*);
    ~TestServer();

    void		addAfter(TestServer*);
    void		setWait();
    boolean		testWaitAndReset();
    Server*		orphanServer();

    TestServer*		getNext() const;
    int			getFD() const;
    Server*		getServer() const;
    const TimeKeeper&	getTime() const;

  private:
    TestServer*		prev;
    TestServer*		next;
    int			fd;
    Server*		server;
    boolean		wait;
    TimeKeeper		startTime;
};

static int		debug = 0;
static boolean		useGivenPort = False;
static int		wksSocket;
static int		wksPort = ServerPort + 1;
static int		maxFileDescriptor;
static time_t		lastChangeTime;
static Server*		serverList;
static TestServer*	testingList;
static Client		client[MaxClients];

// statistics
static int		bytesRead = 0, bytesReadTotal = 0;
static int		bytesWritten = 0, bytesWrittenTotal = 0;
static int		numAdds = 0, numAddsTotal = 0, numAddsBad = 0;
static int		numRemoves = 0, numRemovesTotal = 0, numRemovesBad = 0;
static int		numSets = 0, numSetsTotal = 0, numSetsBad = 0;
static int		numGets = 0, numGetsTotal = 0, numGetsBad = 0;
static int		numLists = 0, numListsTotal = 0;
static int		numBad = 0, numBadTotal = 0;
static TimeKeeper	lastDumpTime, startTime;

//
// Server
//

Server::Server(const char* inName,
		const char* inVersion,
		const char* inBuild,
		const char* inGameInfo,
		const char* inTitle) :
		prev(NULL),
		next(NULL),
		fresh(True),
		refCount(1),
		time(TimeKeeper::getCurrent())
{
  name = strdup(inName);
  // start with a padded localhost, fill in address when testing
  build = strdup(inBuild);
  title = strdup(inTitle);
  gameInfo = strdup(inGameInfo);
  strncpy(version, inVersion, 8);
  version[8] = '\0';
  strncpy(address,"127.000.000.001",15);
  address[15] = '\0';

  // extract name and port from name
  char* delimiter = strchr(name, ':');
  if (delimiter) {
    *delimiter = '\0';
    name = strdup(name);
    port = strdup(delimiter + 1);
    *delimiter = ':';
  }
  else {
    name = strdup(name);
    port = NULL;
  }

  // truncate title
  if (strlen(title) > 127)
    title[127] = '\0';

  // replace whitespace in title with spaces
  for (char* scanTitle = title; *scanTitle; ++scanTitle)
    if (isspace(*scanTitle))
      *scanTitle = ' ';

  if (debug >= 1)
    fprintf(stderr, "added server: %s, %s, %s, %s, %s\n", name, build, gameInfo, address, title);
}

Server::~Server()
{
  if (debug >= 1)
    fprintf(stderr, "removed server: %s\n", address);

  free(title);
  free(gameInfo);
  free(build);
  free(name);
  if (port) free(port);

  if (prev) prev->next = next;
  if (next) next->prev = prev;
}

void Server::addAfter(Server* newServer)
{
  assert(newServer->prev == NULL && newServer->next == NULL);
  newServer->prev = this;
  newServer->next = next;
  if (next) next->prev = newServer;
  next = newServer;
}

void Server::ref()
{
  ++refCount;
}

void Server::unref()
{
  --refCount;
  assert(refCount >= 0);
}

void Server::setStale()
{
  if (fresh) {
    unref();
    if (debug >= 1)
      fprintf(stderr, "server %s is stale\n", address);
  }
  fresh = False;
}

void Server::setFresh()
{
  if (!fresh) {
    ref();
    if (debug >= 1)
      fprintf(stderr, "server %s is fresh\n", address);
  }
  time = TimeKeeper::getCurrent();
  fresh = True;
}

void Server::setNumPlayers(int* players)
{
  PingPacket::repackHexPlayerCounts(gameInfo, players);
}

void Server::setTitle(const char* inTitle)
{
	if (title)
		free(title);
	title = strdup(inTitle);
}

void Server::setGameInfo(const char* inGameInfo)
{
	if (gameInfo)
		free(gameInfo);
	gameInfo = strdup(inGameInfo);
}

Server* Server::getNext() const
{
  return next;
}

boolean Server::isReferenced() const
{
  return (refCount > 0);
}

boolean Server::isStale(const TimeKeeper& currentTime) const
{
  return (currentTime - time >= ServerExpiration);
}

boolean Server::isFresh() const
{
  return fresh;
}

const char* Server::getTitle() const
{
  return title;
}

const char* Server::getAddress() const
{
  return address;
}

const char* Server::getVersion() const
{
  return version;
}

const char* Server::getBuild() const
{
  return build;
}

const char* Server::getGameInfo() const
{
  return gameInfo;
}

const char* Server::getName() const
{
  return name;
}

const char* Server::getPort() const
{
  return port;
}


//
// TestServer
//

TestServer::TestServer(int in_fd, Server* inServer) :
				prev(NULL),
				next(NULL),
				fd(in_fd),
				server(inServer),
				wait(False),
				startTime(TimeKeeper::getCurrent())
{
  // do nothing
}

TestServer::~TestServer()
{
  delete server;
  if (fd != -1) close(fd);
  if (prev) prev->next = next;
  if (next) next->prev = prev;
}

void TestServer::addAfter(TestServer* newServer)
{
  assert(newServer->prev == NULL && newServer->next == NULL);
  newServer->prev = this;
  newServer->next = next;
  if (next) next->prev = newServer;
  next = newServer;
}

void TestServer::setWait()
{
  wait = True;
}

boolean TestServer::testWaitAndReset()
{
  if (!wait) return False;
  wait = False;
  return True;
}

Server* TestServer::orphanServer()
{
  Server* tmp = server;
  server = NULL;
  return tmp;
}

TestServer* TestServer::getNext() const
{
  return next;
}

int TestServer::getFD() const
{
  return fd;
}

Server* TestServer::getServer() const
{
  return server;
}

const TimeKeeper& TestServer::getTime() const
{
  return startTime;
}


//
// functions
//

// get the number seconds since the epoch
static time_t getTime()
{
#if defined(_WIN32)
  return time(NULL);
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec;
#endif
}

// print an RFC 1123 compliant data into buffer
static void getRFC1123Date(char* buffer, time_t time)
{
  static const char* wkday[] = { "Sun", "Mon", "Tue",
				 "Wed", "Thu", "Fri", "Sat" };
  static const char* month[] = { "Jan", "Feb", "Mar", "Apr",
				 "May", "Jun", "Jul", "Aug",
				 "Sep", "Oct", "Nov", "Dec" };
  struct tm* tm = gmtime(&time);
  sprintf(buffer, "%s, %02d %s %4d %02d:%02d:%02d GMT",
				wkday[tm->tm_wday],
				tm->tm_mday,
				month[tm->tm_mon],
				tm->tm_year + 1900,
				tm->tm_hour,
				tm->tm_min,
				tm->tm_sec);
}

// find an unused client slot
static int findOpenSlot()
{
  for (int i = 0; i < MaxClients; ++i)
    if (client[i].fd == NotConnected)
      return i;
  return -1;
}

// reference the given server and every server after it in the list
static void refServerList(Server* scan)
{
  while (scan) {
    scan->ref();
    scan = scan->getNext();
  }
}

// unreference the given server and every server after it in the list
static void unrefServerList(Server* scan)
{
  if (scan) scan = scan->getNext();
  while (scan) {
    scan->unref();
    scan = scan->getNext();
  }
}

// accept a connection into a client slot
static void acceptClient(int fd)
{
  // find an open slot
  int index = findOpenSlot();
  assert(index != -1);
  assert(client[index].fd == NotConnected);

  // open socket
  struct sockaddr_in addr;
  AddrLen addr_len = sizeof(addr);
  client[index].fd = accept(fd, (struct sockaddr*)&addr, &addr_len);
  if (client[index].fd == NotConnected) {
    nerror("accepting on wks");
    return;
  }

  // prepare client slot
  client[index].sendingReply = False;
  client[index].offset       = 0;
  client[index].length       = sizeof(client[index].buffer) - 1;
  client[index].nextServer   = NULL;
  client[index].time         = TimeKeeper::getCurrent();

  if (client[index].fd > maxFileDescriptor)
    maxFileDescriptor = client[index].fd;

  if (debug >= 2)
    fprintf(stderr, "accepted from %s on %d\n",
				inet_ntoa(addr.sin_addr), client[index].fd);
}

// close a client connection and free the client slot
static void removeClient(int index)
{
  assert(client[index].fd != NotConnected);

  if (debug >= 2)
    fprintf(stderr, "closing %d\n", client[index].fd);

  // close connection
  close(client[index].fd);

  // unref remaining servers
  unrefServerList(client[index].nextServer);

  // open slot
  client[index].fd = NotConnected;
}

// start the server
static boolean serverStart()
{
#if defined(_WIN32)
  const BOOL optOn = TRUE;
  BOOL opt = optOn;
#else
  const int optOn = 1;
  int opt = optOn;
#endif
  maxFileDescriptor = 0;

  // init addr:port structure
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // look up service name and use that port if no port given on
  // command line.  if no service then use default port.
  addr.sin_port = htons(wksPort);
  if (!useGivenPort) {
    struct servent* service = getservbyname("bzfls", "tcp");
    if (service) {
      addr.sin_port = service->s_port;
    }
  }

  // open well known service port
  wksSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (wksSocket == -1) {
    nerror("couldn't make connect socket");
    return False;
  }
#ifdef SO_REUSEADDR
  /* set reuse address */
  opt = optOn;
  if (setsockopt(wksSocket, SOL_SOCKET, SO_REUSEADDR, (SSOType)&opt, sizeof(opt)) < 0) {
    nerror("serverStart: setsockopt SO_REUSEADDR");
    close(wksSocket);
    return False;
  }
#endif
  if (bind(wksSocket, (const struct sockaddr*)&addr, sizeof(addr)) == -1) {
    nerror("couldn't bind connect socket");
    close(wksSocket);
    return False;
  }

  // listen for connections
  if (listen(wksSocket, 5) == -1) {
    nerror("couldn't make connect socket queue");
    close(wksSocket);
    return False;
  }
  maxFileDescriptor = wksSocket;

  // make server list head
  serverList = new Server("", "", "", "", "");

  // make testing list head
  testingList = new TestServer(-1, NULL);

  // initialize clients
  for (int i = 0; i < MaxClients; ++i)
    client[i].fd = NotConnected;

  // initialize times
  lastChangeTime = getTime();
  return True;
}

// shutdown the server
static void serverStop()
{
  // close listen socket
  close(wksSocket);

  // close client connections
  for (int i = 0; i < MaxClients; ++i)
    if (client[i].fd != NotConnected)
      removeClient(i);

  // clean up database
  Server* scan = serverList;
  while (scan) {
    Server* next = scan->getNext();
    delete scan;
    scan = next;
  }

  // clean up testing list
  TestServer* tscan = testingList;
  while (tscan) {
    TestServer* next = tscan->getNext();
    delete tscan;
    tscan = next;
  }
}

// remove unused server entries in the database and make old entries stale
static void checkList(const TimeKeeper& time)
{
  Server* scan = serverList->getNext();
  while (scan) {
    // get the next server in list
    Server* next = scan->getNext();

    // if server hasn't updated recently then unref it
    if (scan->isStale(time)) {
      scan->setStale();
      lastChangeTime = getTime();
    }

    // if server is not referenced then remove it
    if (!scan->isReferenced()) {
      delete scan;
      lastChangeTime = getTime();
    }

    // next server
    scan = next;
  }
}

// check if server at address is accessible and responsive
static boolean scheduleServerTest(Server* server)
{
  // create socket, make it non-blocking, and start connection
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
    return False;
  if (BzfNetwork::setNonBlocking(fd) < 0) {
    close(fd);
    return False;
  }
  int port = ServerPort;
  if (server->getPort()) {
    port = atoi(server->getPort());
    if (port < 1 || port > 65535)
      return False;
  }
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr = Address::getHostAddress(server->getName());
  sprintf(server->address, "%s", inet_ntoa(addr.sin_addr)); // FIXME this will bomb on ipv6
  if (connect(fd, (CNCTType*)&addr, sizeof(addr)) < 0) {
#if defined(_WIN32)
#undef EINPROGRESS
#define EINPROGRESS EWOULDBLOCK
#endif
    if (getErrno() != EINPROGRESS) {
      // connect failed
      close(fd);
      return False;
    }
    else {
      // connection in progress.  must now wait for connection to
      // complete.
      if (fd > maxFileDescriptor)
	maxFileDescriptor = fd;
      testingList->addAfter(new TestServer(fd, server));
      return True;
    }
  }
  else {
    // connection succeeded immediately
    close(fd);
    serverList->addAfter(server);
    lastChangeTime = getTime();
    return True;
  }
}

// lookup a server in the list by name
static Server* findServer(const char* name)
{
  // search server list for exact same name
  Server* scan = serverList->getNext();
  while (scan && strcmp(scan->getName(), name) != 0)
    scan = scan->getNext();
  return scan;
}

// lookup a server in the list by name
static TestServer* findTestServer(const char* name)
{
  // search server list for exact same name
  TestServer* scan = testingList->getNext();
  while (scan && strcmp(scan->getServer()->getName(), name) != 0)
    scan = scan->getNext();
  return scan;
}

// add a server to the list (or freshen an existing entry)
static int addServer(const char* name, const char* version,
	const char* build, const char* gameInfo, const char* title)
{
  // check format of version
  if (strncmp(version, "BZFS", 4) != 0)
    return 1;

  // is server already in the list?  if so then freshen it.
  Server* exists = findServer(name);
  if (exists) {
  	exists->setTitle(title);
  	exists->setGameInfo(gameInfo);
    exists->setFresh();
    lastChangeTime = getTime();
    return 0;
  }

  // is server already in testing list?  if so then ignore it.
  if (findTestServer(name))
    return 1;

  // create server object but don't add it to our list yet
  Server* server = new Server(name, version, build, gameInfo, title);

  // can we connect to the server?  if not then we reject the request
  // to add the server.  this prevents most cases of inaccessible
  // servers being added to the list.  a typical situation here is a
  // LAN with network address translation (NAT);  the IP address is
  // likely to be 192.168.1.xxx, which is not unique.  another
  // scenario is a server behind a firewall supplying its own
  // address, not the address of the firewall tunnel.  our attempt
  // to connect to a server in these cases will fail.  since we
  // don't want to hang waiting for the server connection, we only
  // start the test process here.
  if (!scheduleServerTest(server)) {
    delete server;
    return 1;
  }
  return 0;
}

// find the end of a request message
static int findMsgEnd(const char* buffer, int, int start)
{
  // find \r\n\r\n or \n\n sequence in buffer.  return -1 if not found,
  // otherwise index of start of match.  start looking around index start.
  start -= 4;
  if (start < 0) start = 0;
  const char* ptr = strstr(buffer + start, "\r\n\r\n");
  if (ptr == NULL)
    ptr = strstr(buffer + start, "\n\n");
  if (ptr == NULL)
    return -1;
  return ptr - buffer;
}

// parse a whitespace separated string into tokens (modifying input string).
// parse up to argc-1 tokens and remaining tokens go into last argument.
// return False if insufficient tokens found (last argument can be empty),
// else return True.
static boolean parseRequest(char* buffer, char** args, int argc,
				boolean lastEmpty = False)
{
  // string must not be empty or have leading whitespace
  if (*buffer == '\0' || isspace(*buffer))
    return False;

  // parse
  int i = 0;
  while (i < argc - 1) {
    // save next argument
    args[i++] = buffer;

    // skip argument
    while (!isspace(*buffer) && *buffer != '\0')
      ++buffer;

    // failed if we reached end of string and not at last argument.
    if (*buffer == '\0')
      if (i == argc)
	return True;
      else if (!lastEmpty || i < argc - 1)
	return False;
      else
	break;

    // delimit argument
    *buffer++ = '\0';

    // skip whitespace
    while (isspace(*buffer) && *buffer != '\0')
      ++buffer;

    if (*buffer == '\0')
     return False;
  }

  // rest goes into last argument
  args[i] = buffer;
  return True;
}

// continue sending reply
static boolean continueReplyClient(int index)
{
  // go to the next fresh server in the list
  Server* server = client[index].nextServer;
  while (server != NULL) {
    server = server->getNext();
    if (server != NULL) {
      server->unref();
      if (server->isFresh())
	break;
    }
  }
  client[index].nextServer = server;

  // done if no more servers
  if (server == NULL)
    return False;

  // print server info into client buffer
  sprintf(client[index].buffer, "%s %s %s %s %s\r\n",
				server->getName(),
				server->getVersion(),
				server->getGameInfo(),
				server->getAddress(),
				server->getTitle());
  client[index].offset = 0;
  client[index].length = strlen(client[index].buffer);

  return True;
}

// parse a request and begin reply
static boolean startReplyClient(int index)
{
  // now replying
  client[index].sendingReply = True;

  // get request
  char* request = (char*)client[index].buffer;

  // parse request to get command
  char* args[NumTeams + 5];
  if (!parseRequest(request, args, 2, True))
    return False;

  // get command into cmd and remaining arguments into request
  char* cmd = args[0];
  request = args[1];
  if (debug >= 4)
    fprintf(stderr, "command %s on %d;  args: %s\n",
				cmd, client[index].fd, request);

  // parse request
  if (strcmp(cmd, "ADD") == 0) {
    numAdds++;
    numAddsTotal++;

    // looks like a server requesting to add itself
    // ADD <name> <version> <build> <gameinfo> <title>
    if (!parseRequest(request, args, 5)) {
      numAddsBad++;
      return False;
    }

    client[index].buffer[0] = (char)addServer(args[0], args[2],
						args[1], args[3], args[4]);
    client[index].offset = 0;
    client[index].length = 1;
    return True;
  }

  else if (strcmp(cmd, "REMOVE") == 0) {
    numRemoves++;
    numRemovesTotal++;

    // looks like a server requesting to remove itself
    // REMOVE <name>
    args[0] = request;

    // lookup server in list.  if there, make it stale
    Server* server = findServer(args[0]);
    if (server != NULL) {
      server->setStale();
      lastChangeTime = getTime();

      // okay
      client[index].buffer[0] = 0;
    }
    else {
      // not on list
      client[index].buffer[0] = 1;
      numRemovesBad++;
    }

    client[index].offset = 0;
    client[index].length = 1;
    return True;
  }

  else if (strcmp(cmd, "SETNUM") == 0) {
    numSets++;
    numSetsTotal++;

    // looks like a server updating its player count
    // SETNUM <address> <rogue-count> <r-count> <g-count> <b-count> <p-count>
    if (!parseRequest(request, args, NumTeams + 1)) {
      numSetsBad++;
      return False;
    }

    // lookup server
    Server* server = findServer(args[0]);
    if (server != NULL) {
      int players[NumTeams];
      for (int i = 0; i < NumTeams; ++i)
	players[i] = atoi(args[i + 1]);
      server->setNumPlayers(players);
      server->setFresh();
      lastChangeTime = getTime();

      // okay
      client[index].buffer[0] = 0;
    }
    else {
      // not in list
      client[index].buffer[0] = 1;
      numSetsBad++;
    }

    client[index].offset = 0;
    client[index].length = 1;
    return True;
  }

  else if (strcmp(cmd, "LIST") == 0) {
    numLists++;
    numListsTotal++;

    // looks like a player requesting a server list
    // LIST
    unrefServerList(client[index].nextServer);
    client[index].nextServer = serverList;
    refServerList(client[index].nextServer->getNext());
    return continueReplyClient(index);
  }

  else if (strcmp(cmd, "GET") == 0) {
    numGets++;
    numGetsTotal++;

    // looks like a browser requesting a server list.  act like an HTTP
    // server.
    // GET <url> HTTP/*
    if (!parseRequest(request, args, 2)) {
      numGetsBad++;
      return False;
    }

    // ignore all HTTP headers (which are in args[2]).  print HTTP
    // reply headers into buffer.
    char date[30], modified[30];
    getRFC1123Date(date, getTime());
    getRFC1123Date(modified, lastChangeTime);
    sprintf(client[index].buffer,
				"HTTP/1.1 200 OK\r\n"
				"Date: %s\r\n"
				"Server: bzfls/%d.%d%c.%d\r\n"
				"Last-Modified: %s\r\n"
				"Connection: close\r\n"
				"Content-Type: text/plain\r\n"
				"\r\n",
				date,
				(VERSION / 10000000) % 100,
				(VERSION / 100000) % 100,
				(char)('a' - 1 + (VERSION / 1000) % 100),
				VERSION % 1000,
				modified);
    client[index].offset = 0;
    client[index].length = strlen(client[index].buffer);

    // schedule list to be sent
    unrefServerList(client[index].nextServer);
    client[index].nextServer = serverList;
    refServerList(client[index].nextServer->getNext());

    return True;
  }
  else {
    numBad++;
    numBadTotal++;
  }

  // unrecognized
  return False;
}

static void readClient(int index, const TimeKeeper& tm)
{
  assert(client[index].fd != NotConnected);
  int length = recv(client[index].fd,
				client[index].buffer + client[index].offset,
				client[index].length - client[index].offset, 0);


  // read successful;  check for buffer full or end of message
  if (length > 0) {
    bytesRead += length;
    bytesReadTotal += length;

    // update activity time
    client[index].time = tm;

    // look for end of message
    client[index].buffer[client[index].offset + length] = '\0';
    int msgEnd = findMsgEnd(client[index].buffer,
				client[index].offset + length,
				client[index].offset);

    // account for new data
    client[index].offset += length;

    // no end yet.  if we've filled our input buffer then hangup.
    if (msgEnd == -1) {
      if (client[index].offset == client[index].length)
	removeClient(index);
    }

    // message is complete.  truncate to EOM delimiter and initiate response.
    else {
      client[index].buffer[msgEnd] = '\0';
      if (!startReplyClient(index))
	removeClient(index);
    }
  }

  // client hungup
  else if (length == 0) {
    removeClient(index);
  }

  // read failed.  why?
  else if (length < 0) {
    if (getErrno() != EINTR)
      removeClient(index);
  }
}

static void writeClient(int index, const TimeKeeper& tm)
{
  // write more data
  int length = send(client[index].fd,
				client[index].buffer + client[index].offset,
				client[index].length - client[index].offset, 0);

  // write was successful.  record what was sent.  close connection if
  // nothing left to send.
  if (length > 0) {
    bytesWritten += length;
    bytesWrittenTotal += length;

    // update activity time
    client[index].time = tm;

    // account for data
    client[index].offset += length;

    // all done with this portion.  get next portion.
    if (client[index].offset == client[index].length)
      if (!continueReplyClient(index))
	removeClient(index);
  }

  // send failed.  check why.
  else if (length < 0) {
    if (getErrno() != EINTR)
      removeClient(index);
  }
}

static void dumpServerList(int /*sig*/)
{
#if !defined(_WIN32)				// no SIGUSR1 in Windows
  signal(SIGUSR1, SIG_PF(dumpServerList));
#endif

  // open a file to receive server list
  FILE* file = fopen("/tmp/bzfls.lst", "w");
  if (file == NULL)
    return;

  // dump every fresh server to the file
  Server* server = serverList;
  while (server != NULL) {
    server = server->getNext();
    if (server != NULL && server->isFresh()) {
      fprintf(file, "%s %s %s %s %s\r\n",
				server->getName(),
				server->getBuild(),
				server->getVersion(),
				server->getGameInfo(),
				server->getTitle());
    }
  }

  fclose(file);
}

static void dumpTraffic(int /*sig*/)
{
#if !defined(_WIN32)				// no SIGUSR2 in Windows
  signal(SIGUSR2, SIG_PF(dumpTraffic));
#endif

  // open a file to receive traffic counts
  FILE* file = fopen("/tmp/bzfls.cnt", "w");
  if (file == NULL)
    return;

  TimeKeeper t = TimeKeeper::getCurrent();
  fprintf(file, "                incremental  total        bad (total)\n");
  fprintf(file, "time:           %-10d   %-10d\n",
				(int)(t - lastDumpTime), (int)(t - startTime));
  fprintf(file, "bytes read:     %-10d   %-10d\n",
				bytesRead, bytesReadTotal);
  fprintf(file, "bytes written:  %-10d   %-10d\n",
				bytesWritten, bytesWrittenTotal);
  fprintf(file, "ADD:            %-10d   %-10d   %-10d\n",
				numAdds, numAddsTotal, numAddsBad);
  fprintf(file, "REMOVE:         %-10d   %-10d   %-10d\n",
				numRemoves, numRemovesTotal, numRemovesBad);
  fprintf(file, "SETNUM:         %-10d   %-10d   %-10d\n",
				numSets, numSetsTotal, numSetsBad);
  fprintf(file, "GET:            %-10d   %-10d   %-10d\n",
				numGets, numGetsTotal, numGetsBad);
  fprintf(file, "LIST:           %-10d   %-10d\n",
				numLists, numListsTotal);
  fprintf(file, "bad requests:   %-10d   %-10d\n",
				numBad, numBadTotal);
  fclose(file);

  bytesRead = 0;
  bytesWritten = 0;
  numAdds = 0;
  numRemoves = 0;
  numSets = 0;
  numGets = 0;
  numLists = 0;
  numBad = 0;
  lastDumpTime = t;
}

static void bootstrapList(int argc, char** argv)
{
  int i;
  FILE* file;
  boolean stdinDone = False;
  char buffer[MaxInputMsgSize];
  char* args[5];

  // do all files
  for (i = 0; i < argc; ++i) {
    // open file
    if (strcmp(argv[i], "-") == 0) {
      if (stdinDone)
	continue;
      file = stdin;
      stdinDone = True;
    }
    else {
      file = fopen(argv[i], "r");
      if (file == NULL) {
	fprintf(stderr, "can't open file: %s\n", argv[i]);
	continue;
      }
    }

    // read each line and add server.  could use more error checking here.
    while (fgets(buffer, sizeof(buffer), file)) {
      if (parseRequest(buffer, args, 5))
	addServer(args[0], args[2], args[1], args[3], args[4]);
    }

    // close file
    if (file != stdin)
      fclose(file);
  }
}

static int exitCode = 0;
static boolean done = False;

static void terminateServer(int /*sig*/)
{
  signal(SIGINT, SIG_PF(terminateServer));
  signal(SIGTERM, SIG_PF(terminateServer));
  exitCode = 0;
  done = True;
}

static const char* usageString =
"[-p <port>] [-version] [<server-list-file> ...]";

static void printVersion(FILE* out)
{
  fprintf(out, "%s\n", copyright);

  fprintf(out, "BZFLAG server, version %d.%d%c%d\n",
		(VERSION / 10000000) % 100,
		(VERSION / 100000) % 100,
		(char)('a' - 1 + (VERSION / 1000) % 100),
		VERSION % 1000);

  fprintf(out, "  protocol %c.%d%c\n", ServerVersion[4],
				(ServerVersion[5] != '0') ?
					atoi(ServerVersion + 5) :
					atoi(ServerVersion + 6),
				(char)tolower(ServerVersion[7]));
}

static void usage(const char* pname)
{
  printVersion(stderr);
  fprintf(stderr, "usage: %s %s\n", pname, usageString);
  exit(1);
}

static int parse(int argc, char** argv)
{
  // parse command line
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-help") == 0) {
      usage(argv[0]);
    }
    else if (strncmp(argv[i], "-d", 2) == 0) {
      int count = 0;
      char* scan;
      for (scan = argv[i]+1; *scan == 'd'; scan++) count++;
      if (*scan != '\0') {
	fprintf(stderr, "bad argument %s\n", argv[i]);
	usage(argv[0]);
      }
      debug += count;
    }
    else if (strcmp(argv[i], "-p") == 0) {
      // use a different port
      if (++i == argc) {
	fprintf(stderr, "argument expected for -p\n");
	usage(argv[0]);
      }
      wksPort = atoi(argv[i]);
      if (wksPort < 1 || wksPort > 65535) wksPort = ServerPort;
      else useGivenPort = True;
    }
    else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-version") == 0) {
      printVersion(stdout);
      exit(0);
    }
    else {
      // remaining arguments are files to bootstrap server list from
      return i;
    }
  }
  return argc;
}

int main(int argc, char** argv)
{
  // check time bomb
  if (timeBombBoom()) {
    fprintf(stderr, "This release expired on %s.\n", timeBombString());
    fprintf(stderr, "Please upgrade to the latest release.\n");
    exit(0);
  }

  // print expiration date
  if (timeBombString()) {
    fprintf(stderr, "This release will expire on %s.\n", timeBombString());
    fprintf(stderr, "Version %d.%d%c%d\n",
		(VERSION / 10000000) % 100, (VERSION / 100000) % 100,
		(char)('a' - 1 + (VERSION / 1000) % 100), VERSION % 1000);
  }

  // trap some signals
  if (signal(SIGINT, SIG_IGN) != SIG_IGN)	// let user kill server
    signal(SIGINT, SIG_PF(terminateServer));
  signal(SIGTERM, SIG_PF(terminateServer));	// ditto
#if !defined(_WIN32)				// these signals not in Win32
  signal(SIGPIPE, SIG_IGN);			// don't die on broken pipe
  signal(SIGUSR1, SIG_PF(dumpServerList));
  signal(SIGUSR2, SIG_PF(dumpTraffic));
#endif

  // initialize
#if defined(_WIN32)
  {
    static const int major = 2, minor = 2;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(major, minor), &wsaData)) {
      fprintf(stderr, "Failed to initialize winsock.  Terminating.\n");
      return 1;
    }
    if (LOBYTE(wsaData.wVersion) != major ||
	HIBYTE(wsaData.wVersion) != minor) {
      fprintf(stderr, "Version mismatch in winsock;"
			"  got %d.%d.  Terminating.\n",
			(int)LOBYTE(wsaData.wVersion),
			(int)HIBYTE(wsaData.wVersion));
      WSACleanup();
      return 1;
    }
  }
#endif /* defined(_WIN32) */

  int fileArg = parse(argc, argv);

  // start listening and prepare world database
  if (!serverStart()) {
#if defined(_WIN32)
    WSACleanup();
#endif /* defined(_WIN32) */
    return 1;
  }

  // read and add servers from files
  bootstrapList(argc - fileArg, argv + fileArg);

  // we purge the list of unresponsive servers every CheckListInterval
  // seconds.  purging keeps our database from getting filled with
  // servers that didn't say goodbye.
  TimeKeeper nextCheckListTime = TimeKeeper::getCurrent();
  nextCheckListTime += CheckListInterval;

  // note time for statistics
  lastDumpTime = TimeKeeper::getCurrent();
  startTime = lastDumpTime;

  int i;
  while (!done) {
    // prepare select sets
    fd_set read_set;
    fd_set write_set;
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    if (findOpenSlot() != -1)
      FD_SET(wksSocket, &read_set);	// listen for connections if slot open
    for (i = 0; i < MaxClients; i++)	// add each connected client
      if (client[i].fd != NotConnected) {
	// if we're sending a reply then wait for socket to be
	// writable.  otherwise wait for data from the client.
	if (client[i].sendingReply && client[i].offset < client[i].length)
	  FD_SET(client[i].fd, &write_set);
	else if (!client[i].sendingReply)
	  FD_SET(client[i].fd, &read_set);
      }
    TestServer* scan = testingList->getNext();
    while (scan) {
      if (!scan->testWaitAndReset())
	FD_SET(scan->getFD(), &write_set);
      scan = scan->getNext();
    }

    // wait for communication or 5 seconds
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    int nfound = select(maxFileDescriptor+1,
			(fd_set*)&read_set, (fd_set*)&write_set, 0, &timeout);

    // get time now
    TimeKeeper tm = TimeKeeper::getCurrent();

    // check messages
    if (nfound >= 0) {
      // first check for initial contacts
      if (FD_ISSET(wksSocket, &read_set))
	acceptClient(wksSocket);

      // send/receive messages
      for (i = 0; i < MaxClients; i++) {
	if (client[i].fd != NotConnected && FD_ISSET(client[i].fd, &read_set))
	  readClient(i, tm);
	if (client[i].fd != NotConnected && FD_ISSET(client[i].fd, &write_set))
	  writeClient(i, tm);
      }

      // check for complete test connection to server
      scan = testingList->getNext();
      while (scan) {
	TestServer* next = scan->getNext();
	if (FD_ISSET(scan->getFD(), &write_set)) {
	  // connection completed or timed out.  try reading one byte.
	  // if there's an error or zero bytes read then assume we
	  // cannot connect to the server, unless the error is EAGAIN.
	  // in that case just wait for data.
	  char byte;
	  int n = recv(scan->getFD(), &byte, 1, 0);
	  if (n <= 0) {
	    if (n == 0 || getErrno() != EAGAIN)
	      delete scan;
	    else
	      scan->setWait();
	  }
	  else {
	    // success!  add to list
	    serverList->addAfter(scan->orphanServer());
	    lastChangeTime = getTime();
	    delete scan;
	  }
	}
	scan = next;
      }
    }

    // select failed.  avoid spinning CPU.
    else if (nfound < 0) {
      if (getErrno() != EINTR)
	sleep(1);
    }

    // timeout connections that aren't active.  we don't have many client
    // slots because clients don't need to stay connected for very long.
    // the timeout forces connections to be brief.
    for (i = 0; i < MaxClients; i++)
      if (client[i].fd != NotConnected &&
	  (tm - client[i].time) > DisconnectTimeout)
	removeClient(i);

    // throw out pending servers that aren't responding
    scan = testingList->getNext();
    while (scan) {
      TestServer* next = scan->getNext();
      if (tm - scan->getTime() > DisconnectTimeout)
	delete scan;
      scan = next;
    }

    // clean the list if we've reached the check time
    if (tm - nextCheckListTime >= 0.0) {
      checkList(tm);
      nextCheckListTime  = tm;
      nextCheckListTime += CheckListInterval;
    }
  }

  serverStop();

  // clean up
#if defined(_WIN32)
  WSACleanup();
#endif /* defined(_WIN32) */

  // done
  return exitCode;
}
