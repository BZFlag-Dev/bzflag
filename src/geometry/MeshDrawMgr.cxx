/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

// implementation header
#include "MeshDrawMgr.h"

// common headers
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "MeshDrawInfo.h"
#include "bzfio.h" // for DEBUGx()


GLuint MeshDrawMgr::unloadList = INVALID_GL_LIST_ID;


void MeshDrawMgr::init()
{
}


void MeshDrawMgr::kill()
{
}


MeshDrawMgr::MeshDrawMgr(const MeshDrawInfo* _drawInfo)
{
  drawInfo = _drawInfo;
  if ((drawInfo == NULL) || !drawInfo->isValid()) {
    printf("MeshDrawMgr: invalid drawInfo\n");
    fflush(stdout);
    return;
  } else {
    DEBUG4("MeshDrawMgr: initializing\n");
    fflush(stdout);
  }

  drawLods = drawInfo->getDrawLods();
  vertices = (const GLfloat*)drawInfo->getVertices();
  normals = (const GLfloat*)drawInfo->getNormals();
  texcoords = (const GLfloat*)drawInfo->getTexcoords();

  lodCount = drawInfo->getLodCount();
  lodLists = new LodList[lodCount];

  for (int i = 0; i < lodCount; i++) {
    LodList& lodList = lodLists[i];
    lodList.count = drawLods[i].count;
    lodList.setLists = new GLuint[lodList.count];
    for (int j = 0; j < lodList.count; j++) {
      lodList.setLists[j] = INVALID_GL_LIST_ID;
    }
  }

  makeLists();
  OpenGLGState::registerContextInitializer(freeContext, initContext, this);
  return;
}


MeshDrawMgr::~MeshDrawMgr()
{
  DEBUG4("MeshDrawMgr: killing\n");

  OpenGLGState::unregisterContextInitializer(freeContext, initContext, this);
  freeLists();

  for (int i = 0; i < lodCount; i++) {
    delete[] lodLists[i].setLists;
  }
  delete[] lodLists;

  return;
}


inline void MeshDrawMgr::rawExecuteCommands(int lod, int set)
{
  const DrawLod& drawLod = drawLods[lod];
  const DrawSet& drawSet = drawLod.sets[set];
  const int cmdCount = drawSet.count;
  for (int i = 0; i < cmdCount; i++) {
    const DrawCmd& cmd = drawSet.cmds[i];
    glDrawElements(cmd.drawMode, cmd.count, cmd.indexType, cmd.indices);
  }
  return;
}


void MeshDrawMgr::executeSet(int lod, int set, bool _normals, bool _texcoords)
{
  // FIXME
  const AnimationInfo* animInfo = drawInfo->getAnimationInfo();
  if (animInfo != NULL) {
    glPushMatrix();
    glRotatef(animInfo->angle, 0.0f, 0.0f, 1.0f);
  }

  const GLuint list = lodLists[lod].setLists[set];
  if (list != INVALID_GL_LIST_ID) {
    glCallList(list);
  } else {
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glEnableClientState(GL_VERTEX_ARRAY);
    if (_normals) {
      glNormalPointer(GL_FLOAT, 0, normals);
      glEnableClientState(GL_NORMAL_ARRAY);
    }
    if (_texcoords) {
      glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    rawExecuteCommands(lod, set);
    disableArrays();
  }

  if (animInfo != NULL) {
    glPopMatrix();
  }

  return;
}


inline void MeshDrawMgr::rawDisableArrays()
{
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  return;
}


void MeshDrawMgr::disableArrays()
{
  if (unloadList == INVALID_GL_LIST_ID) {
    rawDisableArrays();
  } else {
    glCallList(unloadList);
  }
  return;
}


void MeshDrawMgr::makeLists()
{
  int errCount = 0;
  // reset the error state
  while (glGetError() != GL_NO_ERROR) {
    errCount++; // avoid a possible spin-lock?
    if (errCount > 666) {
      DEBUG1("MeshDrawMgr::makeLists() glError\n");
      return; // don't make the lists, something is borked
    }
  };

  if (unloadList == INVALID_GL_LIST_ID) {
    unloadList = glGenLists(1);
    glNewList(unloadList, GL_COMPILE);
    {
      disableArrays();
    }
    glEndList();
    if (glGetError() != GL_NO_ERROR) {
      DEBUG1("MeshDrawMgr::makeLists() glError\n");
      unloadList = INVALID_GL_LIST_ID;
    }
  }

  glVertexPointer(3, GL_FLOAT, 0, vertices);
  glEnableClientState(GL_VERTEX_ARRAY);
  glNormalPointer(GL_FLOAT, 0, normals);
  glEnableClientState(GL_NORMAL_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  for (int lod = 0; lod < lodCount; lod++) {
    const DrawLod& drawLod = drawLods[lod];
    for (int set = 0; set < drawLod.count; set++) {
      const DrawSet& drawSet = drawLod.sets[set];
      if (drawSet.wantList) {
	lodLists[lod].setLists[set] = glGenLists(1);

	glNewList(lodLists[lod].setLists[set], GL_COMPILE);
	{
	  rawExecuteCommands(lod, set);
	}
	glEndList();

	if (glGetError() != GL_NO_ERROR) {
	  DEBUG1("MeshDrawMgr::makeLists() %i/%i glError\n", lod, set);
	  lodLists[lod].setLists[set] = INVALID_GL_LIST_ID;
	} else {
	  DEBUG3("MeshDrawMgr::makeLists() %i/%i created\n", lod, set);
	}
      }
    }
  }

  disableArrays();

  return;
}


void MeshDrawMgr::freeLists()
{
  if (unloadList != INVALID_GL_LIST_ID) {
    glDeleteLists(unloadList, 1);
    unloadList = INVALID_GL_LIST_ID;
  }

  for (int lod = 0; lod < lodCount; lod++) {
    for (int set = 0; set < lodLists[lod].count; set++) {
      if (lodLists[lod].setLists[set] != INVALID_GL_LIST_ID) {
	glDeleteLists(lodLists[lod].setLists[set], 1);
	lodLists[lod].setLists[set] = INVALID_GL_LIST_ID;
      }
    }
  }

  return;
}


void MeshDrawMgr::initContext(void* data)
{
  ((MeshDrawMgr*)data)->makeLists();
  return;
}


void MeshDrawMgr::freeContext(void* data)
{
  ((MeshDrawMgr*)data)->freeLists();
  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

