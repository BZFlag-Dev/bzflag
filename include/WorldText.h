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

#ifndef WORLD_TEXT_H
#define WORLD_TEXT_H

#include "common.h"

/* system interface headers */
#include <string>
#include <vector>
#include <set>
#include <iostream>

/* common interface headers */
#include "MeshTransform.h"


class BzMaterial;


class WorldText {
  public:
    WorldText();
    WorldText(const WorldText& text);
    ~WorldText();

    bool isValid() const;

    int packSize() const;
    void *pack(void* buf) const;
    void *unpack(void* buf);

    void print(std::ostream& out, const std::string& indent) const;

  public:
    std::string name;
    std::string data;
    std::string font;

    float fontSize;
    float justify;  // justification  (0.0 = left, 1.0 = right)
    float lineSpace;
    float fixedWidth;
    float lengthPerPixel;
    
    bool useBZDB;   // 'data' refers to a bzdb variable
    bool billboard;

    const BzMaterial* bzMaterial;

    MeshTransform xform;

    // polygon offset
    float poFactor;
    float poUnits;
};


class WorldTextManager {
  public:
    WorldTextManager();
    ~WorldTextManager();

    void clear();

    int addText(WorldText* text);

    const std::vector<WorldText*>& GetTexts() const { return texts; }

    void getFontURLs(std::set<std::string>& fontURLs) const;

    int packSize() const;
    void *pack(void* buf) const;
    void *unpack(void* buf);

    void print(std::ostream& out, const std::string& indent) const;
    
  private:
    std::vector<WorldText*> texts;
};


extern WorldTextManager WORLDTEXTMGR;


#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
