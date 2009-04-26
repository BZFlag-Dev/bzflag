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
 *        Encapsulates an individual mesh face in the game environment.
 */

#ifndef BZF_MESH_FACE_OBSTACLE_H
#define BZF_MESH_FACE_OBSTACLE_H

#include "common.h"

// system headers
#include <string>
#include <set>
#include <iostream>

// common headers
#include "global.h"
#include "vectors.h"
#include "Ray.h"
#include "Obstacle.h"
#include "BzMaterial.h"


class LinkPhysics;


class MeshFace : public Obstacle {

  friend class MeshObstacle;
  friend class ObstacleModifier;

  public:
    static const char* getClassName(); // const
  private:
    static const char* typeName;

  //==========================================================================//

  private:
    enum PlaneBits {
      XPlane    = (1 << 0),
      YPlane    = (1 << 1),
      ZPlane    = (1 << 2),
      UpPlane   = (1 << 3),
      DownPlane = (1 << 4),
      WallPlane = (1 << 5)
    };

  //==========================================================================//

  public:
    enum SpecialBits {
      LinkSrcFace       = (1 << 0),
      LinkDstFace       = (1 << 1),
      LinkSrcRebound    = (1 << 2),
      LinkSrcNoGlow     = (1 << 3),
      LinkSrcNoRadar    = (1 << 4),
      LinkSrcNoSound    = (1 << 5),
      LinkSrcNoEffect   = (1 << 6)
    };

    enum LinkGeoBits {
      LinkAutoSscale = (1 << 0),
      LinkAutoTscale = (1 << 1),
      LinkAutoPscale = (1 << 2)
    };

    struct LinkGeometry {
      LinkGeometry()
      : centerIndex(-1) // index to a vertex
      , sDirIndex(-1)   // index to a normal
      , tDirIndex(-1)   // index to a normal
      , pDirIndex(-1)   // index to a normal
      , sScale(1.0f)
      , tScale(1.0f)
      , pScale(0.0f) // note the 0.0f
      , angle(0.0f)
      , bits(LinkAutoSscale | LinkAutoTscale | LinkAutoPscale)
      {}
      int centerIndex;
      int sDirIndex;
      int tDirIndex;
      int pDirIndex;
      fvec3 center;
      fvec3 sDir; // usually points right,   looking towards the plane
      fvec3 tDir; // usually points upwards, looking towards the plane
      fvec3 pDir; // usually the -plane.xyz() for srcs, +plane.xyz() for dsts
      float sScale;
      float tScale;
      float pScale;
      float angle; // calculated
      uint8_t bits;
    };

    struct SpecialData {
      SpecialData()
      : stateBits(0)
      , baseTeam(-1)
      , linkSrcMinSpeed(-MAXFLOAT)
      , linkSrcMaxSpeed(+MAXFLOAT)
      , linkSrcShotBlockBits(0)
      , linkSrcTankBlockBits(0)
      {}

      uint16_t stateBits; // using SpecialBits enum

      int baseTeam;

      std::string  linkName;
      LinkGeometry linkSrcGeo;
      LinkGeometry linkDstGeo;
      float linkSrcMinSpeed;
      float linkSrcMaxSpeed;
      uint8_t linkSrcShotBlockBits;
      uint8_t linkSrcTankBlockBits;
    };

  //==========================================================================//

  public:
    MeshFace(class MeshObstacle* mesh);
    MeshFace(MeshObstacle* mesh, int vertexCount,
             const fvec3** vertices,
             const fvec3** normals,
             const fvec2** texcoords,
             const BzMaterial* material, int physics,
             bool noclusters, bool smoothBounce,
             unsigned char drive, unsigned char shoot, bool ricochet,
             const SpecialData* special = NULL);
    ~MeshFace();

    void setSpecialData(const SpecialData* data);

    const char*  getType() const;
    ObstacleType getTypeID() const { return faceType; }

    bool isValid() const;
    bool isFlatTop() const;

    void clearBaseTeam() {
      if (specialData) {
        specialData->baseTeam = -1;
      }
    }

    // collision routines
    float intersect(const Ray&) const;
    void  getNormal(const fvec3& p, fvec3& n) const;
    void  get3DNormal(const fvec3& p, fvec3& n) const;

    bool  inCylinder(const fvec3& p, float radius, float height) const;
    bool  inBox(const fvec3& p, float angle,
                float halfWidth, float halfBreadth, float height) const;
    bool  inMovingBox(const fvec3& oldPos, float oldAngle,
                      const fvec3& newPos, float newAngle,
                      float halfWidth, float halfBreadth, float height) const;
    bool  isCrossing(const fvec3& p, float angle,
                     float halfWidth, float halfBreadth, float height,
                     fvec4* plane) const;
    bool  getHitNormal(const fvec3& pos1, float azimuth1,
                       const fvec3& pos2, float azimuth2,
                       float halfWidth, float halfBreadth,
                       float height, fvec3& normal) const;

    // team base routines
    fvec3 getRandomPoint() const;

