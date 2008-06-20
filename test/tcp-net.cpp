/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "common.h"
#include <set>

#ifdef TEST_NET

class MyServerListener : public TCPServerDataPendingListener
{
    public:
        MyServerListener()
        {
            end = 0;
        }

        bool connect ( TCPServerConnection *connection, TCPServerConnectedPeer *peer )
        {
            printf("client connected\n");
            connSet.insert(connection);
            return true;
        }

        void pending ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, unsigned int count )
        {
            char buf[2048];
            tvPacketList& packets = peer->getPackets();
            for(tvPacketList::iterator itr = packets.begin(); itr != packets.end(); ++itr)
            {
                unsigned int len;
                unsigned char * data = (*itr).get(len);
                if(len >= 2048) break;
                strcpy(buf, (char*)data);
                buf[len] = 0;
                printf("%s\n", buf);
            }
        }

        void disconnect ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, bool forced = false )
        {
            printf("disconnected!\n");
            connSet.erase(connection);
            end = 1;
        }

        bool end;
    private:
        std::set<TCPServerConnection *> connSet;
        
};

void test_listen(void *)
{
    // don't use an instance of TCPConnection in this thread to avoid thread safety issues
    TCPServerConnection server;
    if(server.listen(1234, 10) != eTCPNoError)
    {
        printf("can't listen\n");
        return;
    }
    MyServerListener listener;
    server.addListener(&listener);
    while(1)
    {
        server.update();
        if(listener.end)
            break;
    }
}

void test_connect(void *)
{
    TCPClientConnection* client = TCPConnection::instance().newClientConnection("127.0.0.1", 1234);
    if(!client)
    {
        printf("cannot connect!");
        return;
    }
    
    _sleep(500);
    client->sendData(0, "just another text");
    _sleep(500);

    client->disconnect();

    TCPConnection::instance().deleteClientConnection(client);
}

void test_net()
{
    // initialize the net library
    TCPConnection::instance();

    _beginthread(test_listen, 0, NULL);
    _sleep(500);
    _beginthread(test_connect, 0, NULL);
}
#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8