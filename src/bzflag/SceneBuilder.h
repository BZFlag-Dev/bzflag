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
#include <string>

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
	std::string			makeBuffer(const World*);
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
	std::string			color;
	std::string			normal;
	std::string			texcoord;
	std::string			vertex;
	std::string			primitives1;
	std::string			primitives2;
	std::string			primitives3;
	std::string			primitives4;
	unsigned int		nVertex;
};

#endif // BZF_SCENE_BUILDER_H
