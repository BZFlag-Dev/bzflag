/* bzflag
 * Copyright (c) 1993 - 2000 Tim Riker
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
 *
 */

#ifndef	BZF_REGION_H
#define	BZF_REGION_H

#include "common.h"
#include "AList.h"

class RegionPoint {
  public:
			RegionPoint(float x, float y);
			RegionPoint(const float v[2]);
			~RegionPoint();

    const float*	get() const;

  private:
    float		p[2];
};

class BzfRegion;
BZF_DEFINE_ALIST(RegionPointList, RegionPoint);
BZF_DEFINE_ALIST(RegionList, BzfRegion*);

class BzfRegion {
  public:
			BzfRegion(int sides, const float p[][2]);
			~BzfRegion();

    boolean		isInside(const float p[2]) const;
    int			classify(const float p1[2], const float p2[2]) const;
    BzfRegion*		orphanSplitRegion(const float p1[2], const float p2[2]);

    int			getNumSides() const;
    const RegionPoint&	getCorner(int index) const;
    BzfRegion*		getNeighbor(int index) const;

    boolean		test(int mailboxIndex);
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
					boolean onRight);
    void		addSide(const RegionPoint&, BzfRegion* neighbor);
    void		setNeighbor(const BzfRegion* oldNeighbor,
					BzfRegion* newNeighbor);
    void		tidy();

  private:
    RegionPointList	corners;
    RegionList		neighbors;
    int			mailbox;

    BzfRegion*		target;
    float		distance;
    RegionPoint		A;
};

class RegionPriorityQueue {
  public:
			RegionPriorityQueue();
			~RegionPriorityQueue();
    void		insert(BzfRegion* region, float priority);
    BzfRegion*		remove();
    void		removeAll();
    boolean		isEmpty() const;

  private:
    struct Node {
      public:
			Node(BzfRegion* region, float priority);
      public:
	Node*		next;
	BzfRegion*	region;
	float		priority;
    };

  private:
    Node*		head;
};

#endif // BZF_REGION_H
