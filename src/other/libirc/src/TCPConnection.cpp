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

// TCP/IP connection classes

#include "TCPConnection.h"
//#include "sdl_net/SDLnetsys.h"

#define DEFAULT_READ_CHUNK 512

unsigned int lastUID = 0;

#include <map>
//---------------------------------------------------------------------------------------------------//
// TCP/IP packet class

TCPPacket::TCPPacket()
{
	data = NULL;
	size = 0;
}

TCPPacket::TCPPacket( const TCPPacket &p )
{
	data = (unsigned char*)malloc(p.size);
	memcpy(data,p.data,p.size);
	size = p.size;
}

TCPPacket::TCPPacket( unsigned char* buf, unsigned int len )
{
	data = NULL;
	size = 0;
	set(buf,len);
}

TCPPacket::~TCPPacket()
{
	if (data)
		free(data);
	data = NULL;
	size = 0;
}

TCPPacket& TCPPacket::operator = ( const TCPPacket &p )
{
	if (data)
		free (data);

	data = (unsigned char*)malloc(p.size);
	memcpy(data,p.data,p.size);
	size = p.size;
	return *this;
}

void TCPPacket::set ( unsigned char* buf, unsigned int len )
{
	if (data)
		free (data);

	data = (unsigned char*)malloc(len);
	memcpy(data,buf,len);
	size = len;
}

unsigned char* TCPPacket::get ( unsigned int &len )
{
	len = size;
	return data;
}

//---------------------------------------------------------------------------------------------------//
// TCP/IP connection class for clients


TCPClientConnection::TCPClientConnection()
{
	parent = NULL;
	lastError = eTCPNotInit;
	serverIP.host = 0;
	serverIP.port = 0;
	socket = NULL;
	readChunkSize = 512;
}

TCPClientConnection::TCPClientConnection( std::string server, unsigned short port, TCPConnection *parentConnection )
{
	parent = NULL;
	lastError = eTCPNotInit;
	serverIP.host = 0;
	serverIP.port = 0;
	socket = NULL;
	readChunkSize = 512;

	parent = parentConnection;

	if (server.size() && port != 0)
		connect(server,port);
}

TCPClientConnection::~TCPClientConnection()
{
}

teTCPError TCPClientConnection::connect ( std::string server, unsigned short port )
{
	if ( net_ResolveHost(&serverIP, server.c_str(), port))
		return setError(eTCPUnknownError);

	if ( serverIP.host == INADDR_NONE )
		return setError(eTCPBadAddress);

	return connect();	
}

teTCPError TCPClientConnection::connect ( void )
{
	disconnect();

	if ( serverIP.host == 0 || serverIP.port == 0 || serverIP.host == INADDR_NONE )
		return setError(eTCPBadAddress);

	socket = net_TCP_Open(&serverIP);
	if ( socket == NULL )
		return setError(eTCPConnectionFailed);

	if (parent)
		parent->addClientSocket(this);

	return setError(eTCPNoError);
}

teTCPError TCPClientConnection::disconnect( void )
{
	if (!connected())
		return setError(eTCPSocketNFG);

	net_TCP_Close(socket);
	socket = NULL;

	if (parent)
		parent->removeClientSocket(this);

	return setError(eTCPNoError);
}

bool TCPClientConnection::connected ( void )
{
	return socket != NULL;
}

bool TCPClientConnection::packets ( void )
{
	return packetList.size() > 0;
}

tvPacketList& TCPClientConnection::getPackets ( void )
{
	return packetList;
}

void TCPClientConnection::readData ( void )
{
	bool done = false;
	int dataRead = 0;

	unsigned char	*chunk = (unsigned char*)malloc(readChunkSize);
	unsigned char	*data = NULL;
	int		totalSize = 0;

	while (!done)
	{
		dataRead =  net_TCP_Recv(socket, chunk, readChunkSize);
		if (dataRead > 0)
		{
			if (data)
				data = (unsigned char*)realloc(data,totalSize+dataRead);
			else
				data = (unsigned char*)malloc(dataRead);

			if (data)
			{
				memcpy(&data[totalSize],chunk,dataRead);
				totalSize += dataRead;

				if (dataRead < readChunkSize)
					done = true;
			}
			else	// there was an error
				done = true;

		}
		else // there was an error
			done = true;
	}

	if (chunk)
		free(chunk);

	if (data)
	{
		packetList.push_back(TCPPacket(data,totalSize));
		free(data);
	}

	// notify any listeners
	callDataPendingListeners((int)packetList.size());
}

