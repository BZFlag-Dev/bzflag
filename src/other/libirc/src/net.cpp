/* libIRC
* Copyright (c) 2004 Christopher Sean Morrison
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named LICENSE that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

// code yanked directly from nET
// Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga

/* Since the UNIX/Win32/BeOS code is so different from MacOS,
we'll just have two completely different sections here.
*/

#include "net.h"
#include <string>

static int net_started = 0;

// global error stuff
std::string globalErrorString;

void net_SetError(const char *error)
{
	globalErrorString = error;
}

const char* net_GetError(void)
{
	return globalErrorString.c_str();
}

void net_ClearError(void)
{
	globalErrorString = "";
}

/* Initialize/Cleanup the network API */
int  net_Init(void)
{
	if ( !net_started ) {
#ifdef __USE_W32_SOCKETS
		/* Start up the windows networking */
		WORD version_wanted = MAKEWORD(1,1);
		WSADATA wsaData;

		if ( WSAStartup(version_wanted, &wsaData) != 0 ) {
			net_SetError("Couldn't initialize Winsock 1.1\n");
			return(-1);
		}
#else
		;
#endif
	}
	++net_started;
	return(0);
}
void net_Quit(void)
{
	if ( net_started == 0 ) {
		return;
	}
	if ( net_started-- == 0 ) {
#ifdef __USE_W32_SOCKETS
		/* Clean up windows networking */
		if ( WSACleanup() == SOCKET_ERROR ) {
			if ( WSAGetLastError() == WSAEINPROGRESS ) {
				WSACancelBlockingCall();
				WSACleanup();
			}
		}
#else
		;
#endif
	}
}

/* Resolve a host name and port to an IP address in network form */
int net_ResolveHost(IPaddress *address, const char *host, unsigned short port)
{
	int retval = 0;

	/* Perform the actual host resolution */
	if ( host == NULL ) {
		address->host = INADDR_ANY;
	} else {
		address->host = inet_addr(host);
		if ( address->host == INADDR_NONE ) {
			struct hostent *hp;

			hp = gethostbyname(host);
			if ( hp ) {
				memcpy(&address->host,hp->h_addr,hp->h_length);
			} else {
				retval = -1;
			}
		}
	}
	address->port = NET_SwapBE16(port);

	/* Return the status */
	return(retval);
}

/* Resolve an ip address to a host name in canonical form.
If the ip couldn't be resolved, this function returns NULL,
otherwise a pointer to a static buffer containing the hostname
is returned.  Note that this function is not thread-safe.
*/
/* Written by Miguel Angel Blanch.
* Main Programmer of Arianne RPG.
* http://come.to/arianne_rpg
*/
const char *net_ResolveIP(IPaddress *ip)
{
	struct hostent *hp;

	hp = gethostbyaddr((char *)&ip->host, 4, AF_INET);
	if ( hp != NULL ) {
		return hp->h_name;
	}
	return NULL;
}

struct net_Socket {
	int ready;
	SOCKET channel;
};

struct _net_SocketSet {
	int numsockets;
	int maxsockets;
	struct net_Socket **sockets;
};

/* Allocate a socket set for use with net_CheckSockets() 
This returns a socket set for up to 'maxsockets' sockets, or NULL if 
the function ran out of memory. 
*/
net_SocketSet net_AllocSocketSet(int maxsockets)
{
	struct _net_SocketSet *set;
	int i;

	set = (struct _net_SocketSet *)malloc(sizeof(*set));
	if ( set != NULL ) {
		set->numsockets = 0;
		set->maxsockets = maxsockets;
		set->sockets = (struct net_Socket **)malloc
			(maxsockets*sizeof(*set->sockets));
		if ( set->sockets != NULL ) {
			for ( i=0; i<maxsockets; ++i ) {
				set->sockets[i] = NULL;
			}
		} else {
			free(set);
			set = NULL;
		}
	}
	return(set);
}

/* Add a socket to a set of sockets to be checked for available data */
int net_AddSocket(net_SocketSet set, net_GenericSocket sock)
{
	if ( sock != NULL ) {
		if ( set->numsockets == set->maxsockets ) {
			net_SetError("socketset is full");
			return(-1);
		}
		set->sockets[set->numsockets++] = (struct net_Socket *)sock;
	}
	return(set->numsockets);
}

/* Remove a socket from a set of sockets to be checked for available data */
int net_DelSocket(net_SocketSet set, net_GenericSocket sock)
{
	int i;

	if ( sock != NULL ) {
		for ( i=0; i<set->numsockets; ++i ) {
			if ( set->sockets[i] == (struct net_Socket *)sock ) {
				break;
			}
		}
		if ( i == set->numsockets ) {
			net_SetError("socket not found in socketset");
			return(-1);
		}
		--set->numsockets;
		for ( ; i<set->numsockets; ++i ) {
			set->sockets[i] = set->sockets[i+1];
		}
	}
	return(set->numsockets);
}

