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

#ifndef	GFX_BLOCK_H
#define	GFX_BLOCK_H

#include "common.h"

// system headers
#include <vector>


class EventClient;


//============================================================================//

class GfxBlock {
  public:
    enum BlockType {
      Global = 0,
      Tank,
      Shot,
      Flag,
      BlockTypeCount
    };

  public:
    GfxBlock();
    GfxBlock(int type, int id, bool world);
    GfxBlock(int id, const char* name, bool world); // for global GfxBlocks
    ~GfxBlock();

    void init(int _type, int _id, bool _world);

    void clear();
    bool set(EventClient* ec, bool queue);
    bool remove(EventClient* ec);

    inline bool blocked()    const { return !clients.empty(); }
    inline bool notBlocked() const { return  clients.empty(); }
    inline bool test(EventClient* ec) const {
      return !clients.empty() && (clients[0] == ec);
    }

    inline int  getType()    const { return type; }
    inline int  getID()      const { return id;   }
    inline bool worldBlock() const { return world; }

    static const char* getTypeString(int type);
    static int getStringType(const char* name);

  private:
    int type;
    int id;
    bool world;
    std::vector<EventClient*> clients;
};


//============================================================================//

class GfxBlockMgr {

  friend class GfxBlock;

  public:
    enum BlockID {

      // world items
      Obstacles = 0,
      Sky,
      Stars,
      Clouds,
      Ground,
      Lights,
      Mirror,
      Shadows,
      Mountains,
      Explosions,
      Insides,
      Halos,
      TrackMarks,
      Weather,
      Effects,

      // UI items
      Cursor,
      Console,
      Radar,
      TeamScores,
      PlayerScores,
      Menu,
      Compose,
      TargetBox,
      ShotStatus,
      Markers,
      Times,
      Labels,
      Cracks,
      Status,
      Clock,
      FlagHelp,
      Alerts,

      BlockIDCount
    };

    static GfxBlock obstacles;
    static GfxBlock sky;
    static GfxBlock stars;
    static GfxBlock clouds;
    static GfxBlock ground;
    static GfxBlock lights;
    static GfxBlock mirror;
    static GfxBlock shadows;
    static GfxBlock mountains;
    static GfxBlock explosions;
    static GfxBlock insides;
    static GfxBlock halos;
    static GfxBlock trackMarks;
    static GfxBlock weather;
    static GfxBlock effects;

    static GfxBlock cursor;
    static GfxBlock console;
    static GfxBlock radar;
    static GfxBlock teamScores;
    static GfxBlock playerScores;
    static GfxBlock menu;
    static GfxBlock compose;
    static GfxBlock targetBox;
    static GfxBlock shotStatus;
    static GfxBlock markers;
    static GfxBlock times;
    static GfxBlock labels;
    static GfxBlock cracks;
    static GfxBlock status;
    static GfxBlock clock;
    static GfxBlock flagHelp;
    static GfxBlock alerts;
    
    static inline GfxBlock* get(int id) {
      if ((id < 0) || (id >= BlockIDCount)) {
        return NULL;
      }
      return blocks[id];
    }

    static void removeClient(EventClient* ec);

    static const char* getIDString(int id);
    static int getStringID(const char* name);

    static void check();

  private:
    static GfxBlock* blocks[BlockIDCount];
};


//============================================================================//

#endif // GFX_BLOCK_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
