/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if !defined(_WIN32)

#include "network.h"
#include "ErrorHandler.h"
#include "Address.h"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#if defined(_old_linux_)
#define hstrerror(x) sys_errlist[x]
#elif defined(sun)
#define hstrerror(x) "<network error>"
#endif

extern "C" {

void					nerror(const char* msg)
{
	if (msg)
		printError("%s: %s", msg, strerror(errno));
	else
		printError("%s", strerror(errno));
}

void					bzfherror(const char* msg)
{
	if (msg)
		printError("%s: %s", msg, hstrerror(h_errno));
	else
		printError("%s", hstrerror(h_errno));
}

int						getErrno()
{
	return errno;
}

}

int						BzfNetwork::setNonBlocking(int fd)
{
	int mode = fcntl(fd, F_GETFL, 0);
	if (mode == -1 || fcntl(fd, F_SETFL, mode | O_NDELAY) < 0)
		return -1;
	return 0;
}

#else /* defined(_WIN32) */

#include "network.h"
#include "ErrorHandler.h"
#include "Address.h"
#include <stdio.h>
#include <string.h>

extern "C" {

int						inet_aton(const char* cp, struct in_addr* pin)
{
	unsigned long a = inet_addr(cp);
	if (a == (unsigned long)-1) {
		// could be an error or cp could be a broadcast address.
		// FIXME -- this check is a little simplistic.
		if (strcmp(cp, "255.255.255.255") != 0) return 0;
	}

	pin->s_addr = a;
	return 1;
}

// win32 apparently cannot lookup error messages for us
static const struct { int code; const char* msg; } netErrorCodes[] = {
		/* 10004 */{WSAEINTR,			"The (blocking) call was canceled via WSACancelBlockingCall"},
		/* 10009 */{WSAEBADF,			"Bad file handle"},
		/* 10013 */{WSAEACCES,			"The requested address is a broadcast address, but the appropriate flag was not set"},
		/* 10014 */{WSAEFAULT,			"WSAEFAULT"},
		/* 10022 */{WSAEINVAL,			"WSAEINVAL"},
		/* 10024 */{WSAEMFILE,			"No more file descriptors available"},
		/* 10035 */{WSAEWOULDBLOCK,		"Socket is marked as non-blocking and no connections are present or the receive operation would block"},
		/* 10036 */{WSAEINPROGRESS,		"A blocking Windows Sockets operation is in progress"},
		/* 10037 */{WSAEALREADY,		"The asynchronous routine being canceled has already completed"},
		/* 10038 */{WSAENOTSOCK,		"At least on descriptor is not a socket"},
		/* 10039 */{WSAEDESTADDRREQ,	"A destination address is required"},
		/* 10040 */{WSAEMSGSIZE,		"The datagram was too large to fit into the specified buffer and was truncated"},
		/* 10041 */{WSAEPROTOTYPE,		"The specified protocol is the wrong type for this socket"},
		/* 10042 */{WSAENOPROTOOPT,		"The option is unknown or unsupported"},
		/* 10043 */{WSAEPROTONOSUPPORT,"The specified protocol is not supported"},
		/* 10044 */{WSAESOCKTNOSUPPORT,"The specified socket type is not supported by this address family"},
		/* 10045 */{WSAEOPNOTSUPP,		"The referenced socket is not a type that supports that operation"},
		/* 10046 */{WSAEPFNOSUPPORT,	"BSD: Protocol family not supported"},
		/* 10047 */{WSAEAFNOSUPPORT,	"The specified address family is not supported"},
		/* 10048 */{WSAEADDRINUSE,		"The specified address is already in use"},
		/* 10049 */{WSAEADDRNOTAVAIL,	"The specified address is not available from the local machine"},
		/* 10050 */{WSAENETDOWN,		"The Windows Sockets implementation has detected that the network subsystem has failed"},
		/* 10051 */{WSAENETUNREACH,		"The network can't be reached from this hos at this time"},
		/* 10052 */{WSAENETRESET,		"The connection must be reset because the Windows Sockets implementation dropped it"},
		/* 10053 */{WSAECONNABORTED,	"The virtual circuit was aborted due to timeout or other failure"},
		/* 10054 */{WSAECONNRESET,		"The virtual circuit was reset by the remote side"},
		/* 10055 */{WSAENOBUFS,			"No buffer space is available or a buffer deadlock has occured. The socket cannot be created"},
		/* 10056 */{WSAEISCONN,			"The socket is already connected"},
		/* 10057 */{WSAENOTCONN,		"The socket is not connected"},
		/* 10058 */{WSAESHUTDOWN,		"The socket has been shutdown"},
		/* 10059 */{WSAETOOMANYREFS,	"BSD: Too many references"},
		/* 10060 */{WSAETIMEDOUT,		"Attempt to connect timed out without establishing a connection"},
		/* 10061 */{WSAECONNREFUSED,	"The attempt to connect was forcefully rejected"},
		/* 10062 */{WSAELOOP,			"Undocumented WinSock error code used in BSD"},
		/* 10063 */{WSAENAMETOOLONG,	"Undocumented WinSock error code used in BSD"},
		/* 10064 */{WSAEHOSTDOWN,		"Undocumented WinSock error code used in BSD"},
		/* 10065 */{WSAEHOSTUNREACH,	"No route to host"},
		/* 10066 */{WSAENOTEMPTY,		"Undocumented WinSock error code"},
		/* 10067 */{WSAEPROCLIM,		"Undocumented WinSock error code"},
		/* 10068 */{WSAEUSERS,			"Undocumented WinSock error code"},
		/* 10069 */{WSAEDQUOT,			"Undocumented WinSock error code"},
		/* 10070 */{WSAESTALE,			"Undocumented WinSock error code"},
		/* 10071 */{WSAEREMOTE,			"Undocumented WinSock error code"},
		/* 10091 */{WSASYSNOTREADY,		"Underlying network subsytem is not ready for network communication"},
		/* 10092 */{WSAVERNOTSUPPORTED,	"The version of WinSock API support requested is not provided in this implementation"},
		/* 10093 */{WSANOTINITIALISED,	"WinSock subsystem not properly initialized"},
		/* 10101 */{WSAEDISCON,			"Virtual circuit has gracefully terminated connection"},
		/* 11001 */{WSAHOST_NOT_FOUND,	"Host not found"},
		/* 11002 */{WSATRY_AGAIN,		"Host not found"},
		/* 11003 */{WSANO_RECOVERY,		"Host name lookup error"},
		/* 11004 */{WSANO_DATA,			"No data for host"},
		/* end   */{0,					NULL}
};

void					nerror(const char* msg)
{
	const int err = getErrno();
	const char* errmsg = "<unknown error>";
	for (int i = 0; netErrorCodes[i].code != 0; ++i)
		if (netErrorCodes[i].code == err) {
			errmsg = netErrorCodes[i].msg;
			break;
		}
	if (msg)
		printError("%s: %s (%d)", msg, errmsg, err);
	else
		printError("%s (%d)", errmsg, err);
}

void					herror(const char* msg)
{
	nerror(msg);
}

int						getErrno()
{
	return WSAGetLastError();
}

}