teTCPError TCPClientConnection::sendData ( void *data, int len )
{
	if (!socket)
		return setError(eTCPSocketNFG);

	if (!data || len < 1)
		return setError(eTCPDataNFG);

	int lenSent = net_TCP_Send(socket,data,len);

	if (lenSent < len)
		return setError(eTCPConnectionFailed);

	return setError(eTCPNoError);
}

teTCPError TCPClientConnection::sendData ( const char *data, int len )
{
	return sendData((void*)data,len);
}

teTCPError TCPClientConnection::sendData ( std::string data )
{
	return sendData(data.c_str(),(int)data.size());
}

// data pending listeners
void TCPClientConnection::addListener ( TCPClientDataPendingListener* listener )
{
	if (!listener)
		return;

	dataPendingList.push_back(listener);
}

void TCPClientConnection::removeListener ( TCPClientDataPendingListener* listener )
{
	if (!listener)
		return;

	tvClientDataPendingListenerList::iterator	itr = dataPendingList.begin();
	while( itr != dataPendingList.end() )
	{
		if (*itr == listener)
			itr = dataPendingList.erase(itr);
		else
			itr++;
	}
}

void TCPClientConnection::callDataPendingListeners ( int count )
{
	if ( count >1 )
		return;

	tvClientDataPendingListenerList::iterator	itr = dataPendingList.begin();
	while( itr != dataPendingList.end() )
	{
		(*itr)->pending(this,count);
		itr++;
	}
}

teTCPError TCPClientConnection::getLastError ( void )
{
	return lastError;
}

teTCPError TCPClientConnection::setError ( teTCPError error )
{
	return lastError = error;
}

void TCPClientConnection::setReadChunkSize ( unsigned int size )
{
	if (size > 0)
		readChunkSize = size;
}

unsigned int TCPClientConnection::getReadChunkSize ( void )
{
	return readChunkSize;
}

//---------------------------------------------------------------------------------------------------//
// TCP/IP listener class for server connected peers

TCPServerConnectedPeer::TCPServerConnectedPeer()
{
	socket = NULL;
	lastError = eTCPNoError;
	UID = ++lastUID;
	param = NULL;
}

TCPServerConnectedPeer::~TCPServerConnectedPeer()
{
}

/*TCPServerConnectedPeer& TCPServerConnectedPeer::operator = ( const TCPServerConnectedPeer &p )
{
UID = p.UID;
info->socket = p.info->socket;
info->address.host = p.info->address.host;
info->address.port = p.info->address.port;
info->lastError = p.info->lastError;
packetList = p.packetList;
host = p.host;
param = p.param;

return *this;
} */

// data pending
bool TCPServerConnectedPeer::packets ( void )
{
	return packetList.size() > 0;
}

tvPacketList& TCPServerConnectedPeer::getPackets ( void )
{
	return packetList;
}

void TCPServerConnectedPeer::flushPackets ( void )
{
	packetList.clear();
}

teTCPError TCPServerConnectedPeer::getLastError ( void )
{
	return lastError;
}

teTCPError TCPServerConnectedPeer::setError ( teTCPError error )
{
	return lastError = error;
}

const std::string TCPServerConnectedPeer::getHostMask ( void )
{
	if (socket ||!host.size())
		host = net_ResolveIP(&address);

	return host;
}

bool TCPServerConnectedPeer::getIP ( unsigned char ip[4] )
{
	if (!socket)
		return false;

	memcpy(ip,&address.host,4);
	return true;
}

void TCPServerConnectedPeer::connect ( void* _socket )
{
	socket = (TCPsocket)_socket;
	address = *net_TCP_GetPeerAddress(socket);
}

