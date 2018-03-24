/* bzflag
 * Copyright (c) 2019-2019 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "VBO_Manager.h"

// common implementation header
#include "OpenGLGState.h"
#include "VBO_Handler.h"

VBOclient::~VBOclient()
{
}

void VBOclient::initVBO()
{
}

bool VBO_Manager::glContextReady = false;

VBO_Manager::VBO_Manager ()
{
    OpenGLGState::registerContextInitializer(freeContext, initContext, this);
}

VBO_Manager::~VBO_Manager ()
{
    OpenGLGState::unregisterContextInitializer(freeContext, initContext, this);
}

void VBO_Manager::registerHandler(VBO_Handler *handler)
{
    handlerList.push_back(handler);
}

void VBO_Manager::unregisterHandler(VBO_Handler *handler)
{
    handlerList.remove(handler);
}

void VBO_Manager::registerClient(VBOclient *client)
{
    clientList.push_back(client);
    if (glContextReady)
        client->initVBO();
}

void VBO_Manager::unregisterClient(VBOclient *client)
{
    clientList.remove(client);
}

void VBO_Manager::initAll()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    for (auto handler : handlerList) handler->init();
    for (auto client : clientList) client->initVBO();
}

void VBO_Manager::destroyAll()
{
    for (auto handler : handlerList) handler->destroy();
}

void VBO_Manager::initContext(void* data)
{
    glContextReady = true;
    static_cast<VBO_Manager*>(data)->initAll();
}

void VBO_Manager::freeContext(void* data)
{
    glContextReady = false;
    static_cast<VBO_Manager*>(data)->destroyAll();
}

VBO_Manager vboManager;


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
