/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* TetraBuilding:
 *	Encapsulates a tetrahederon in the game environment.
 */

#ifndef	BZF_MESH_FACE_OBSTACLE_H
#define	BZF_MESH_FACE_OBSTACLE_H

#include "common.h"
#include <string>
#include <iostream>
#include "vectors.h"
#include "Ray.h"
#include "Obstacle.h"
#include "MeshMaterial.h"


class MeshFace : public Obstacle {

  friend class MeshObstacle;

  public:
    MeshFace(class MeshObstacle* mesh);
    MeshFace(MeshObstacle* _mesh, int _vertexCount,
             float** _vertices, float** _normals,
             float** _texcoords, const MeshMaterial& _material,
             bool smoothBounce, bool drive, bool shoot);
    ~MeshFace();

    void finalize();

    const char* getType() const;
    static const char* getClassName(); // const
    bool isValid() const;
    void getExtents(float* mins, float* maxs) const;

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

    int getVertexCount() const;
    bool useNormals() const;
    bool useTexcoords() const;
    const float *getVertex(int index) const;
    const float *getNormal(int index) const;
    const float *getTexcoord(int index) const;
    const float *getPlane() const;
    const MeshMaterial *getMaterial() const;

    void *pack(void*);
    void *unpack(void*);
    int packSize();

    void print(std::ostream& out, int level);

  private:
    static const char* typeName;

    class MeshObstacle* mesh;
    int vertexCount;
    float** vertices;
    float** normals;
    float** texcoords;
    MeshMaterial material;
    bool smoothBounce;

    fvec3 mins, maxs;
    fvec4 plane;
    MeshFace* edges; // edge 0 is between vertex 0 and 1, etc...
    fvec4* edgePlanes;
};


inline int MeshFace::getVertexCount() const
{
  return vertexCount;
}

inline const float* MeshFace::getVertex(int index) const
{
  return (const float*)vertices[index];
}

inline const float *MeshFace::getNormal(int index) const
{
  return (const float*)normals[index];
}

inline const float *MeshFace::getTexcoord(int index) const
{
  return (const float*)texcoords[index];
}

inline const float *MeshFace::getPlane() const
{
  return plane;
}

inline const MeshMaterial *MeshFace::getMaterial() const
{
  return &material;
}

inline bool MeshFace::useNormals() const
{
  return (normals != NULL);
}

inline bool MeshFace::useTexcoords() const
{
  return (texcoords != NULL);
}


#endif // BZF_MESH_FACE_OBSTACLE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

