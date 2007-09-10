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

#ifndef	__REGION_H__
#define	__REGION_H__

#include "common.h"

/* system interface headers */
#include <vector>


const float	     maxDistance = 1.0e6;

class RegionPoint {
  public:
			RegionPoint(float x, float y);
			RegionPoint(const float v[2]);
			~RegionPoint();

    const float*	get() const;

  private:
    float		p[2];
};


class BzfRegion {
  public:
			BzfRegion(int sides, const float p[][2]);
			~BzfRegion();

    bool		isInside(const float p[2]) const;
    // get point distance from Region. Point should be outside Region!
    float		getDistance(const float p[2], float nearest[2]) const;
    int			classify(const float p1[2], const float p2[2]) const;
    BzfRegion*		orphanSplitRegion(const float p1[2], const float p2[2]);

    int			getNumSides() const;
    const RegionPoint&	getCorner(int index) const;
    BzfRegion*		getNeighbor(int index) const;

    bool		test(int mailboxIndex);
    void		setPathStuff(float distance, BzfRegion* target,
					const float p[2], int mailboxIndex);
    float		getDistance() const;
    BzfRegion*		getTarget() const;
    const float*	getA() const;

  protected:
			BzfRegion();
    void		splitEdge(const BzfRegion* oldNeighbor,
					BzfRegion* newNeighbor,
					const RegionPoint& p,
					bool onRight);
    void		addSide(const RegionPoint&, BzfRegion* neighbor);
    void		setNeighbor(const BzfRegion* oldNeighbor,
					BzfRegion* newNeighbor);
    void		tidy();

  private:
    std::vector<RegionPoint>	corners;
    std::vector<BzfRegion*>		neighbors;
    int			mailbox;

    BzfRegion*		target;
    float		distance;
    RegionPoint		A;
};


#endif /* __REGION_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
