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

#ifndef __REGION_H__
#define __REGION_H__

#include "common.h"

// system headers
#include <vector>

// common headers
#include "vectors.h"




class RegionPoint {
  public:
    inline RegionPoint(float x, float y) : p(x, y) {}
    inline RegionPoint(const fvec2& v)   : p(v)   {}
    inline ~RegionPoint() {}
    inline const fvec2& get() const { return p; }
  private:
    fvec2 p;
};


class BzfRegion {
  public:
    static const float maxDistance;

  public:
    BzfRegion(int sides, const fvec2 p[]);
    ~BzfRegion();

    bool    isInside(const fvec2& p) const;
    // get point distance from Region. Point should be outside Region!
    float   getDistance(const fvec2& p, fvec2& nearest) const;
    int     classify(const fvec2& p1, const fvec2& p2) const;
    BzfRegion*    orphanSplitRegion(const fvec2& p1, const fvec2& p2);

    int     getNumSides() const;
    const RegionPoint&  getCorner(int index) const;
    BzfRegion*    getNeighbor(int index) const;

    bool    test(int mailboxIndex);
    void    setPathStuff(float distance, BzfRegion* target,
                         const fvec2& p, int mailboxIndex);
    float   getDistance() const;
    BzfRegion*    getTarget() const;
    const fvec2&  getA() const;

  protected:
    BzfRegion();
    void    splitEdge(const BzfRegion* oldNeighbor,
                      BzfRegion* newNeighbor,
                      const RegionPoint& p,
                      bool onRight);
    void    addSide(const RegionPoint&, BzfRegion* neighbor);
    void    setNeighbor(const BzfRegion* oldNeighbor,
                        BzfRegion* newNeighbor);
    void    tidy();

  private:
    std::vector<RegionPoint>  corners;
    std::vector<BzfRegion*>   neighbors;
    int     mailbox;

    BzfRegion*    target;
    float   distance;
    RegionPoint   A;
};


#endif /* __REGION_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab expandtab
