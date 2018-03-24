/* bzflag
 * Copyright (c) 1993-2020 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

// inherits from
#include "VBO_Manager.h"

// System headers
#include <vector>

// common interface headers
#include "bzfgl.h"
#include "MeshDrawInfo.h"

class MeshDrawMgr : public VBOclient
{
public:
    MeshDrawMgr(const MeshDrawInfo* drawInfo);
    ~MeshDrawMgr();

    void executeSet(int lod, int set, bool useNormals, bool useTexcoords);
    void initVBO() override;

private:
    const MeshDrawInfo* drawInfo;

    int vboArrayIndex; // this needs some explanation (or encapsulation in VBOclient)

    using LodList = std::vector<int>;
    std::vector<LodList> lodLists;
};


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