teTCPError TCPServerConnectedPeer::sendData ( void *data, int len )
{
	if (!socket)
		return setError(eTCPSocketNFG);

	if (!data || len < 1)
		return setError(eTCPDataNFG);

	int lenSent = net_TCP_Send(socket,data,len);

	if (lenSent < len)
		return setError(eTCPConnectionFailed);

	return setError(eTCPNoError);
}

teTCPError TCPServerConnectedPeer::sendData ( const char *data, int len )
{
	return sendData((void*)data,len);
}

teTCPError TCPServerConnectedPeer::sendData ( std::string data )
{
	return sendData(data.c_str(),(int)data.size());
}

bool TCPServerConnectedPeer::readData ( void )
{
	if (!socket)
		return false;

	unsigned char buffer[513];
	unsigned char *realData = NULL;
	unsigned int realDataSize = 0;

	memset(buffer,0,513);
	int read = net_TCP_Recv(socket,buffer,512);

	if (read < 0)
		return false;

	while ( read > 0 )
	{
		unsigned char *temp = realData;
		realData = (unsigned char*)malloc(realDataSize+read);
		if (temp)
			memcpy(realData,temp,realDataSize);

		memcpy(&realData[realDataSize],buffer,read);
		realDataSize += read;
		if ( temp )
			free (temp);

		read = net_TCP_Recv(socket,buffer,512);
	}

	if ( realData )
	{
		TCPPacket	packet(realData,realDataSize);
		packetList.push_back(packet);

		free(realData);
	}

	return true;
}

//---------------------------------------------------------------------------------------------------//
// TCP/IP listener class for servers


TCPServerConnection::TCPServerConnection()
{
	lastError = eTCPNotInit;
	serverIP.port = maxUsers = 0;
	socket = NULL;
	parent = NULL;
	readChunkSize = 512;
}

TCPServerConnection::TCPServerConnection( unsigned short port, unsigned int connections, TCPConnection *parentConnection )
{
	lastError = eTCPNotInit;
	parent = parentConnection;
	socket = NULL;
	readChunkSize = 512;
	listen(port,connections);
}

TCPServerConnection::~TCPServerConnection()
{
	disconnect();
}

teTCPError TCPServerConnection::listen ( unsigned short port, unsigned int connections )
{
	disconnect();

	if ( port == 0)
		return setError(eTCPBadPort);

	maxUsers = connections;
	serverIP.host = INADDR_ANY;
	serverIP.port = port;

	socket = net_TCP_Open(&serverIP);
	if ( socket == NULL )
		return setError(eTCPConnectionFailed);

	socketSet = net_AllocSocketSet(getMaxConnections()+1);
	if (!socketSet)
		return setError(eTCPSocketNFG);

	IPaddress serverIP;
	net_ResolveHost(&serverIP, NULL, getPort());
	socket = net_TCP_Open(&serverIP);
	net_TCP_AddSocket(socketSet,socket);

	return setError(eTCPNoError);
}

teTCPError TCPServerConnection::disconnect( void )
{
	if (!listening())
		return setError(eTCPSocketNFG);

	std::map<TCPsocket,TCPServerConnectedPeer>::iterator itr = peers.begin();

	while ( itr != peers.end() )
	{
		for ( unsigned int i = 0; i < dataPendingList.size(); i++ )
			dataPendingList[i]->disconnect(this,&itr->second,true);

		net_TCP_Close(itr->first);
		net_DelSocket(socketSet,(net_GenericSocket)itr->first);
	}

	peers.clear();

	net_TCP_Close(socket);
	net_DelSocket(socketSet,(net_GenericSocket)socket);

	net_FreeSocketSet(socketSet);

	socketSet = NULL;
	socket = NULL;

	return setError(eTCPNoError);
}

bool TCPServerConnection::listening ( void )
{
	return socket != NULL;
}

unsigned short TCPServerConnection::getPort ( void )
{	
	return serverIP.port;
}

unsigned int TCPServerConnection::getMaxConnections ( void )
{
	return maxUsers;
}

std::vector<TCPServerConnectedPeer*> TCPServerConnection::getPeers ( void )
{
	std::map<TCPsocket,TCPServerConnectedPeer>::iterator itr = peers.begin();
	std::vector<TCPServerConnectedPeer*> peerList;

	while ( itr != peers.end() )
	{
		peerList.push_back(&(itr->second));
		itr++;
	}

	return peerList;
}