    // teleportation routines
    float getProximity(const fvec3& p, float radius) const;
    float isTeleported(const Ray&) const;
    bool  hasCrossed(const fvec3& oldPos, const fvec3& newPos) const;
    bool  shotCanCross(const fvec3& pos,
                       const fvec3& vel, int team, int shotType) const;
    bool  tankCanCross(const fvec3& pos, // FIXME -- also flagType?
                       const fvec3& vel, int team) const;
    bool  teleportShot(const MeshFace& dstFace, const LinkPhysics&,
                       const fvec3& srcPos, fvec3& dstPos,
                       const fvec3& srcVel, fvec3& dstVel) const;
    bool  teleportTank(const MeshFace& dstFace, const LinkPhysics&,
                       const fvec3& srcPos,    fvec3& dstPos,
                       const fvec3& srcVel,    fvec3& dstVel,
                       const float& srcAngle,  float& dstAngle,
                       const float& srcAngVel, float& dstAngVel) const;

    inline MeshObstacle* getMesh() const { return mesh; }
    inline int  getID()            const { return faceID; }
    inline int  getVertexCount()   const { return vertexCount;  }
    inline bool useNormals()       const { return normals   != NULL; }
    inline bool useTexcoords()     const { return texcoords != NULL; }
    inline int  getPhysicsDriver() const { return phydrv; }
    inline bool noClusters()       const { return noclusters; }
    inline bool isSmoothBounce()   const { return smoothBounce; }
    inline const fvec4& getPlane() const { return plane; }

    inline const fvec3& getVertex(int i)   const { return *vertices[i];  }
    inline const fvec3& getNormal(int i)   const { return *normals[i];   }
    inline const fvec2& getTexcoord(int i) const { return *texcoords[i]; }

    inline bool isZPlane()    const { return ((planeBits & ZPlane)    != 0); }
    inline bool isUpPlane()   const { return ((planeBits & UpPlane)   != 0); }
    inline bool isDownPlane() const { return ((planeBits & DownPlane) != 0); }

    inline const BzMaterial*  getMaterial()    const { return bzMaterial;  }
    inline const SpecialData* getSpecialData() const { return specialData; }

    inline bool isValidVertex(int index) {
      return ((index >= 0) && (index < vertexCount));
    }
    inline bool isValidNormal(int index) {
      return (isValidVertex(index) && (normals != NULL));
    }
    inline bool isValidTexcoord(int index) {
      return (isValidVertex(index) && (texcoords != NULL));
    }

    inline bool isSpecial() const {
      return (specialData != NULL);
    }
    inline bool isBaseFace() const {
      return isSpecial() && (specialData->baseTeam >= 0);
    }
    inline bool isLinkSrc() const {
      return isSpecial() && ((specialData->stateBits & LinkSrcFace) != 0);
    }
    inline bool isLinkDst() const {
      return isSpecial() && ((specialData->stateBits & LinkDstFace) != 0);
    }
    inline bool isLinkFace() const {
      return isSpecial() &&
             ((specialData->stateBits & (LinkSrcFace | LinkDstFace)) != 0);
    }
    inline bool linkSrcRebound() const {
      return isSpecial() && ((specialData->stateBits & LinkSrcRebound) != 0);
    }
    inline bool linkSrcNoGlow() const {
      return isSpecial() && ((specialData->stateBits & LinkSrcNoGlow) != 0);
    }
    inline bool linkSrcNoRadar() const {
      return isSpecial() && ((specialData->stateBits & LinkSrcNoRadar) != 0);
    }
    inline bool linkSrcNoSound() const {
      return isSpecial() && ((specialData->stateBits & LinkSrcNoSound) != 0);
    }
    inline bool linkSrcNoEffect() const {
      return isSpecial() && ((specialData->stateBits & LinkSrcNoEffect) != 0);
    }

    inline std::string getLinkName() const {
      if (!isSpecial() || (mesh == NULL)) { return ""; }
      return ((Obstacle*)mesh)->getName() + ":" + specialData->linkName;
    }

    static inline const MeshFace* getShotLinkSrc(const Obstacle* obs) {
      if ((obs == NULL) || (obs->getTypeID() != faceType)) { return NULL; }
      const MeshFace* face = (const MeshFace*) obs;
      if (!face->isLinkSrc())          { return NULL; }
      if (face->isShootThrough() == 0) { return NULL; }
      return face;
    }

    static inline const MeshFace* getTankLinkSrc(const Obstacle* obs) {
      if ((obs == NULL) || (obs->getTypeID() != faceType)) { return NULL; }
      const MeshFace* face = (const MeshFace*) obs;
      if (!face->isLinkSrc())          { return NULL; }
      if (face->isDriveThrough() == 0) { return NULL; }
      return face;
    }

    fvec3 calcCenter() const;
    void  calcAxisSpan(const fvec3& point, const fvec3& dir,
                       float& minDist, float& maxDist) const;

    int getBaseTeam() const {
      return isBaseFace() ? specialData->baseTeam : -1;
    }

    int packSize() const;
    void *pack(void*) const;
    void *unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  //==========================================================================//

  public:
    mutable float scratchPad;

  private:
    void finalize();
    void setupSpecialData();
    void setupSpecialGeometry(LinkGeometry& geo, bool isSrc);
    inline void setID(int id) { faceID = id; }

  private:
    class MeshObstacle* mesh;
    int faceID;
    int vertexCount;
    const fvec3** vertices;
    const fvec3** normals;
    const fvec2** texcoords;
    const BzMaterial* bzMaterial;
    bool  smoothBounce;
    bool  noclusters;
    int   phydrv;

    fvec4  plane;
    fvec4* edgePlanes;

    MeshFace* edges; // edge 0 is between vertex 0 and 1, etc...
                     // not currently used for anything

    char planeBits;

    SpecialData* specialData;
};



#endif // BZF_MESH_FACE_OBSTACLE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
