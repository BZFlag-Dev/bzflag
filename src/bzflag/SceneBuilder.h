/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * SceneDatabaseBuilder:
 *	Encapsulates creation of display database
 */

#ifndef	BZF_SCENE_BUILDER_H
#define	BZF_SCENE_BUILDER_H

#include "common.h"
#include "OpenGLMaterial.h"
#include "OpenGLTexture.h"

class SceneRenderer;
class SceneDatabase;
class WallObstacle;
class BoxBuilding;
class PyramidBuilding;
class BaseBuilding;
class Teleporter;
class World;

class SceneDatabaseBuilder {
  public:
			SceneDatabaseBuilder(const SceneRenderer*);
			~SceneDatabaseBuilder();

    SceneDatabase*	make(const World*);

  protected:
    void		addWall(SceneDatabase*, const WallObstacle&);
    void		addBox(SceneDatabase*, const BoxBuilding&);
    void		addPyramid(SceneDatabase*, const PyramidBuilding&);
    void		addBase(SceneDatabase*, const BaseBuilding&);
    void		addTeleporter(SceneDatabase*, const Teleporter&);

  private:
    // disallow duplication
			SceneDatabaseBuilder(const SceneDatabaseBuilder&);
    SceneDatabaseBuilder& operator=(const SceneDatabaseBuilder&);

  private:
    const SceneRenderer	*renderer;

    OpenGLMaterial	wallMaterial;
    OpenGLTexture	wallTexture;
    float		wallTexWidth, wallTexHeight;
    boolean		wallLOD;

    OpenGLMaterial	boxMaterial;
    OpenGLTexture	boxTexture;
    OpenGLTexture	boxTopTexture;
    float		boxTexWidth, boxTexHeight;
    boolean		boxLOD;

    OpenGLMaterial	pyramidMaterial;
    OpenGLTexture	pyramidTexture;
    boolean		pyramidLOD;

    boolean		baseLOD;

    OpenGLMaterial	teleporterMaterial;
    OpenGLTexture	teleporterTexture;
    boolean		teleporterLOD;

    static const GLfloat wallColors[4][4];
    static const GLfloat wallModulateColors[4][4];
    static const GLfloat wallLightedColors[1][4];
    static const GLfloat wallLightedModulateColors[1][4];
    static const GLfloat boxColors[6][4];
    static const GLfloat boxModulateColors[6][4];
    static const GLfloat boxLightedColors[6][4];
    static const GLfloat boxLightedModulateColors[6][4];
    static const GLfloat pyramidColors[5][4];
    static const GLfloat pyramidModulateColors[5][4];
    static const GLfloat pyramidLightedColors[5][4];
    static const GLfloat pyramidLightedModulateColors[5][4];
    static const GLfloat teleporterColors[3][4];
    static const GLfloat teleporterModulateColors[3][4];
    static const GLfloat teleporterLightedColors[3][4];
    static const GLfloat teleporterLightedModulateColors[3][4];
};

#endif // BZF_SCENE_BUILDER_H
// ex: shiftwidth=2 tabstop=8
