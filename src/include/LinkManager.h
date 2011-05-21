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

/* OpenGLLight:
 *  Encapsulates an OpenGL (point or directional) light source.
 */

#ifndef BZF_LINK_MANAGER_H
#define BZF_LINK_MANAGER_H

#include "common.h"

// system headers
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>

// common headers
#include "LinkDef.h"
#include "LinkPhysics.h"


class MeshFace;
class FlagType;


class LinkManager {
  public:
    typedef std::vector<int>         IntVec;
    typedef std::vector<std::string> StringVec;

    struct DstData {
      DstData() : face(NULL) {}
      DstData(const MeshFace* f, const LinkPhysics& lp)
        : face(f)
        , physics(lp)
      {}
      bool operator<(const DstData& dd) const;

      const MeshFace* face;
      LinkPhysics  physics;
    };

    struct DstIndexList {
      DstIndexList() : needTest(false) {}
      bool needTest;
      IntVec dstIDs;
    };

    typedef std::map<const MeshFace*, DstIndexList> LinkMap;

    typedef std::vector<DstData>   DstDataVec;
    typedef std::map<DstData, int> DstDataIntMap;

    typedef std::vector<const MeshFace*>   FaceVec;
    typedef std::set<const MeshFace*>      FaceSet;
    typedef std::map<const MeshFace*, int> FaceIntMap;

    struct NameFace {
      NameFace(const std::string& n, const MeshFace* f) : name(n), face(f) {}
      std::string     name;
      const MeshFace* face;
    };
    typedef std::vector<NameFace> NameFaceVec;

  public:
    LinkManager();
    ~LinkManager();

    void doLinking();

    void clear();

    void addLinkDef(const LinkDef& linkDef);

    const MeshFace* getShotLinkDst(const MeshFace* srcLink,
                                   unsigned int seed,
                                   int& linkSrcID, int& linkDstID,
                                   const LinkPhysics*& physics,
                                   const fvec3& pos, const fvec3& vel,
                                   int team, const FlagType* flagType) const;
    const MeshFace* getTankLinkDst(const MeshFace* srcLink,
                                   int& linkSrcID, int& linkDstID,
                                   const LinkPhysics*& physics,
                                   const fvec3& pos, const fvec3& vel,
                                   int team, const FlagType* flagType) const;

    const MeshFace* getLinkSrcFace(int linkSrcID) const;
    const MeshFace* getLinkDstFace(int linkDstID) const;
    const DstData*  getLinkDstData(int linkDstID) const;

    int getLinkSrcID(const MeshFace* linkSrc) const;
    int getLinkDstID(const MeshFace* linkDst, const LinkPhysics& lp) const;

    inline const FaceVec&    getLinkSrcs() const { return linkSrcs; }
    inline const DstDataVec& getLinkDsts() const { return linkDsts; }
    inline const LinkMap&    getLinkMap()  const { return linkMap;  }

    inline const FaceSet& getLinkSrcSet()  const { return linkSrcSet;  }
    inline const FaceSet& getLinkDstSet()  const { return linkDstSet;  }
    inline const FaceSet& getLinkFaceSet() const { return linkFaceSet; }

    void getVariables(std::set<std::string>& vars) const;

  private:
    void buildNameMap();

    bool matchLinks(const StringVec& patterns, FaceSet& faces) const;

    void createLink(const MeshFace* linkSrc,
                    const MeshFace* linkDst, const LinkPhysics& physics);

    void crossLink(); // make sure that all 'teleporter' sourced
    // links are valid (using passthrough links)

    void printDebug();

  private:
    LinkDefVec linkDefs;

    LinkMap linkMap;

    FaceVec    linkSrcs;
    FaceIntMap linkSrcMap;

    DstDataVec    linkDsts;
    DstDataIntMap linkDstMap;

    FaceSet linkSrcSet;
    FaceSet linkDstSet;
    FaceSet linkFaceSet;

    NameFaceVec nameFaceVec;
};


extern LinkManager linkManager;


#endif // BZF_LINK_MANAGER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
