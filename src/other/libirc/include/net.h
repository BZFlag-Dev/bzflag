// code yanked directly from SDLNET
// Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga

#ifndef _NETWORKING_H_
#define _NETWORKING_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// endian action

// byte order setuff from SDLNET_BYTEORDER.h
/* Macros for determining the byte-order of this platform */

/* The two types of endianness */
#define NET_LIL_ENDIAN	1234
#define NET_BIG_ENDIAN	4321

/* Pardon the mess, I'm trying to determine the endianness of this host.
I'm doing it by preprocessor defines rather than some sort of configure
script so that application code can use this too.  The "right" way would
be to dynamically generate this file on install, but that's a lot of work.
*/
#if  defined(__i386__) || defined(__ia64__) || defined(WIN32) || \
	(defined(__alpha__) || defined(__alpha)) || \
	defined(__arm__) || \
	(defined(__mips__) && defined(__MIPSEL__)) || \
	defined(__SYMBIAN32__) || \
	defined(__x86_64__) || \
	defined(__LITTLE_ENDIAN__)
#define NET_BYTEORDER	NET_LIL_ENDIAN
#else
#define NET_BYTEORDER	NET_BIG_ENDIAN
#endif

/* The macros used to swap values */
/* Try to use superfast macros on systems that support them */
#ifdef linux
#include <asm/byteorder.h>
#ifdef __arch__swab16
#define NET_Swap16  __arch__swab16
#endif
#ifdef __arch__swab32
#define NET_Swap32  __arch__swab32
#endif
#endif /* linux */
/* Use inline functions for compilers that support them, and static
functions for those that do not.  Because these functions become
static for compilers that do not support inline functions, this
header should only be included in files that actually use them.
*/
#ifndef NET_Swap16
static inline unsigned short NET_Swap16(unsigned short D) {
	return((D<<8)|(D>>8));
}
#endif
#ifndef NET_Swap32
static inline unsigned int NET_Swap32(unsigned int D) {
	return((D<<24)|((D<<8)&0x00FF0000)|((D>>8)&0x0000FF00)|(D>>24));
}
#endif
#ifdef NET_HAS_64BIT_TYPE
#ifndef NET_Swap64
static inline long long SDLNET_Swap64(long long val) {
	Uint32 hi, lo;

	/* Separate into high and low 32-bit values and swap them */
	lo = (Uint32)(val&0xFFFFFFFF);
	val >>= 32;
	hi = (Uint32)(val&0xFFFFFFFF);
	val = NET_Swap32(lo);
	val <<= 32;
	val |= NET_Swap32(hi);
	return(val);
}
#endif
#else
#ifndef NET_Swap64
/* This is mainly to keep compilers from complaining in code.
If there is no real 64-bit datatype, then compilers will complain about
the fake 64-bit datatype that SDL provides when it compiles user code.
*/
#define NET_Swap64(X)	(X)
#endif
#endif /* NET_HAS_64BIT_TYPE */


/* Byteswap item from the specified endianness to the native endianness */
#if NET_BYTEORDER == NET_LIL_ENDIAN
#define NET_SwapLE16(X)	(X)
#define NET_SwapLE32(X)	(X)
#define NET_SwapLE64(X)	(X)
#define NET_SwapBE16(X)	NET_Swap16(X)
#define NET_SwapBE32(X)	NET_Swap32(X)
#define NET_SwapBE64(X)	NET_Swap64(X)
#else
#define NET_SwapLE16(X)	NET_Swap16(X)
#define NET_SwapLE32(X)	NET_Swap32(X)
#define NET_SwapLE64(X)	NET_Swap64(X)
#define NET_SwapBE16(X)	(X)
#define NET_SwapBE32(X)	(X)
#define NET_SwapBE64(X)	(X)
#endif

/* Include system network headers */
#if defined(__WIN32__) || defined(WIN32)
	#define __USE_W32_SOCKETS
	#include <windows.h>
#else /* UNIX */
	#include <sys/time.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <netinet/in.h>
	#ifndef __BEOS__
		#include <arpa/inet.h>
	#endif
	#ifdef linux /* FIXME: what other platforms have this? */
		#include <netinet/tcp.h>
	#endif
	#include <netdb.h>
	#include <sys/socket.h>
#endif /* WIN32 */

/* System-dependent definitions */
#ifndef __USE_W32_SOCKETS
	#define closesocket	close
	#define SOCKET	int
	#define INVALID_SOCKET	-1
	#define SOCKET_ERROR	-1
#endif /* __USE_W32_SOCKETS */

#ifndef INADDR_ANY
#define INADDR_ANY		0x00000000
#endif

#ifndef INADDR_NONE
#define INADDR_NONE		0xFFFFFFFF
#endif

#ifndef INADDR_BROADCAST
#define INADDR_BROADCAST	0xFFFFFFFF
#endif

// SDL error stuff form SDL_error.h
void net_SetError(const char *error);
const char* net_GetError(void);
void net_ClearError(void);

//-------------Init and cleanup
int net_Init(void);
void net_Quit(void);


//-------------IPv4 hostname resolution API

typedef struct {
	unsigned int host;			/* 32-bit IPv4 host address */
	unsigned short port;			/* 16-bit protocol port */
} IPaddress;

int net_ResolveHost(IPaddress *address, const char *host, unsigned short port);
const char * net_ResolveIP(IPaddress *ip);

//-------------TCP network API

typedef struct _TCPsocket *TCPsocket;

