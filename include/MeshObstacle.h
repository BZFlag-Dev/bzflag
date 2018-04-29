/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* TetraBuilding:
 *	Encapsulates a tetrahederon in the game environment.
 */

#ifndef	BZF_MESH_OBSTACLE_H
#define	BZF_MESH_OBSTACLE_H

#include "common.h"
#include <string>
#include <vector>
#include <iostream>
#include "vectors.h"
#include "Ray.h"
#include "Obstacle.h"
#include "MeshFace.h"
#include "MeshTransform.h"

class MeshDrawInfo;

class MeshObstacle : public Obstacle {
  public:
    MeshObstacle();
    MeshObstacle(const MeshTransform& transform,
		 const std::vector<char>& checkTypes,
		 const std::vector<cfvec3>& checkPoints,
		 const std::vector<cfvec3>& vertices,
		 const std::vector<cfvec3>& normals,
		 const std::vector<cfvec2>& texcoords,
		 int faceCount, bool noclusters,
		 bool bounce, bool drive, bool shoot, bool ricochet);

    bool addFace (const std::vector<int>& vertices,
		  const std::vector<int>& normals,
		  const std::vector<int>& texcoords,
		  const BzMaterial* bzMaterial, int physics,
		  bool noclusters, bool bounce, bool drive, bool shoot,
		  bool ricochet, bool triangulate);

    ~MeshObstacle();

    void finalize();

    Obstacle* copyWithTransform(const MeshTransform&) const;
    void copyFace(int face, MeshObstacle* mesh) const;

    void setName(const std::string& name);
    const std::string&	getName() const;

    enum CheckType {
      CheckInside =  0,
      CheckOutside = 1,
      InsideParity = 2,
      OutsidePartiy = 3
    };

    const char* getType() const;
    static const char* getClassName(); // const
    bool isValid() const;

    float intersect(const Ray&) const;
    void getNormal(const float* p, float* n) const;
    void get3DNormal(const float* p, float* n) const;

    bool inCylinder(const float* p, float radius, float height) const;
    bool inBox(const float* p, float angle,
	       float halfWidth, float halfBreadth, float height) const;
    bool inMovingBox(const float* oldP, float oldAngle,
		     const float *newP, float newAngle,
		     float halfWidth, float halfBreadth, float height) const;
    bool isCrossing(const float* p, float angle,
		    float halfWidth, float halfBreadth, float height,
		    float* plane) const;

    bool getHitNormal(const float* pos1, float azimuth1,
		      const float* pos2, float azimuth2,
		      float halfWidth, float halfBreadth,
		      float height, float* normal) const;

    bool containsPoint(const float point[3]) const;
    bool containsPointNoOctree(const float point[3]) const;

    const char *getCheckTypes() const;
    const afvec3 *getCheckPoints() const;
    afvec3 *getVertices() const;
    afvec3 *getNormals() const;
    afvec2 *getTexcoords() const;
    int getCheckCount() const;
    int getVertexCount() const;
    int getNormalCount() const;
    int getTexcoordCount() const;
    int getFaceCount() const;
    MeshFace* getFace(int face) const;
    const float* getPosition() const;
    const float* getSize() const;
    bool useSmoothBounce() const;
    bool noClusters() const;

    MeshDrawInfo* getDrawInfo() const;
    void setDrawInfo(MeshDrawInfo*);

    int packSize() const;
    void *pack(void*) const;
    const void *unpack(const void*);

    void print(std::ostream& out, const std::string& indent) const;
    void printOBJ(std::ostream& out, const std::string& indent) const;

  private:
    void makeFacePointers(const std::vector<int>& _vertices,
			  const std::vector<int>& _normals,
			  const std::vector<int>& _texcoords,
			  float**& v, float**& n, float**& t);

  private:
    static const char* typeName;

    std::string name;

    int checkCount;
    char* checkTypes;
    afvec3* checkPoints;
    int vertexCount;
    afvec3* vertices;
    int normalCount;
    afvec3* normals;
    int texcoordCount;
    afvec2* texcoords;
    int faceCount, faceSize;
    MeshFace** faces;
    bool smoothBounce;
    bool noclusters;
    bool inverted; // used during building. can be ditched if
		   // edge tables are setup with bi-directional
		   // ray-vs-face tests and parity counts.

    MeshDrawInfo* drawInfo; // hidden data stored in extra texcoords
};

inline const char *MeshObstacle::getCheckTypes() const
{
  return checkTypes;
}

inline const afvec3 *MeshObstacle::getCheckPoints() const
{
  return checkPoints;
}

inline afvec3 *MeshObstacle::getVertices() const
{
  return vertices;
}

inline afvec3 *MeshObstacle::getNormals() const
{
  return normals;
}

inline afvec2 *MeshObstacle::getTexcoords() const
{
  return texcoords;
}

inline int MeshObstacle::getCheckCount() const
{
  return checkCount;
}

inline int MeshObstacle::getVertexCount() const
{
  return vertexCount;
}

inline int MeshObstacle::getNormalCount() const
{
  return normalCount;
}

inline int MeshObstacle::getTexcoordCount() const
{
  return texcoordCount;
}

inline int MeshObstacle::getFaceCount() const
{
  return faceCount;
}

inline MeshFace* MeshObstacle::getFace(int face) const
{
  return faces[face];
}

inline const float* MeshObstacle::getPosition() const
{
  return pos;
}

inline const float* MeshObstacle::getSize() const
{
  return size;
}

inline bool MeshObstacle::useSmoothBounce() const
{
  return smoothBounce;
}

inline bool MeshObstacle::noClusters() const
{
  return noclusters;
}

inline MeshDrawInfo* MeshObstacle::getDrawInfo() const
{
  return drawInfo;
}

inline const std::string& MeshObstacle::getName() const
{
  return name;
}

inline void MeshObstacle::setName(const std::string& str)
{
  name = str;
  return;
}

#endif // BZF_MESH_OBSTACLE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
