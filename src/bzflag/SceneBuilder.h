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

/*
 * SceneDatabaseBuilder:
 *  Encapsulates creation of display database
 */

#ifndef BZF_SCENE_BUILDER_H
#define BZF_SCENE_BUILDER_H

#include "common.h"

// common headers
#include "global.h"
#include "vectors.h"
#include "OpenGLMaterial.h"


class SceneRenderer;
class SceneDatabase;
class WallObstacle;
class MeshObstacle;
class BoxBuilding;
class PyramidBuilding;
class BaseBuilding;
class Teleporter;
class World;
class BzMaterial;


class SceneDatabaseBuilder {
  public:
    SceneDatabaseBuilder();
    ~SceneDatabaseBuilder();

    SceneDatabase* make(const World*);

  protected:
    void addWall(SceneDatabase*, const WallObstacle&);
    void addMesh(SceneDatabase*, MeshObstacle*);
    void addBox(SceneDatabase*, BoxBuilding&);
    void addPyramid(SceneDatabase*, PyramidBuilding&);
    void addBase(SceneDatabase*, BaseBuilding&);
    void addTeleporter(SceneDatabase*, const Teleporter&, const World*);
    void addWaterLevel(SceneDatabase*, const World*);
    void addWorldTexts(SceneDatabase*);

  private:
    // disallow duplication
    SceneDatabaseBuilder(const SceneDatabaseBuilder&);
    SceneDatabaseBuilder& operator=(const SceneDatabaseBuilder&);

  private:
    float wallTexWidth, wallTexHeight;
    bool  wallLOD;

    float boxTexWidth, boxTexHeight;
    bool  boxLOD;

    bool  pyramidLOD;

    bool  baseLOD;

    bool  teleporterLOD;

    /* FIXME - find the default materials before generating?
        const BzMaterial* wallMaterial;
        const BzMaterial* pyramidMaterial;
        const BzMaterial* boxTopMaterial;
        const BzMaterial* boxWallMaterial;
        const BzMaterial* baseTopMaterials[5];
        const BzMaterial* baseWallMaterials[5];
    */
};

#endif // BZF_SCENE_BUILDER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
