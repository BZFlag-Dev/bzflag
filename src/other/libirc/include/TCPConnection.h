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

// TCP/IP connection class header

#ifndef _TCP_CONNECTION_H_
#define _TCP_CONNECTION_H_

#include <vector>
#include <string>
#include <map>

#include "Singleton.h"
#include "net.h"

// ------------------------TCP classes--------------------
class TCPConnection;

typedef enum
{
	eTCPNoError = 0,
	eTCPNotInit,
	eTCPTimeout,
	eTCPBadAddress,
	eTCPBadPort,
	eTCPConnectionFailed,
	eTCPSocketNFG,
	eTCPInitFailed,
	eTCPSelectFailed,
	eTCPDataNFG,
	eTCPUnknownError
}teTCPError;

// Packet data class
// this class is realy just to hold data, and make sure it get deallocated.
class TCPPacket
{
public:
	TCPPacket();
	TCPPacket( const TCPPacket &p );
	TCPPacket( unsigned char* buf, unsigned int len );
	~TCPPacket();

	TCPPacket& operator = ( const TCPPacket &p );

	void set ( unsigned char* buf, unsigned int len );
	unsigned char* get ( unsigned int &len );

protected:
	unsigned char	*data;
	unsigned int	size;
};

typedef std::vector<TCPPacket>	tvPacketList;

class TCPClientConnection;

class TCPClientDataPendingListener
{
public:
	virtual ~TCPClientDataPendingListener(){return;};
	virtual void pending ( TCPClientConnection *connection, int count ) = 0;
};

typedef std::vector<TCPClientDataPendingListener*> tvClientDataPendingListenerList;

// TCP/IP client connection to some host on some port
class TCPClientConnection
{
public:
	// connections
	teTCPError connect ( std::string server, unsigned short port );
	teTCPError connect ( void );
	teTCPError disconnect( void );

	// status
	bool connected ( void );

	// packet polling
	bool packets ( void );
	tvPacketList& getPackets ( void );

	// error handaling
	teTCPError getLastError ( void );
	teTCPError setError ( teTCPError error );

	// data read
	// generaly only called by the parent
	void readData ( void );

	// data send
	teTCPError sendData ( void *data, int len );
	teTCPError sendData ( const char *data, int len );
	teTCPError sendData ( std::string data );

	// utils
	void setReadChunkSize ( unsigned int size );
	unsigned int getReadChunkSize ( void );

	// data pending listeners
	// let the people register a callback class to be called when data is receved
	void addListener ( TCPClientDataPendingListener* listener );
	void removeListener ( TCPClientDataPendingListener* listener );

	TCPsocket	socket;

protected:
	// who's your daddy
	friend	class TCPConnection;

	// only daddy should make us
	TCPClientConnection();
	TCPClientConnection( std::string server, unsigned short port, TCPConnection *parentConnection );
	~TCPClientConnection();

	TCPConnection			*parent;
	tvPacketList			packetList;

	teTCPError	lastError;
	IPaddress	serverIP;
	int readChunkSize;

	// listeners
	tvClientDataPendingListenerList	dataPendingList;
	void callDataPendingListeners ( int count );
};

// server connection classes
class TCPServerConnection;

class TCPServerConnectedPeer
{
public:
	TCPServerConnectedPeer();
	~TCPServerConnectedPeer();

	// data pending
	bool packets ( void );
	tvPacketList& getPackets ( void );
	unsigned int getPacketCount ( void ) {return (unsigned int)packetList.size();}
	void flushPackets ( void );

	//TCPServerConnectedPeer& operator = ( const TCPServerConnectedPeer &p );

	// error handling
	teTCPError getLastError ( void );
	teTCPError setError ( teTCPError error );

	// called by the server class
	void connect ( void* _socket );
	bool readData ( void );

	// data send
	teTCPError sendData ( void *data, int len );
	teTCPError sendData ( const char *data, int len );
	teTCPError sendData ( std::string data );

	// client info
	unsigned int getUID ( void ){ return UID; }
	void* getParam ( void ) {return param;}
	void setParam ( void * p ){ param = p;}
	const std::string getHostMask ( void );
	bool getIP ( unsigned char ip[4] );

protected:
	unsigned int UID;