TCPServerConnectedPeer* TCPServerConnection::getPeerFromUID ( unsigned int UID )
{
	std::map<TCPsocket,TCPServerConnectedPeer>::iterator itr = peers.begin();

	while ( itr != peers.end() )
	{
		if (itr->second.getUID() == UID)
			return &itr->second;
		itr++;
	}

	return NULL;
}

bool TCPServerConnection::disconectPeer ( unsigned int UID )
{
	std::map<TCPsocket,TCPServerConnectedPeer>::iterator itr = peers.begin(), itr2 = peers.begin();

	while ( itr != peers.end() )
	{
		if (itr->second.getUID() == UID )
		{
			// they got discoed.
			for ( unsigned int i = 0; i < dataPendingList.size(); i++ )
				dataPendingList[i]->disconnect(this,&itr->second,true);

			net_TCP_DelSocket(socketSet, itr->first);
			net_TCP_Close(itr->first);

			itr2 = itr;
			itr2++;
			peers.erase(itr);
			itr = itr2;
			return true;
		}
		itr++;
	}
	return false;
}

bool TCPServerConnection::disconectPeer ( TCPServerConnectedPeer* peer )
{
	if (!peer)
		return false;

	std::map<TCPsocket,TCPServerConnectedPeer>::iterator itr = peers.begin(), itr2 = peers.begin();

	while ( itr != peers.end() )
	{
		if (&itr->second == peer )
		{
			// they got discoed.
			for ( unsigned int i = 0; i < dataPendingList.size(); i++ )
				dataPendingList[i]->disconnect(this,peer,true);

			net_TCP_DelSocket(socketSet, itr->first);
			net_TCP_Close(itr->first);

			itr2 = itr;
			itr2++;
			peers.erase(itr);
			itr = itr2;
			return true;
		}
		itr++;
	}
	return false;
}

bool TCPServerConnection::update ( void )
{
	if (net_CheckSockets(socketSet, ~0) < 1)
		return true;

	// see if our boys have any new connections
	if ( net_SocketReady(socket) )
	{
		TCPsocket		newsock;

		newsock = net_TCP_Accept(socket);
		while ( newsock != NULL )
		{
			TCPServerConnectedPeer	peer;
			peer.connect(newsock);
			peers[newsock] = peer;

			bool accept = true;
			for ( unsigned int i = 0; i < dataPendingList.size(); i++ )
			{
				if ( !dataPendingList[i]->connect(this,&(peers[newsock])) )
					accept = false;
			}
			if (accept)
				net_TCP_AddSocket(socketSet, newsock);
			else
			{
				peers.erase(peers.find(newsock));
				net_TCP_Close(newsock);
			}

			newsock = net_TCP_Accept(socket);
		}
	}

	std::map<TCPsocket,TCPServerConnectedPeer>::iterator itr = peers.begin(), itr2 = peers.begin();

	while ( itr != peers.end() )
	{
		if (net_SocketReady(itr->first))
		{
			if (itr->second.readData())
			{
				if ( itr->second.getPacketCount())
				{
					for ( unsigned int i = 0; i < dataPendingList.size(); i++ )
						dataPendingList[i]->pending(this,&itr->second,itr->second.getPacketCount());
				}
				itr++;
			}
			else
			{
				// they got discoed.
				for ( unsigned int i = 0; i < dataPendingList.size(); i++ )
					dataPendingList[i]->disconnect(this,&itr->second);

				net_TCP_DelSocket(socketSet, itr->first);
				net_TCP_Close(itr->first);

				itr2 = itr;
				itr2++;
				peers.erase(itr);
				itr = itr2;
				return true;
			}
		}
		else
			itr++;
	}

	return true;
}

teTCPError TCPServerConnection::getLastError ( void )
{
	return lastError;
}

teTCPError TCPServerConnection::setError ( teTCPError error )
{
	return error;
}

// data pending listeners
void TCPServerConnection::addListener ( TCPServerDataPendingListener* listener )
{
	if (!listener)
		return;

	dataPendingList.push_back(listener);
}

