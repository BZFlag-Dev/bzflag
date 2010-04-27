/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LUA_EVENT_ORDER_H
#define LUA_EVENT_ORDER_H


// 0 is reserved for the server
#define LUA_WORLD_SCRIPT_ID   10
#define LUA_BZORG_SCRIPT_ID   20
#define LUA_USER_SCRIPT_ID    30

// game state events
#define LUA_WORLD_GAME_ORDER  10
#define LUA_BZORG_GAME_ORDER  20
#define LUA_USER_GAME_ORDER   30

// world drawing order
#define LUA_WORLD_DRAW_WORLD_ORDER  10
#define LUA_BZORG_DRAW_WORLD_ORDER  20
#define LUA_USER_DRAW_WORLD_ORDER   30

// screen drawing order  (and input events)
#define LUA_WORLD_DRAW_SCREEN_ORDER  10
#define LUA_BZORG_DRAW_SCREEN_ORDER  20
#define LUA_USER_DRAW_SCREEN_ORDER   30


#endif // LUA_EVENT_ORDER_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
