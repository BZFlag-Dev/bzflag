/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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

#ifndef	BZF_MESH_FACE_OBSTACLE_H
#define	BZF_MESH_FACE_OBSTACLE_H

#include "common.h"
#include <string>
#include <iostream>
#include "vectors.h"
#include "Ray.h"
#include "Obstacle.h"
#include "global.h"
#include "BzMaterial.h"
//#include "PhysicsDrive.h"


class MeshFace : public Obstacle {

  friend class MeshObstacle;
  friend class ObstacleModifier;

  public:
    MeshFace(class MeshObstacle* mesh);
    MeshFace(MeshObstacle* mesh, int vertexCount,
	     float** vertices, float** normals, float** texcoords,
	     const BzMaterial* material, int physics,
	     bool noclusters, bool smoothBounce, bool drive, bool shoot);
    ~MeshFace();

    const char* getType() const;
    static const char* getClassName(); // const
    bool isValid() const;
    bool isFlatTop() const;

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

    MeshObstacle* getMesh() const;
    int getVertexCount() const;
    bool useNormals() const;
    bool useTexcoords() const;
    const float* getVertex(int index) const;
    const float* getNormal(int index) const;
    const float* getTexcoord(int index) const;
    const float* getPlane() const;
    const BzMaterial* getMaterial() const;
    int getPhysicsDriver() const;
    bool noClusters() const;
    bool isSmoothBounce() const;

    bool isSpecial() const;
    bool isBaseFace() const;
    bool isLinkToFace() const;
    bool isLinkFromFace() const;
    bool isZPlane() const;
    bool isUpPlane() const;
    bool isDownPlane() const;

    void setLink(const MeshFace* link);
    const MeshFace* getLink() const;

    int packSize() const;
    void *pack(void*) const;
    void *unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  public:
    mutable float scratchPad;

  private:
    void finalize();

  private:
    static const char* typeName;

    class MeshObstacle* mesh;
    int vertexCount;
    float** vertices;
    float** normals;
    float** texcoords;
    const BzMaterial* bzMaterial;
    bool smoothBounce;
    bool noclusters;
    int phydrv;

    fvec4 plane;
    fvec4* edgePlanes;

    MeshFace* edges; // edge 0 is between vertex 0 and 1, etc...
		     // not currently used for anything

    enum {
      XPlane    = (1 << 0),
      YPlane    = (1 << 1),
      ZPlane    = (1 << 2),
      UpPlane   = (1 << 3),
      DownPlane = (1 << 4),
      WallPlane = (1 << 5)
    } PlaneBits;

    char planeBits;


    enum {
      LinkToFace      = (1 << 0),
      LinkFromFace    = (1 << 1),
      BaseFace	      = (1 << 2),
      IcyFace	 = (1 << 3),
      StickyFace      = (1 << 5),
      DeathFace       = (1 << 6),
      PortalFace      = (1 << 7)
    } SpecialBits;

    // combining all types into one struct, because I'm lazy
    typedef struct {
      // linking data
      const MeshFace* linkFace;
      bool linkFromFlat;
      float linkFromSide[3]; // sideways vector
      float linkFromDown[3]; // downwards vector
      float linkFromCenter[3];
      bool linkToFlat;
      float linkToSide[3]; // sideways vector
      float linkToDown[3]; // downwards vector
      float linkToCenter[3];
      // base data
      TeamColor teamColor;
    } SpecialData;

    uint16_t specialState;
    SpecialData* specialData;
};

inline MeshObstacle* MeshFace::getMesh() const
{
  return mesh;
}

inline const BzMaterial* MeshFace::getMaterial() const
{
  return bzMaterial;
}

inline int MeshFace::getPhysicsDriver() const
{
  return phydrv;
}

inline const float* MeshFace::getPlane() const
{
  return plane;
}

inline int MeshFace::getVertexCount() const
{
  return vertexCount;
}

inline const float* MeshFace::getVertex(int index) const
{
  return (const float*)vertices[index];
}

inline const float* MeshFace::getNormal(int index) const
{
  return (const float*)normals[index];
}

inline const float* MeshFace::getTexcoord(int index) const
{
  return (const float*)texcoords[index];
}

inline bool MeshFace::useNormals() const
{
  return (normals != NULL);
}

inline bool MeshFace::useTexcoords() const
{
  return (texcoords != NULL);
}

inline bool MeshFace::noClusters() const
{
  return noclusters;
}

inline bool MeshFace::isSmoothBounce() const
{
  return smoothBounce;
}

inline bool MeshFace::isSpecial() const
{
  return (specialState != 0);
}

inline bool MeshFace::isBaseFace() const
{
  return (specialState & BaseFace) != 0;
}

inline bool MeshFace::isLinkToFace() const
{
  return (specialState & LinkToFace) != 0;
}

inline bool MeshFace::isLinkFromFace() const
{
  return (specialState & LinkFromFace) != 0;
}

inline const MeshFace* MeshFace::getLink() const
{
  return specialData->linkFace;
}

inline bool MeshFace::isZPlane() const
{
  return ((planeBits & ZPlane) != 0);
}

inline bool MeshFace::isUpPlane() const
{
  return ((planeBits & UpPlane) != 0);
}

inline bool MeshFace::isDownPlane() const
{
  return ((planeBits & DownPlane) != 0);
}


#endif // BZF_MESH_FACE_OBSTACLE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

