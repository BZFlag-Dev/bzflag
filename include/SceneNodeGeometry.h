/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_SCENE_NODE_GEOMETRY_H
#define BZF_SCENE_NODE_GEOMETRY_H

#include "SceneNodeGroup.h"
#include "BoundingBox.h"

class SceneVisitorParams;

class SceneNodeGeometryBundle {
public:
	SceneNodeGeometryBundle();

	SceneNodeVFFloat	color;						// 4-vector * n
	SceneNodeVFFloat	texcoord;					// 2-vector * n
	SceneNodeVFFloat	normal;						// 3-vector * n
	SceneNodeVFFloat	vertex;						// 3-vector * n
};

class SceneNodeGeometry : public SceneNodeGroup {
public:
	static const unsigned int kComputedIndex;
	enum Property { Color, TexCoord, Normal, Vertex };

	SceneNodeGeometry();

	// bundle management.  push() sets the current bundle to the new
	// bundle.  if the current bundle is being popped then pop() sets
	// the current bundle to the new last bundle.
	void				setBundle(unsigned int);
	unsigned int		getBundle() const;
	unsigned int		getNumBundles() const;
	void				pushBundle();
	void				popBundle();
	void				clearBundles();
	void				refBundle(Property, unsigned int refIndex);
	unsigned int		getBundleRef(Property) const;

	// get the axis-aligned bounding box around all bundles.  if there
	// are no vertices then the box is point at the origin.
	void				getBoundingBox(BoundingBox* box);

	// compute() saves interpolated data to scratch space.  the
	// same scratch space is reused until nextScratchBundle() is
	// called, when a new bundle is created.  resetScratchBundles()
	// allows all scratch bundles to be reused.
	void				resetScratchBundles();
	void				nextScratchBundle();

	// compute the current geometry
	void				compute(const SceneVisitorParams&);

	// SceneNode overrides
	virtual bool		visit(SceneVisitor*);

	// current bundle fields (note that these are pointers)
	SceneNodeVFFloat*	color;						// 4-vector * n
	SceneNodeVFFloat*	texcoord;					// 2-vector * n
	SceneNodeVFFloat*	normal;						// 3-vector * n
	SceneNodeVFFloat*	vertex;						// 3-vector * n

protected:
	virtual ~SceneNodeGeometry();

	void				addScratchBundle();
	void				setScratchBundle();
	void				interpolate(SceneNodeVFFloat& dst, float t,
							const SceneNodeVFFloat& a,
							const SceneNodeVFFloat& b) const;

private:
	typedef std::vector<SceneNodeGeometryBundle> Bundles;
	typedef std::vector<SceneNodeGeometryBundle*> BundlePtrs;
	typedef std::vector<int> IndirectionTable;

	unsigned int		current;
	Bundles				bundles;
	BundlePtrs			scratch;
	unsigned int		currentScratch;
	IndirectionTable	colorTable;
	IndirectionTable	texcoordTable;
	IndirectionTable	normalTable;
	IndirectionTable	vertexTable;
	SceneNodeVFFloat*	currentColor;
	SceneNodeVFFloat*	currentTexcoord;
	SceneNodeVFFloat*	currentNormal;
	SceneNodeVFFloat*	currentVertex;
	bool				boundingBoxDirty;
	BoundingBox			boundingBox;
};

#endif