int						BzfNetwork::setNonBlocking(int fd)
{
	int on = 1;
	return ioctl(fd, FIONBIO, &on);
}

#endif /* defined(_WIN32) */

// valid bzflag url's are:
//   http://<hostname>[:<port>][<path-to-text-file>]
//     connect to above HTTP server requesting named file.  response
//     must be another url of any valid form.  a 302 response code
//     (object moved) is understood and redirection is automatic.
//
//   file:<pathname>
//     reads from file.  contents must be a url of any valid form.
//     note: pathname can include a drive specifier on windows.
//     note: pathname must be absolute.
//
//   bzflist://<hostname>[:<port>]
//     named host must be a bzflag list server listening on port
//     (if not supplied, port is ServerPort + 1).  hostname can
//     be a hostname or IP address.
//
// note that partially formed urls are not allowed.  for example, the
// http:// cannot be elided.

std::string				BzfNetwork::dereferenceHTTP(
								const std::string& hostname, int port,
								const std::string& pathname)
{
	// we need to getenv("HTTP_PROXY") here and use if if it exists
	// note that a complete implementation may have:
	// HTTP_PROXY=http://user@zone:password@www.example.com:port
	// HTTP_NO_PROXY=example.com,example.org:8088

	// the username/password should be sent in reply to a
	// 401 auth required challenge

	// lookup server address
	Address address = Address::getHostAddress(hostname.c_str());
	if (address.isAny())
		return std::string();

	// create socket
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		nerror("creating socket for HTTP");
		return std::string();
	}

	// connect to HTTP server
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = address;
	if (connect(fd, (CNCTType*)&addr, sizeof(addr)) < 0) {
		nerror("connecting to HTTP server");
		closesocket(fd);
		return std::string();
	}

	// form request
	std::string data("GET ");
	data += pathname;					// FIXME -- escape special characters
	data += " HTTP/1.0\r\nHost: ";
	data += hostname;
	data += "\r\nAccept: */*\r\n\r\n";

	// send request
	if (send(fd, data.c_str(), data.size(), 0) < 0) {
		nerror("sending request to HTTP server");
		closesocket(fd);
		return std::string();
	}

	// slurp up reply
	char line[256];
	data = "";
	int size;
	while ((size = recv(fd, line, sizeof(line), 0)) > 0)
		data.append(line, size);

	// fail if there was an error receiving
	if (size < 0) {
		nerror("receiving reply from HTTP server");
		closesocket(fd);
		return std::string();
	}

	// close socket
	closesocket(fd);

	// parse reply.  first check for HTTP response and get the result code.
	const char* scan = data.c_str();
	char code[4];
	if (strncmp(data.c_str(), "HTTP/1.", 7) != 0)
		return std::string();
	scan += 7;
	while (*scan && !isspace(*scan)) ++scan;
	while (*scan &&  isspace(*scan)) ++scan;
	memcpy(code, scan, 3);
	code[3] = '\0';

	// what was the result?  we only accept results 200 (OK) and 302
	// (object moved).
	if (strcmp(code, "200") == 0) {
		// skip past headers to body
		const char* body = strstr(scan, "\r\n\r\n");
		if (body)
			body += 4;
		else {
			body = strstr(scan, "\n\n");
			if (body)
				body += 2;
			else
				return std::string();
		}

		// data is entire body
		return data.substr(body - data.c_str());
	}
	else if (strcmp(code, "302") == 0) {
		// find Location: header
		const char* location = strstr(scan, "Location:");
		if (!location)
			return std::string();

		// data is rest of Location: header line sans leading/trailing whitespace.
		// skip to beginning of url.
		location += 9;
		while (*location && isspace(*location))
			++location;

		// find end of header line, minus whitespace
		const char* end = strchr(location, '\n');
		if (!end)
			return std::string();
		while (end > location && isspace(end[-1]))
			--end;

		// get length
		const int urlLength = end - location;
		if (urlLength == 0)
			return std::string();

		// copy url
		return data.substr(location - data.c_str(), urlLength);
	}

	return std::string();
}

