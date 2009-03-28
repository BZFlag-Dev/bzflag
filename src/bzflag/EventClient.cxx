/* bzflag
* Copyright (c) 1993 - 2009 Tim Riker
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

// implementation header
#include "EventClient.h"


//============================================================================//
//============================================================================//

EventClient::EventClient(const std::string& _name, int _scriptID,
                         int gameState, int drawWorld, int drawScreen,
                         bool _fullRead, bool _gameCtrl, bool _inputCtrl)
: clientName      (_name)
, scriptID        (_scriptID)
, gameStateOrder  (gameState)
, drawWorldOrder  (drawWorld)
, drawScreenOrder (drawScreen)
, fullRead        (_fullRead)
, gameCtrl        (_gameCtrl)
, inputCtrl       (_inputCtrl)
{
}


EventClient::~EventClient()
{
}


//============================================================================//
//============================================================================//