	TCPsocket				socket;
	IPaddress				address;
	teTCPError				lastError;

	tvPacketList			packetList;
	std::string				host;

	void					*param;
};

class TCPServerDataPendingListener
{
public:
	virtual ~TCPServerDataPendingListener(){return;};
	virtual bool connect ( TCPServerConnection *connection, TCPServerConnectedPeer *peer ) = 0;
	virtual void pending ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, unsigned int count ) = 0;
	virtual void disconnect ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, bool forced = false ){};
};

typedef std::vector<TCPServerDataPendingListener*> tvServerDataPendingListenerList;

typedef struct 
{

}trServerConectedPeer;

// TCP/IP server connection listening for some connections on some port
class TCPServerConnection
{
public:
	TCPServerConnection();
	TCPServerConnection( unsigned short port, unsigned int connections, TCPConnection *parentConnection );
	~TCPServerConnection();

	teTCPError listen ( unsigned short port, unsigned int connections );
	teTCPError disconnect( void );

	// status
	bool listening ( void );

	// info
	unsigned short getPort ( void );
	unsigned int getMaxConnections ( void );

	// error handaling
	teTCPError getLastError ( void );
	teTCPError setError ( teTCPError error );

	// data pending listeners
	// let the people register a callback class to be called when data is receved
	void addListener ( TCPServerDataPendingListener* listener );
	void removeListener ( TCPServerDataPendingListener* listener );

	std::vector<TCPServerConnectedPeer*> getPeers ( void );
	TCPServerConnectedPeer* getPeerFromUID ( unsigned int UID );

	bool disconectPeer ( unsigned int UID );
	bool disconectPeer ( TCPServerConnectedPeer* peer );

	// update the shizzle
	bool update ( void );

protected:
	// who's your daddy
	friend	class TCPConnection;
	TCPConnection			*parent;

	teTCPError									lastError;
	int											maxUsers;
	IPaddress									serverIP;
	TCPsocket									socket;
	int											readChunkSize;

	net_SocketSet							socketSet;
	std::map<TCPsocket,TCPServerConnectedPeer>	peers;

	// listeners
	tvServerDataPendingListenerList	dataPendingList;
};

// master TCP/IP connection manager
// takes care of loading and clearing out the TCP stack if it needs it
// gives out client and server connections
// handles the non blocking updates of connections.
class TCPConnection : public Singleton<TCPConnection>
{
public:
	// initalises the socket system, on windows inits WSA
	// called automaticly by the constructor, but exposted
	// in case the client needs to reinit the socket system
	teTCPError init ( void );

	// kills the socket system, closes all connections,
	// and on windows cleans up the WSA system
	void kill ( void );

	// has all connections check for new data
	teTCPError update ( void );
	void setUpdateTimeout ( int timeout );

	// returns a new client conenction to the specified host and port
	TCPClientConnection* newClientConnection ( std::string server, unsigned short port );

	// returns a new server connection that will listen for the 
	// specified number of connections on the specfied port
	TCPServerConnection* newServerConnection ( unsigned short port, int connections );

	// disconnects and removes connections
	void deleteClientConnection ( TCPClientConnection* connection );
	void deleteServerConnection ( TCPServerConnection* connection );

	// returns the lost host
	std::string getLocalHost ( void );

protected:
	friend class Singleton<TCPConnection>;

	TCPConnection();
	~TCPConnection();

	friend	class TCPClientConnection;
	friend	class TCPServerConnection;

	typedef std::vector<TCPClientConnection*>	tvClientConnectionList;
	tvClientConnectionList		clientConnections;

	typedef std::vector<TCPServerConnection*>	tvServerConnectionList;
	tvServerConnectionList		serverConnections;

	bool addClientSocket ( TCPClientConnection* client );

	bool removeClientSocket ( TCPClientConnection* client );

	typedef std::map<TCPsocket, TCPClientConnection* > tmClientSocketMap;
	tmClientSocketMap	clientSockets;
	net_SocketSet	clientSocketSet;

	bool initedSocketInterface;
	int	 timeout;
};

#endif //_TCP_CONNECTION_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
