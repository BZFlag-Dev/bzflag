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

#ifndef BZF_REGION_SHAPE_H
#define BZF_REGION_SHAPE_H

#include "common.h"
#include <map>
#include "math3D.h"

class TransformableShape;
class IntersectionPoint;

class RegionShape {
public:
	RegionShape() { }
	virtual ~RegionShape() { }

	// get the volume of the shape.  this should be exact for primitive
	// shapes but can be an estimate for complex shapes.
	virtual Real		getVolume() const = 0;

	// test if a point is inside the shape
	virtual bool		isInside(const Vec3&) const = 0;

	// generate a random point inside the shape.  ideally, every point
	// in the shape has equal probability of being returned but that's
	// not required.
	virtual void		getRandomPoint(Vec3&) const = 0;

private:
	// not implemented
	RegionShape(const RegionShape&);
	RegionShape& operator=(const RegionShape&);
};

class RegionShapePrimitive : public RegionShape {
public:
	RegionShapePrimitive(TransformableShape* adopted);
	virtual ~RegionShapePrimitive();

	// get the region's shape
	TransformableShape*	getShape() const;

	// RegionShape overrides
	virtual Real		getVolume() const;
	virtual bool		isInside(const Vec3&) const;
	virtual void		getRandomPoint(Vec3&) const;

private:
	TransformableShape*	shape;
};

class RegionShapeBExpr : public RegionShape {
public:
	RegionShapeBExpr(RegionShape* aAdopted, RegionShape* bAdopted);
	virtual ~RegionShapeBExpr();

	// RegionShape overrides
	virtual Real		getVolume() const = 0;
	virtual bool		isInside(const Vec3&) const = 0;
	virtual void		getRandomPoint(Vec3&) const = 0;

protected:
	RegionShape*		a() const { return aShape; }
	RegionShape*		b() const { return bShape; }

	Real				estimateIntersectionVolume() const;

private:
	RegionShape*		aShape;
	RegionShape*		bShape;
};

class RegionShapeUnion : public RegionShapeBExpr {
public:
	RegionShapeUnion(RegionShape* aAdopted, RegionShape* bAdopted);
	virtual ~RegionShapeUnion();

	// RegionShape overrides
	virtual Real		getVolume() const;
	virtual bool		isInside(const Vec3&) const;
	virtual void		getRandomPoint(Vec3&) const;

private:
	Real				abVolume;
	Real				aFraction;
};

class RegionShapeDifference : public RegionShapeBExpr {
public:
	RegionShapeDifference(RegionShape* aAdopted, RegionShape* bAdopted);
	virtual ~RegionShapeDifference();

	// RegionShape overrides
	// note -- getRandomPoint() performance drops as abVol/aVol, where
	// aVol is the original volume of a and abVol is the volume of the
	// difference.  therefore do not subtract large volumes if you don't
	// want performance to suffer.  note that if the difference is the
	// empty set then getRandomPoint() will infinitely loop.
	virtual Real		getVolume() const;
	virtual bool		isInside(const Vec3&) const;
	virtual void		getRandomPoint(Vec3&) const;

private:
	Real				abVolume;
};

class RegionShapeIntersection : public RegionShapeBExpr {
public:
	RegionShapeIntersection(RegionShape* aAdopted, RegionShape* bAdopted);
	virtual ~RegionShapeIntersection();

	// RegionShape overrides
	virtual Real		getVolume() const;
	virtual bool		isInside(const Vec3&) const;
	virtual void		getRandomPoint(Vec3&) const;

private:
	Real				abVolume;
};

#endif
