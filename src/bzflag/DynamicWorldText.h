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

#ifndef DYNAMIC_WORLD_TEXT_H
#define DYNAMIC_WORLD_TEXT_H

#include "common.h"

/* system interface headers */
#include <string>
#include <set>
#include <map>


class WorldText;
class TextSceneNode;
class SceneRenderer;


class DynamicWorldText {
  public:
    void clear();

    void addRenderNodes(SceneRenderer&);
    void addShadowNodes(SceneRenderer&);
    void renderRadar();

    void addText(const WorldText*); // for worldTexts
    bool insertText(WorldText*);    // for addedTexts
    bool removeText(const std::string& name);

    void notifyStyleChange();

  private:
    DynamicWorldText();
    ~DynamicWorldText();

  private:
    typedef std::set<const WorldText*>                 WorldMap;
    typedef std::map<std::string, WorldText*>          AddedMap;
    typedef std::map<const WorldText*, TextSceneNode*> NodeMap;
    WorldMap worldTexts; // came from the world file, but uses BZDB text
    AddedMap addedTexts; // added dynamically from the server
    NodeMap nodes;
    bool needStyleChange;
  
  public:
    static DynamicWorldText bzdbWorldText;
};


#define DYNAMICWORLDTEXT (DynamicWorldText::bzdbWorldText)


#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
