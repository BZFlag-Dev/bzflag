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

#ifndef BZF_SCENE_BUILDER_H
#define BZF_SCENE_BUILDER_H

#include "common.h"
#include "BzfString.h"

class Obstacle;
class WallObstacle;
class BoxBuilding;
class PyramidBuilding;
class BaseBuilding;
class Teleporter;
class World;
class SceneNode;
class Matrix;

class SceneDatabaseBuilder {
public:
	SceneDatabaseBuilder();
	~SceneDatabaseBuilder();

	SceneNode*			make(const World*);

protected:
	BzfString			makeBuffer(const World*);
	void				addWall(const WallObstacle&);
	void				addBox(const BoxBuilding&);
	void				addPyramid(const PyramidBuilding&);
	void				addBase(const BaseBuilding&);
	void				addTeleporter(const Teleporter&);

	void				prepMatrix(const Obstacle&, float dz, Matrix&);
	void				prepNormalMatrix(const Matrix&, Matrix&);
	void				addVertex(const Matrix&, const float*);
	void				addVertex(const Matrix&, float x, float y, float z);
	void				addNormal(const Matrix&, const float*);

private:
	// disallow duplication
	SceneDatabaseBuilder(const SceneDatabaseBuilder&);
	SceneDatabaseBuilder& operator=(const SceneDatabaseBuilder&);

private:
	BzfString			normal;
	BzfString			texcoord;
	BzfString			vertex;
	BzfString			primitives1;
	BzfString			primitives2;
	BzfString			primitives3;
	unsigned int		nVertex;
};

#endif // BZF_SCENE_BUILDER_H
