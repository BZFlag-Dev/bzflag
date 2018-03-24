/* bzflag
 * Copyright (c) 2019-2020 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_VBO_MANAGER_H
#define BZF_VBO_MANAGER_H

// System headers
#include <list>

// Common headers
#include "VBO_Handler.h"

// An abstract class for a client that use GL buffers
// the initVBO method will be called when it is time to (re)initialize
// the GL buffers
// It needs to register/unregister from the VBO_Manager to start/stop
// this mechanism

class VBOclient
{
public:
    virtual ~VBOclient();

    // will be called when GL buffers are (re)created
    // All the static content of the GL buffer can be loaded here,
    // but it will be lost when the GL context is recreated.
    // No allocation should be undone here as the entire content
    // of the GL buffer is declared automatically free
    virtual void initVBO();
};

// A class to informs both
// the clients (VBOclient) and
// the GLbuffer owners (VBO_Handler)
// about the reinizialization of the GL context
// Could be a singleton
class VBO_Manager
{
public:
    VBO_Manager();
    ~VBO_Manager();

    // registration methods for (GL buffers) handlers (VBO_Handler)
    // This class will call
    // VBO_Handler::init() on context creation
    // VBO_Handler::destroy on context deletion
    void registerHandler(VBO_Handler *handler);
    void unregisterHandler(VBO_Handler *handler);

    // registration methods for client (VBOclient)
    // This class will call
    // VBOClient::initVBO() on context creation but
    // after the GL buffers handlers are reset
    void registerClient(VBOclient *client);
    void unregisterClient(VBOclient *client);

private:
    // Those are callback for GL Context creation/free
    // Called by OpenGLGState::
    // data is going to be the VBO_Manager
    // initContext will call VBO_Manager::initAll()
    // freeContext will call VBO_Manager::destroyAll()
    static void initContext(void* data);
    static void freeContext(void* data);

    // will init or reset all clients/handlers
    void initAll();
    void destroyAll();

    std::list<VBO_Handler*> handlerList;
    std::list<VBOclient*> clientList;

    // This will trace if the context is ready or not
    // It is used when a VBOclient register. The initVBO will be called,
    // unless the context is not ready yet
    static bool glContextReady;
};

extern VBO_Manager vboManager;

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4