void TCPServerConnection::removeListener ( TCPServerDataPendingListener* listener )
{
	if (!listener)
		return;

	tvServerDataPendingListenerList::iterator	itr = dataPendingList.begin();
	while( itr != dataPendingList.end() )
	{
		if (*itr == listener)
			itr = dataPendingList.erase(itr);
		else
			itr++;
	}
}

// master connections class

/*const int NO_VARIANT = (-1); */

// initialize the singleton
template <>
TCPConnection* Singleton<TCPConnection>::_instance = (TCPConnection*)0;
typedef std::map<TCPsocket, TCPClientConnection* > tmClientSocketMap;

TCPConnection::TCPConnection()
{
	clientSocketSet = NULL;
	initedSocketInterface = false;
	timeout = 0;
	init();
}

TCPConnection::~TCPConnection()
{
	kill();
}

teTCPError TCPConnection::init ( void )
{
	kill();
	if(net_Init() ==0)
		initedSocketInterface = true;
	return initedSocketInterface ? eTCPNoError : eTCPInitFailed;
}

void TCPConnection::kill ( void )
{
	if (initedSocketInterface)
	{
		net_Quit();
		initedSocketInterface = false;
	}
}

teTCPError TCPConnection::update ( void )
{
	bool	selectError = false;
	if (clientSocketSet)
	{
		int items = net_CheckSockets(clientSocketSet,timeout);
		if (items == -1)
			selectError = true;
		if (items > 0)
		{
			tmClientSocketMap::iterator itr = clientSockets.begin();
			while (itr != clientSockets.end())
			{
				if (net_SocketReady(itr->first))
					itr->second->readData();
				itr++;
			}
		}
	}

	for ( unsigned int i = 0; i < serverConnections.size(); i++ )
		serverConnections[i]->update();

	return selectError ? eTCPSelectFailed : eTCPNoError;
}

void TCPConnection::setUpdateTimeout ( int timeout )
{
	if (timeout > 0)
		timeout = timeout;
}

TCPClientConnection* TCPConnection::newClientConnection ( std::string server, unsigned short port )
{
	TCPClientConnection*	connection = new  TCPClientConnection(server,port,this);
	
	clientConnections.push_back(connection);
	return connection;
}

TCPServerConnection* TCPConnection::newServerConnection ( unsigned short port, int connections )
{
	TCPServerConnection*	connection = new  TCPServerConnection(port,connections,this);

	serverConnections.push_back(connection);
	return connection;
}

void TCPConnection::deleteClientConnection ( TCPClientConnection* connection )
{
	tvClientConnectionList::iterator itr = clientConnections.begin();
	while( itr!=clientConnections.end() )
	{
		if (*itr == connection)
			itr = clientConnections.erase(itr);
		else
			itr++;
	}
	delete(connection);
}

void TCPConnection::deleteServerConnection ( TCPServerConnection* connection )
{
	tvServerConnectionList::iterator itr = serverConnections.begin();
	while( itr!=serverConnections.end() )
	{
		if (*itr == connection)
			itr = serverConnections.erase(itr);
		else
			itr++;
	}
	connection->disconnect();
	delete(connection);
}

std::string TCPConnection::getLocalHost ( void )
{
	return "127.0.0.1";
}

bool TCPConnection::addClientSocket ( TCPClientConnection* client )
{
	if (clientSocketSet)
		net_FreeSocketSet(clientSocketSet);

	clientSockets[client->socket] = client;

	clientSocketSet = net_AllocSocketSet((int)clientSockets.size());

	if (!clientSocketSet)
		return false;

	tmClientSocketMap::iterator itr = clientSockets.begin();
	while (itr != clientSockets.end())
	{
		net_TCP_AddSocket(clientSocketSet,itr->first);
		itr++;
	}

	return true;
}

bool TCPConnection::removeClientSocket ( TCPClientConnection* client )
{
	tmClientSocketMap::iterator itr = clientSockets.find(client->socket);
	if (itr == clientSockets.end())
		return false;

	net_TCP_DelSocket(clientSocketSet,client->socket);
	clientSockets.erase(itr);
	return true;
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