/* This function checks to see if data is available for reading on the
given set of sockets.  If 'timeout' is 0, it performs a quick poll,
otherwise the function returns when either data is available for
reading, or the timeout in milliseconds has elapsed, which ever occurs
first.  This function returns the number of sockets ready for reading,
or -1 if there was an error with the select() system call.
*/
int net_CheckSockets(net_SocketSet set, unsigned int timeout)
{
	int i;
	SOCKET maxfd;
	int retval;
	struct timeval tv;
	fd_set mask;

	/* Find the largest file descriptor */
	maxfd = 0;
	for ( i=set->numsockets-1; i>=0; --i ) {
		if ( set->sockets[i]->channel > maxfd ) {
			maxfd = set->sockets[i]->channel;
		}
	}

	/* Check the file descriptors for available data */
	do {
		errno = 0;

		/* Set up the mask of file descriptors */
		FD_ZERO(&mask);
		for ( i=set->numsockets-1; i>=0; --i ) {
			FD_SET(set->sockets[i]->channel, &mask);
		}

		/* Set up the timeout */
		tv.tv_sec = timeout/1000;
		tv.tv_usec = (timeout%1000)*1000;

		/* Look! */
		retval = select((int)(maxfd+1), &mask, NULL, NULL, &tv);
	} while ( errno == EINTR );

	/* Mark all file descriptors ready that have data available */
	if ( retval > 0 ) {
		for ( i=set->numsockets-1; i>=0; --i ) {
			if ( FD_ISSET(set->sockets[i]->channel, &mask) ) {
				set->sockets[i]->ready = 1;
			}
		}
	}
	return(retval);
}

/* Free a set of sockets allocated by SDL_NetAllocSocketSet() */
extern void net_FreeSocketSet(net_SocketSet set)
{
	if ( set ) {
		free(set->sockets);
		free(set);
	}
}


struct _TCPsocket {
	int ready;
	SOCKET channel;
	IPaddress remoteAddress;
	IPaddress localAddress;
	int sflag;
};

/* Open a TCP network socket
If 'remote' is NULL, this creates a local server socket on the given port,
otherwise a TCP connection to the remote host and port is attempted.
The newly created socket is returned, or NULL if there was an error.
*/
TCPsocket net_TCP_Open(IPaddress *ip)
{
	TCPsocket sock;
	struct sockaddr_in sock_addr;

	/* Allocate a TCP socket structure */
	sock = (TCPsocket)malloc(sizeof(*sock));
	if ( sock == NULL ) {
		net_SetError("Out of memory");
		goto error_return;
	}

	/* Open the socket */
	sock->channel = socket(AF_INET, SOCK_STREAM, 0);
	if ( sock->channel == INVALID_SOCKET ) {
		net_SetError("Couldn't create socket");
		goto error_return;
	}

	/* Connect to remote, or bind locally, as appropriate */
	if ( (ip->host != INADDR_NONE) && (ip->host != INADDR_ANY) ) {

		// #########  Connecting to remote

		memset(&sock_addr, 0, sizeof(sock_addr));
		sock_addr.sin_family = AF_INET;
		sock_addr.sin_addr.s_addr = ip->host;
		sock_addr.sin_port = ip->port;

		/* Connect to the remote host */
		if ( connect(sock->channel, (struct sockaddr *)&sock_addr,
			sizeof(sock_addr)) == SOCKET_ERROR ) {
				net_SetError("Couldn't connect to remote host");
				goto error_return;
			}
			sock->sflag = 0;
	} else {

		// ##########  Binding locally

		memset(&sock_addr, 0, sizeof(sock_addr));
		sock_addr.sin_family = AF_INET;
		sock_addr.sin_addr.s_addr = INADDR_ANY;
		sock_addr.sin_port = ip->port;

		/* allow local address reuse */
		{ int yes = 1;
		setsockopt(sock->channel, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes));
		}

		/* Bind the socket for listening */
		if ( bind(sock->channel, (struct sockaddr *)&sock_addr,
			sizeof(sock_addr)) == SOCKET_ERROR ) {
				net_SetError("Couldn't bind to local port");
				goto error_return;
			}
			if ( listen(sock->channel, 5) == SOCKET_ERROR ) {
				net_SetError("Couldn't listen to local port");
				goto error_return;
			}
#ifdef O_NONBLOCK
			/* Set the socket to non-blocking mode for accept() */
			fcntl(sock->channel, F_SETFL, O_NONBLOCK);
#else
#ifdef WIN32
			{
				/* passing a non-zero value, socket mode set non-blocking */
				unsigned long mode = 1;
				ioctlsocket (sock->channel, FIONBIO, &mode);
			}
#else
#warning How do we set non-blocking mode on other operating systems?
#endif /* WIN32 */
#endif /* O_NONBLOCK */

			sock->sflag = 1;
	}
	sock->ready = 0;