TCPsocket net_TCP_Open(IPaddress *ip);
TCPsocket net_TCP_Accept(TCPsocket server);
IPaddress * net_TCP_GetPeerAddress(TCPsocket sock);
int net_TCP_Send(TCPsocket sock, void *data, int len);
int net_TCP_Recv(TCPsocket sock, void *data, int maxlen);
void net_TCP_Close(TCPsocket sock);

#ifdef _WIN32
	#ifndef socklen_t
		#define socklen_t int
	#endif //socklen_t
#endif //_WIN32

//-------------Hooks for checking sockets for available data

typedef struct _net_SocketSet *net_SocketSet;

/* Any network socket can be safely cast to this socket type */
typedef struct _net_GenericSocket {
	int ready;
} *net_GenericSocket;

net_SocketSet net_AllocSocketSet(int maxsockets);
int net_AddSocket(net_SocketSet set, net_GenericSocket sock);
int net_DelSocket(net_SocketSet set, net_GenericSocket sock);
int net_CheckSockets(net_SocketSet set, unsigned int timeout);
void net_FreeSocketSet(net_SocketSet set);

#define net_SocketReady(sock) \
	((sock != NULL) && ((net_GenericSocket)sock)->ready)

#define net_TCP_AddSocket(set, sock) \
		net_AddSocket(set, (net_GenericSocket)sock)

#define net_TCP_DelSocket(set, sock) \
	net_DelSocket(set, (net_GenericSocket)sock)



//-------------Platform-independent data conversion functions

/* Write a 16/32 bit value to network packet buffer */
void net_Write16(unsigned short value, void *area);
void net_Write32(unsigned int value, void *area);

/* Read a 16/32 bit value from network packet buffer */
unsigned short net_Read16(void *area);
unsigned int net_Read32(void *area);

/* Inline macro functions to read/write network data */

/* Warning, some systems have data access alignment restrictions */
#if defined(sparc) || defined(mips)
#define NET_DATA_ALIGNED	1
#endif
#ifndef NET_DATA_ALIGNED
#define NET_DATA_ALIGNED	0
#endif

/* Write a 16 bit value to network packet buffer */
#if !NET_DATA_ALIGNED
#define net_Write16(value, areap)	\
	(*(unsigned short *)(areap) = SDLNET_SwapBE16(value))
#else
#if NET_BYTEORDER == NET_BIG_ENDIAN
#define net_Write16(value, areap)	\
	do 					\
{					\
	unsigned char *area = (unsigned char *)(areap);	\
	area[0] = (value >>  8) & 0xFF;	\
	area[1] =  value        & 0xFF;	\
} while ( 0 )
#else
#define net_Write16(value, areap)	\
	do 					\
{					\
	unsigned char *area = (unsigned char *)(areap);	\
	area[1] = (value >>  8) & 0xFF;	\
	area[0] =  value        & 0xFF;	\
} while ( 0 )
#endif
#endif /* !NET_DATA_ALIGNED */

/* Write a 32 bit value to network packet buffer */
#if !NET_DATA_ALIGNED
#define net_Write32(value, areap) 	\
	*(unsigned int *)(areap) = NET_SwapBE32(value);
#else
#if NET_BYTEORDER == NET_BIG_ENDIAN
#define net_Write32(value, areap) 	\
	do					\
{					\
	unsigned char *area = (unsigned char *)(areap);	\
	area[0] = (value >> 24) & 0xFF;	\
	area[1] = (value >> 16) & 0xFF;	\
	area[2] = (value >>  8) & 0xFF;	\
	area[3] =  value       & 0xFF;	\
} while ( 0 )
#else
#define net_Write32(value, areap) 	\
	do					\
{					\
	unsigned char *area = (unsigned char *)(areap);	\
	area[3] = (value >> 24) & 0xFF;	\
	area[2] = (value >> 16) & 0xFF;	\
	area[1] = (value >>  8) & 0xFF;	\
	area[0] =  value       & 0xFF;	\
} while ( 0 )
#endif
#endif /* !SDL_DATA_ALIGNED */

/* Read a 16 bit value from network packet buffer */
#if !NET_DATA_ALIGNED
#define net_Read16(areap) 		\
	(NET_SwapBE16(*(unsigned short *)(areap)))
#else
#if NET_BYTEORDER == NET_BIG_ENDIAN
#define net_Read16(areap) 		\
	((((unsigned char *)areap)[0] <<  8) | ((unsigned char *)areap)[1] <<  0)
#else
#define net_Read16(areap) 		\
	((((unsigned char *)areap)[1] <<  8) | ((unsigned char *)areap)[0] <<  0)
#endif
#endif /* !SDL_DATA_ALIGNED */

/* Read a 32 bit value from network packet buffer */
#if !NET_DATA_ALIGNED
#define net_Read32(areap) 		\
	(NET_SwapBE32(*(unsigned int *)(areap)))
#else
#if NET_BYTEORDER == NET_BIG_ENDIAN
#define net_Read32(areap) 		\
	((((unsigned char *)areap)[0] << 24) | (((unsigned char *)areap)[1] << 16) | \
	(((unsigned char *)areap)[2] <<  8) |  ((unsigned char *)areap)[3] <<  0)
#else
#define net_Read32(areap) 		\
	((((unsigned char *)areap)[3] << 24) | (((unsigned char *)areap)[2] << 16) | \
	(((unsigned char *)areap)[1] <<  8) |  ((unsigned char *)areap)[0] <<  0)
#endif
#endif /* !SDL_DATA_ALIGNED */

#endif// _NETWORKING_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
