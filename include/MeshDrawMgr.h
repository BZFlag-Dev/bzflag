/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _MESH_DRAW_MGR_H_
#define _MESH_DRAW_MGR_H_

#include "bzfgl.h"
#include "MeshDrawInfo.h"

class MeshDrawMgr {
  public:
    MeshDrawMgr(const MeshDrawInfo* drawInfo);
    ~MeshDrawMgr();

    void executeSet(int lod, int set, bool normals, bool texcoords);
    void executeSetGeometry(int lod, int set);

    static void disableArrays();
    static void init();
    static void kill();

  private:
    void rawExecuteCommands(int lod, int set);
    static void rawDisableArrays();

    void makeLists();
    void freeLists();
    static void initContext(void* data);
    static void freeContext(void* data);

  private:
    const MeshDrawInfo* drawInfo;

    const DrawLod* drawLods;
    const GLfloat* vertices;
    const GLfloat* normals;
    const GLfloat* texcoords;

    struct LodList {
      int count;
      GLuint* setLists;
    };

    int lodCount;
    LodList* lodLists;

    static GLuint unloadList;
};

#endif // _MESH_DRAW_MGR_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
