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

#include "SceneNodeGeometry.h"
#include "SceneVisitor.h"
#include "SceneVisitorParams.h"
#include <math.h>
#include <assert.h>

//
// SceneNodeGeometryBundle
//

SceneNodeGeometryBundle::SceneNodeGeometryBundle() :
								stipple("stipple", 0, 0, 1),
								color("color", 0, 0, 4),
								texcoord("texcoord", 0, 0, 2),
								normal("normal", 0, 0, 3),
								vertex("vertex", 0, 0, 3)
{
	// do nothing
}


//
// SceneNodeGeometry
//

const unsigned int  SceneNodeGeometry::kComputedIndex = 0xffffffffu;

SceneNodeGeometry::SceneNodeGeometry()
{
	// make one scratch bundle
	addScratchBundle();
	currentScratch = 0;

	// point to scratch bundle
	current = kComputedIndex;
	setScratchBundle();
}

SceneNodeGeometry::~SceneNodeGeometry()
{
	// destroy scratch bundles
	for (BundlePtrs::iterator index = scratch.begin();
								index != scratch.end(); ++index)
		delete *index;
}

void					SceneNodeGeometry::setBundle(unsigned int index)
{
	if (index == kComputedIndex) {
		stipple  = currentStipple;
		color    = currentColor;
		texcoord = currentTexcoord;
		normal   = currentNormal;
		vertex   = currentVertex;
	}
	else {
		assert(index < bundles.size());
		stipple  = &(bundles[stippleTable[index]].stipple);
		color    = &(bundles[colorTable[index]].color);
		texcoord = &(bundles[texcoordTable[index]].texcoord);
		normal   = &(bundles[normalTable[index]].normal);
		vertex   = &(bundles[vertexTable[index]].vertex);
	}
	current = index;
}

unsigned int			SceneNodeGeometry::getBundle() const
{
	return current;
}

unsigned int			SceneNodeGeometry::getNumBundles() const
{
	return bundles.size();
}

void					SceneNodeGeometry::pushBundle()
{
	// add bundle
	const unsigned int index = bundles.size();
	bundles.push_back(SceneNodeGeometryBundle());
	stippleTable.push_back(index);
	colorTable.push_back(index);
	texcoordTable.push_back(index);
	normalTable.push_back(index);
	vertexTable.push_back(index);

	// watch for changes to the vertex list
	bundles.back().vertex.setDirtyFlag(&boundingBoxDirty);

	// bounding box is dirty
	boundingBoxDirty = true;

	// start using bundle
	setBundle(index);
}

void					SceneNodeGeometry::popBundle()
{
	bundles.pop_back();
	stippleTable.pop_back();
	colorTable.pop_back();
	texcoordTable.pop_back();
	normalTable.pop_back();
	vertexTable.pop_back();
	if (current == bundles.size())
		if (current == 0)
			setBundle(kComputedIndex);
		else
			setBundle(current - 1);
	boundingBoxDirty = true;
}

void					SceneNodeGeometry::clearBundles()
{
	bundles.clear();
	stippleTable.clear();
	colorTable.clear();
	texcoordTable.clear();
	normalTable.clear();
	vertexTable.clear();
	setBundle(kComputedIndex);
	boundingBoxDirty = true;
}

void					SceneNodeGeometry::refBundle(
								Property property,
								unsigned int ref)
{
	// must always reference an earlier bundle
	assert(current != kComputedIndex);
	assert(ref <= current);

	IndirectionTable* table;
	switch (property) {
		case Stipple:
			table = &stippleTable;
			break;

		case Color:
			table = &colorTable;
			break;

		case TexCoord:
			table = &texcoordTable;
			break;

		case Normal:
			table = &normalTable;
			break;

		case Vertex:
			table = &vertexTable;
			break;

		default:
			assert(0 && "invalid property");
			return;
	}

	// dereference until we find a direct entry
	if (ref != current) {
		unsigned int deref = (*table)[ref];
		while (deref != ref) {
			ref   = deref;
			deref = (*table)[ref];
		}
	}

	// save reference
	(*table)[current] = ref;

	// update pointers
	setBundle(current);
}

unsigned int			SceneNodeGeometry::getBundleRef(
								Property property) const
{
	assert(current != kComputedIndex);

	switch (property) {
		case Stipple:
			return stippleTable[current];

		case Color:
			return colorTable[current];

		case TexCoord:
			return texcoordTable[current];

		case Normal:
			return normalTable[current];

		case Vertex:
			return vertexTable[current];

		default:
			assert(0 && "invalid property");
			return 0;
	}
}