#ifdef TCP_NODELAY
	/* Set the nodelay TCP option for real-time games */
	{ int yes = 1;
	setsockopt(sock->channel, IPPROTO_TCP, TCP_NODELAY, (char*)&yes, sizeof(yes));
	}
#endif /* TCP_NODELAY */

	/* Fill in the channel host address */
	sock->remoteAddress.host = sock_addr.sin_addr.s_addr;
	sock->remoteAddress.port = sock_addr.sin_port;

	/* The socket is ready */
	return(sock);

error_return:
	net_TCP_Close(sock);
	return(NULL);
}

/* Accept an incoming connection on the given server socket.
The newly created socket is returned, or NULL if there was an error.
*/
TCPsocket net_TCP_Accept(TCPsocket server)
{
	TCPsocket sock;
	struct sockaddr_in sock_addr;
	socklen_t sock_alen;

	/* Only server sockets can accept */
	if ( ! server->sflag ) {
		net_SetError("Only server sockets can accept()");
		return(NULL);
	}
	server->ready = 0;

	/* Allocate a TCP socket structure */
	sock = (TCPsocket)malloc(sizeof(*sock));
	if ( sock == NULL ) {
		net_SetError("Out of memory");
		goto error_return;
	}

	/* Accept a new TCP connection on a server socket */
	sock_alen = sizeof(sock_addr);
	sock->channel = accept(server->channel, (struct sockaddr *)&sock_addr,
#ifdef USE_GUSI_SOCKETS
		(unsigned int *)&sock_alen);
#else
		&sock_alen);
#endif
	if ( sock->channel == SOCKET_ERROR ) {
		net_SetError("accept() failed");
		goto error_return;
	}
#ifdef WIN32
	{
		/* passing a zero value, socket mode set to block on */
		unsigned long mode = 0;
		ioctlsocket (sock->channel, FIONBIO, &mode);
	}
#endif /* WIN32 */
	sock->remoteAddress.host = sock_addr.sin_addr.s_addr;
	sock->remoteAddress.port = sock_addr.sin_port;

	sock->sflag = 0;
	sock->ready = 0;

	/* The socket is ready */
	return(sock);

error_return:
	net_TCP_Close(sock);
	return(NULL);
}

/* Get the IP address of the remote system associated with the socket.
If the socket is a server socket, this function returns NULL.
*/
IPaddress *net_TCP_GetPeerAddress(TCPsocket sock)
{
	if ( sock->sflag ) {
		return(NULL);
	}
	return(&sock->remoteAddress);
}

/* Send 'len' bytes of 'data' over the non-server socket 'sock'
This function returns the actual amount of data sent.  If the return value
is less than the amount of data sent, then either the remote connection was
closed, or an unknown socket error occurred.
*/
int net_TCP_Send(TCPsocket sock, void *datap, int len)
{
	unsigned char *data = (unsigned char *)datap;	/* For pointer arithmetic */
	int sent, left;

	/* Server sockets are for accepting connections only */
	if ( sock->sflag ) {
		net_SetError("Server sockets cannot send");
		return(-1);
	}

	/* Keep sending data until it's sent or an error occurs */
	left = len;
	sent = 0;
	errno = 0;
	do {
		len = send(sock->channel, (const char *) data, left, 0);
		if ( len > 0 ) {
			sent += len;
			left -= len;
			data += len;
		}
	} while ( (left > 0) && ((len > 0) || (errno == EINTR)) );

	return(sent);
}

/* Receive up to 'maxlen' bytes of data over the non-server socket 'sock',
and store them in the buffer pointed to by 'data'.
This function returns the actual amount of data received.  If the return
value is less than or equal to zero, then either the remote connection was
closed, or an unknown socket error occurred.
*/
int net_TCP_Recv(TCPsocket sock, void *data, int maxlen)
{
	int len;

	/* Server sockets are for accepting connections only */
	if ( sock->sflag ) {
		net_SetError("Server sockets cannot receive");
		return(-1);
	}

	errno = 0;
	do {
		len = recv(sock->channel, (char *) data, maxlen, 0);
	} while ( errno == EINTR );

	sock->ready = 0;
	return(len);
}

/* Close a TCP network socket */
void net_TCP_Close(TCPsocket sock)
{
	if ( sock != NULL ) {
		if ( sock->channel != INVALID_SOCKET ) {
			closesocket(sock->channel);
		}
		free(sock);
	}
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8