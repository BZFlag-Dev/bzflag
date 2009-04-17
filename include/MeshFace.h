/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* MeshFace:
 *	Encapsulates an individual mesh face in the game environment.
 */

#ifndef	BZF_MESH_FACE_OBSTACLE_H
#define	BZF_MESH_FACE_OBSTACLE_H

#include "common.h"
#include <string>
#include <set>
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
    static const char* getClassName(); // const
  private:
    static const char* typeName;

  public:
    MeshFace(class MeshObstacle* mesh);
    MeshFace(MeshObstacle* mesh, int vertexCount,
	     const fvec3** vertices,
	     const fvec3** normals,
	     const fvec2** texcoords,
	     const BzMaterial* material, int physics,
	     bool noclusters, bool smoothBounce,
	     unsigned char drive, unsigned char shoot, bool ricochet);
    ~MeshFace();

    const char* getType() const;
    bool isValid() const;
    bool isFlatTop() const;

    float intersect(const Ray&) const;
    void getNormal(const fvec3& p, fvec3& n) const;
    void get3DNormal(const fvec3& p, fvec3& n) const;

    bool inCylinder(const fvec3& p, float radius, float height) const;
    bool inBox(const fvec3& p, float angle,
	       float halfWidth, float halfBreadth, float height) const;
    bool inMovingBox(const fvec3& oldP, float oldAngle,
		     const fvec3& newP, float newAngle,
		     float halfWidth, float halfBreadth, float height) const;
    bool isCrossing(const fvec3& p, float angle,
		    float halfWidth, float halfBreadth, float height,
		    fvec4* plane) const;

    bool getHitNormal(const fvec3& pos1, float azimuth1,
		      const fvec3& pos2, float azimuth2,
		      float halfWidth, float halfBreadth,
		      float height, fvec3& normal) const;

    MeshObstacle* getMesh() const;
    inline int  getID() const { return id; }
    int getVertexCount() const;
    bool useNormals() const;
    bool useTexcoords() const;
    const fvec3& getVertex(int index) const;
    const fvec3& getNormal(int index) const;
    const fvec2& getTexcoord(int index) const;
    const fvec4& getPlane() const;
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
    virtual int getTypeID() const {return meshType;}

  public:
    mutable float scratchPad;

  private:
    void finalize();
    inline void setID(int value) { id = value; }

  private:
    class MeshObstacle* mesh;
    int id;
    int vertexCount;
    const fvec3** vertices;
    const fvec3** normals;
    const fvec2** texcoords;
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

    enum SpecialBits {
      LinkToFace   = (1 << 0),
      LinkFromFace = (1 << 1),
      BaseFace     = (1 << 2),
      IcyFace      = (1 << 3),
      StickyFace   = (1 << 5),
      DeathFace    = (1 << 6),
      PortalFace   = (1 << 7)
    };
    uint16_t specialState;

    // combining all types into one struct, because I'm lazy
    typedef struct {
      // linking data
      const MeshFace* linkFace;
      bool linkFromFlat;
      fvec3 linkFromSide; // sideways vector
      fvec3 linkFromDown; // downwards vector
      fvec3 linkFromCenter;
      bool linkToFlat;
      fvec3 linkToSide; // sideways vector
      fvec3 linkToDown; // downwards vector
      fvec3 linkToCenter;
      std::set<MeshFace*> linkDsts;
      // base data
      TeamColor teamColor;
    } SpecialData;

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

inline const fvec4& MeshFace::getPlane() const
{
  return plane;
}

inline int MeshFace::getVertexCount() const
{
  return vertexCount;
}

inline const fvec3& MeshFace::getVertex(int index) const
{
  return *vertices[index];
}

inline const fvec3& MeshFace::getNormal(int index) const
{
  return *normals[index];
}

inline const fvec2& MeshFace::getTexcoord(int index) const
{
  return *texcoords[index];
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
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