void					SceneNodeGeometry::getBoundingBox(BoundingBox* box)
{
	assert(box != NULL);

	if (boundingBoxDirty) {
		boundingBoxDirty = false;

		// find first bundle with some vertices
		Bundles::iterator index = bundles.begin();
		while (index != bundles.end() && index->vertex.getNum() < 3)
			++index;

		// if no vertices then use an empty bounding box
		if (index == bundles.end()) {
			boundingBox.set(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		}

		else {
			// find extents over all bundles
			float xMin = index->vertex.get(0);
			float yMin = index->vertex.get(1);
			float zMin = index->vertex.get(2);
			float xMax = xMin;
			float yMax = yMin;
			float zMax = zMin;
			for (index = bundles.begin(); index != bundles.end(); ++index) {
				const float* v = index->vertex.get();
				const unsigned int n = index->vertex.getNum();
				for (unsigned int i = 0; i < n; i += 3) {
					if (xMin > v[i + 0])
						xMin = v[i + 0];
					else if (xMax < v[i + 0])
						xMax = v[i + 0];

					if (yMin > v[i + 1])
						yMin = v[i + 1];
					else if (yMax < v[i + 1])
						yMax = v[i + 1];

					if (zMin > v[i + 2])
						zMin = v[i + 2];
					else if (zMax < v[i + 2])
						zMax = v[i + 2];
				}
			}
			boundingBox.set(xMin, yMin, zMin, xMax, yMax, zMax);
		}
	}
	*box = boundingBox;
}

void					SceneNodeGeometry::resetScratchBundles()
{
	currentScratch = 0;
	setScratchBundle();
}

void					SceneNodeGeometry::nextScratchBundle()
{
	// go to next bundle.  if there isn't one then make it.
	if (++currentScratch == scratch.size())
		addScratchBundle();
	setScratchBundle();
}

void					SceneNodeGeometry::addScratchBundle()
{
	scratch.push_back(new SceneNodeGeometryBundle);
}

void					SceneNodeGeometry::setScratchBundle()
{
	if (current == kComputedIndex) {
		currentStipple  = &(scratch[currentScratch]->stipple);
		currentColor    = &(scratch[currentScratch]->color);
		currentTexcoord = &(scratch[currentScratch]->texcoord);
		currentNormal   = &(scratch[currentScratch]->normal);
		currentVertex   = &(scratch[currentScratch]->vertex);
		setBundle(current);
	}
}

#define GET_T(__t)									\
	if (__t != tName) {								\
		tName = __t;								\
		t  = params.getFloat(tName);				\
		if (t < 0.0f)								\
			t = 0.0f;								\
		else if (t > tMax)							\
			t = tMax;								\
		t0 = floorf(t);								\
		t -= t0;									\
		index = static_cast<unsigned int>(t0);		\
		exact = (t == 0.0f);						\
	}

void					SceneNodeGeometry::compute(
								const SceneVisitorParams& params)
{
	// special case -- no bundles
	if (bundles.size() == 0) {
		currentStipple  = &(scratch[currentScratch]->stipple);
		currentColor    = &(scratch[currentScratch]->color);
		currentTexcoord = &(scratch[currentScratch]->texcoord);
		currentNormal   = &(scratch[currentScratch]->normal);
		currentVertex   = &(scratch[currentScratch]->vertex);
		currentStipple->resize(0);
		currentColor->resize(0);
		currentTexcoord->resize(0);
		currentNormal->resize(0);
		currentVertex->resize(0);
		return;
	}

	static BzfString emptyName;
	float t = 0.0f, t0;
	const float tMax = static_cast<float>(bundles.size() - 1);
	bool exact = false;
	unsigned int index = 0;
	BzfString tName = emptyName;

	// interpolate, checking for special case of interpolating a
	// field with itself.
	GET_T(bundles[0].stipple.getInterpolationParameter());
	if (exact || stippleTable[index] == stippleTable[index + 1]) {
		currentStipple = &(bundles[stippleTable[index]].stipple);
	}
	else {
		currentStipple = &(scratch[currentScratch]->stipple);
		interpolate(*currentStipple, t, bundles[stippleTable[index]].stipple,
								bundles[stippleTable[index + 1]].stipple);
	}

	GET_T(bundles[0].color.getInterpolationParameter());
	if (exact || colorTable[index] == colorTable[index + 1]) {
		currentColor = &(bundles[colorTable[index]].color);
	}
	else {
		currentColor = &(scratch[currentScratch]->color);
		interpolate(*currentColor, t, bundles[colorTable[index]].color,
								bundles[colorTable[index + 1]].color);
	}

	GET_T(bundles[0].texcoord.getInterpolationParameter());
	if (exact || texcoordTable[index] == texcoordTable[index + 1]) {
		currentTexcoord = &(bundles[texcoordTable[index]].texcoord);
	}
	else {
		currentTexcoord = &(scratch[currentScratch]->texcoord);
		interpolate(*currentTexcoord, t, bundles[texcoordTable[index]].texcoord,
								bundles[texcoordTable[index + 1]].texcoord);
	}

	GET_T(bundles[0].normal.getInterpolationParameter());
	if (exact || normalTable[index] == normalTable[index + 1]) {
		currentNormal = &(bundles[normalTable[index]].normal);
	}
	else {
		currentNormal = &(scratch[currentScratch]->normal);
		interpolate(*currentNormal, t, bundles[normalTable[index]].normal,
								bundles[normalTable[index + 1]].normal);
	}

	GET_T(bundles[0].vertex.getInterpolationParameter());
	if (exact || vertexTable[index] == vertexTable[index + 1]) {
		currentVertex = &(bundles[vertexTable[index]].vertex);
	}
	else {
		currentVertex = &(scratch[currentScratch]->vertex);
		interpolate(*currentVertex, t, bundles[vertexTable[index]].vertex,
								bundles[vertexTable[index + 1]].vertex);
	}
}

bool					SceneNodeGeometry::visit(SceneVisitor* visitor)
{
	return visitor->visit(this);
}

void					SceneNodeGeometry::interpolate(
								SceneNodeVFFloat& d, float t,
								const SceneNodeVFFloat& a,
								const SceneNodeVFFloat& b) const
{
	assert(a.getNum() == b.getNum());

	const float t1 = 1.0f - t;
	const unsigned int n = a.getNum();
	if (d.getNum() < n)
		d.resize(n);
	for (unsigned int i = 0; i < n; ++i)
		d.set(i, t1 * a.get(i) + t * b.get(i));
}