std::string				BzfNetwork::dereferenceFile(
								const std::string& pathname)
{
	// open file
	FILE* file = fopen(pathname.c_str(), "r");
	if (!file)
		return std::string();

	// slurp up file
	char line[256];
	std::string data;
	while (fgets(line, sizeof(line), file))
		data += line;

	// close file
	fclose(file);

	return data;
}

void					BzfNetwork::insertLines(URLList& list,
								int index, const std::string& data)
{
    const char* start, *end, *tail;

    start = data.c_str();
    while (*start) {
		// skip leading whitespace
		while (*start && isspace(*start))
		    start++;

		// find end of line or end of string
		end = start;
		while (*end && *end != '\r' && *end != '\n')
		    end++;

		// back up over trailing whitespace
		tail = end;
		while (tail > start && isspace(tail[-1]))
		    tail--;

		// if non-empty and not beginning with # then add to list
		if (end > start && *start != '#')
		    list.insert(&list[index++], data.substr(start - data.c_str(), end - start));

		// go to next line
		start = end;
		while (*start == '\r' || *start == '\n')
		    start++;
    }
}

bool					BzfNetwork::dereferenceURLs(
								URLList& list, unsigned int max,
								URLList& failedList)
{
    unsigned int i = 0;

    while (i < list.size() && i < max) {
		std::string protocol, hostname, pathname;
		int port = 0;

		// parse next url
		parseURL(list[i], protocol, hostname, port, pathname);

		// dereference various protocols
		if (protocol == "http") {
		    // get data
		    if (port == 0)
		      port = 80;
		    std::string data = dereferenceHTTP(hostname, port, pathname);

		    // insert new URLs
		    if (data.empty())
				failedList.push_back(list[i]);
		    else
				insertLines(list, i + 1, data);

		    // done with this URL
		    list.erase(&list[i]);
		}

		else if (protocol == "file") {
		    // get data
		    std::string data = dereferenceFile(pathname);

		    // insert new URLs
		    if (data.empty())
				failedList.push_back(list[i]);
		    else
				insertLines(list, i + 1, data);

		    // done with this URL
		    list.erase(&list[i]);
		}

		else if (protocol == "bzflist") {
		    // leave it alone
		    i++;
		}

		else {
		    // invalid protocol or url
		    failedList.push_back(list[i]);
		    list.erase(&list[i]);
		}
    }

    // remove any urls we didn't get to
    if (list.size() > max)
		list.erase(&list[max], list.end());

    return (list.size() > 0);
}

// parse a url into its parts
bool					BzfNetwork::parseURL(const std::string& url,
								std::string& protocol,
								std::string& hostname,
								int& port,
								std::string& pathname)
{
	static const char* defaultHostname = "localhost";

	// scan for :
	const char* base = url.c_str();
	const char* scan = base;
	while (*scan != '\0' && *scan != ':' && !isspace(*scan))
		++scan;

	// url is bad if delimiter not found or is first character or whitespace
	// found.
	if (*scan == '\0' || scan == base || isspace(*scan))
		return false;

	// set defaults
	hostname = defaultHostname;

	// store protocol
	protocol = url.substr(base - url.c_str(), scan - base);
	scan++;

	// store hostname and optional port for some protocols
	if (protocol == "http" || protocol == "bzflist") {
		if (scan[0] == '/' && scan[1] == '/') {
			// scan over hostname and store it
			base = scan + 2;
			scan = base;
			while (*scan != '\0' && *scan != ':' &&
						*scan != '/' && *scan != '\\' && !isspace(*scan))
				++scan;
			if (isspace(*scan))
				return false;
			if (scan != base)
				hostname = url.substr(base - url.c_str(), scan - base);

			// scan over and store port number
			if (*scan == ':') {
				scan++;
				base = scan;
				while (isdigit(*scan))
					++scan;
				port = atoi(base);
			}

			// next character must be / or \ or there must be no next character
			if (*scan != '\0' && *scan != '/' && *scan != '\\')
				return false;
		}
	}
	base = scan;

	// store pathname
	if (*base != 0)
		pathname = url.substr(base - url.c_str());
	else
		pathname = "";

	return true;
}
// ex: shiftwidth=4 tabstop=4
