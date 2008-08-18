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
#include "assert.h"

#ifdef TEST_NET

#include "../bzAuthCommon/Socket.h"
#include "../bzAuthCommon/Protocol.h"
#include <RSA.h>

int sleep_var;

void sleep_thread(void *)
{
    sleep_var = 0;
    while(1)
    {
        if(sleep_var != 0)
        {
            Sleep(sleep_var);
            sleep_var = 0;
        }
        Sleep(1);
    }
}

class TestConnectSocket : public ConnectSocket
{
public:
  TestConnectSocket(SocketHandler *h, const TCPsocket &s) : ConnectSocket(h,s) {}
  TestConnectSocket(SocketHandler *h) : ConnectSocket(h) {}

  void onReadData(PacketHandlerBase *&, Packet &packet)
  {
    switch(packet.getOpcode())
    {
      case DMSG_REGISTER_CHALLENGE: {
        uint8_t *key_n;
        uint32_t e;
        uint16_t n_len;
        assert(packet >> n_len);
        key_n = new uint8_t[n_len];
        packet.read(key_n, (size_t)n_len);
        assert(packet >> e);

        sRSAManager.initialize();
        sRSAManager.getPublicKey().setValues(key_n, (size_t)n_len, e);
        
        char message[] = "newuser password";
        uint8_t *cipher = NULL;
        size_t cipher_len;

        sRSAManager.getPublicKey().encrypt((uint8_t*)message, strlen(message), cipher, cipher_len);

        {
          Packet response(CMSG_REGISTER_RESPONSE, 2 + cipher_len);
          response << (uint16_t)cipher_len;
          response.append(cipher, cipher_len);
          sendData(response);
        }

        sRSAManager.rsaFree(cipher);
        delete[] key_n;
      } break;
      case DMSG_AUTH_CHALLENGE: {
        uint8_t *key_n;
        uint32_t e;
        uint16_t n_len;
        assert(packet >> n_len);
        key_n = new uint8_t[n_len];
        packet.read(key_n, (size_t)n_len);
        assert(packet >> e);

        sRSAManager.initialize();
        sRSAManager.getPublicKey().setValues(key_n, (size_t)n_len, e);
        
        char message[] = "newuser password";
        uint8_t *cipher = NULL;
        size_t cipher_len;

        sRSAManager.getPublicKey().encrypt((uint8_t*)message, strlen(message), cipher, cipher_len);

        {
          Packet response(CMSG_AUTH_RESPONSE, 2 + cipher_len);
          response << (uint16_t)cipher_len;
          response.append(cipher, cipher_len);
          sendData(response);
        }

        sRSAManager.rsaFree(cipher);
        delete[] key_n;
      } break;
      case DMSG_AUTH_SUCCESS:
        uint32_t token;
        packet >> token;
        printf("Auth successful, token %d\n", token); break;
      case DMSG_AUTH_FAIL:
        uint32_t reason;
        packet >> reason;
        printf("Auth failed, reason %d\n", reason); break; 
      case DMSG_REGISTER_SUCCESS:
        printf("Registration successful\n"); break;
      case DMSG_REGISTER_FAIL:
        printf("Registration failed\n"); break;
      default:
        printf("Unexpected opcode %d\n", packet.getOpcode());
    }
  }

  void onDisconnect() { printf("Disconnected\n"); }
};

void test_listen(void *)
{
  
}

void test_random()
{
    /*const int NR_CLIENTS = 10;
    int i;
    TCPClientConnection* client[NR_CLIENTS];
    MyClientListener listener[NR_CLIENTS];

    for(i = 0; i < NR_CLIENTS; i++)
    {
        client[i] = TCPConnection::instance().newClientConnection("127.0.0.1", 1234);
        if(!client[i])
        {
            printf("cannot connect!");
            return;
        }
        client[i]->addListener(&listener[i]);
    }

    //srand((int)time(NULL));
    
    for(i = 0; i < 30; i++)
    {
        //sleep_var = 50 + rand()%250;


        int cli = rand() % 10;
        char msg[1024];
        sprintf(msg, "%d sends %d", cli, rand() % 1000);
        client[cli]->sendData(0, msg);
        while(sleep_var != 0)
            TCPConnection::instance().update();
    }

    _sleep(2000);


    for(i = 0; i < NR_CLIENTS; i++)
    {
        client[i]->disconnect();
        TCPConnection::instance().deleteClientConnection(client[i]);
    }*/
}

void test_comm(uint8_t commType)
{
    SocketHandler *handler = new SocketHandler;
    handler->initialize(32000);
    TestConnectSocket* client = new TestConnectSocket(handler);
    do
    {
      if(kbhit()) return;
      printf("Trying to connect\n");
    }
    while(client->connect("127.0.0.1", 1234) != 0);
    printf("Connected\n");

    {
      uint8_t peerType = BZAUTHD_PEER_CLIENT;
      uint16_t protoVersion = 1;
      uint32_t cliVersion = 2;
      Packet msg(MSG_HANDSHAKE);
      msg << peerType << protoVersion << cliVersion << commType;
      client->sendData(msg);
    }

    while(!kbhit())
    {
      handler->update();
    }

    client->disconnect();
    delete client;
    delete handler;
}

void test_net()
{
    // thread for async sleep
    //_beginthread(sleep_thread, 0, NULL);

    // initialize the net library
    if(!SocketHandler::global_init()) return;

    //test_comm(2) // reg
    test_comm(0); // auth
}
#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8