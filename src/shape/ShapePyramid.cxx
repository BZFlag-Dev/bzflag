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

#include "ShapePyramid.h"
#include "ContactSurfacePoint.h"
#include "ContactSurfaceEdge.h"
#include "ContactSurfacePolygon.h"

//
// ShapePyramid
//

// map face to vertices of face in CCW order (from the outside)
// when z > 0.  first element of each face is vertex count.
static const unsigned int s_faceListZP[5][5] = {
							{ 3, 0, 4, 2, 5 },		// -x
							{ 3, 1, 3, 4, 5 },		// +x
							{ 3, 0, 1, 4, 5 },		// -y
							{ 3, 3, 2, 4, 5 },		// +y
							{ 4, 0, 2, 3, 1 },		// -z
						};

// map face to vertices of face in CCW order (from the outside)
// when z < 0.  first element of each face is vertex count.
static const unsigned int s_faceListZN[5][5] = {
							{ 3, 0, 2, 4, 5 },		// -x
							{ 3, 3, 1, 4, 5 },		// +x
							{ 3, 1, 0, 4, 5 },		// -y
							{ 3, 2, 3, 4, 5 },		// +y
							{ 4, 0, 1, 3, 2 },		// +z
						};

ShapePyramid::ShapePyramid(Real x_, Real y_, Real z_) :
								x(x_),
								y(y_),
								z(z_)
{
	// set vertices
	for (unsigned int i = 0; i < 4; ++i) {
		vertex[i][0] = (i & 1) ? x : -x;
		vertex[i][1] = (i & 2) ? y : -y;
		vertex[i][2] = R_(-0.25) * z;
	}
	vertex[4][0] = R_(0.0);
	vertex[4][1] = R_(0.0);
	vertex[4][2] = R_(0.75) * z;

	// set planes
	if (z >= R_(0.0)) {
		for (unsigned int i = 0; i < 5; ++i)
			plane[i] = Plane(vertex[s_faceListZP[i][1]],
								vertex[s_faceListZP[i][2]],
								vertex[s_faceListZP[i][3]]);
	}
	else {
		for (unsigned int i = 0; i < 5; ++i)
			plane[i] = Plane(vertex[s_faceListZN[i][1]],
								vertex[s_faceListZN[i][2]],
								vertex[s_faceListZN[i][3]]);
	}
}

ShapePyramid::~ShapePyramid()
{
	// do nothing
}

Real					ShapePyramid::getVolume() const
{
	return R_(4.0) / R_(3.0) * x * y * z;
}

void					ShapePyramid::getInertia(Matrix& i) const
{
	// inertia
	Real s = getVolume();
	i.identity();
	i[0]  = s * (y * y + R_(7.0) * z * z);
	i[5]  = s * (x * x + R_(7.0) * z * z);
	i[10] = s * (x * x + y * y);
}

bool					ShapePyramid::isInside(const Vec3& p) const
{
	Real f = p[2] + R_(0.25) * z;
	if (p[2] < R_(0.0) || p[2] > z)
		return false;
	f = R_(1.0) - f / z;
	return (p[0] >= -x * f && p[0] <= x * f &&
			p[1] >= -y * f && p[1] <= y * f);
}

bool					ShapePyramid::intersect(const Ray& r) const
{
	IntersectionPoint p;
	return intersect(p, r);
}

bool					ShapePyramid::intersect(
								IntersectionPoint& p, const Ray& r) const
{
	Real t0 = -R_MAX, t1 = R_MAX;
	unsigned int f0 = 0, f1 = 0;

	// compare ray against each side.  if the ray hits the back
	// of the side then update t1 and f1, if it hits the front
	// then update t0 and f0.
	Real vn, vd, t;
	for (unsigned int i = 0; i < 5; ++i) {
		vn = plane[i].distance(r.getOrigin());
		vd = plane[i].getNormal() * r.getDirection();
		if (vd == R_(0.0)) {
			if (vn > R_(0.0))
				return false;
		}
		else {
			t  = -vn / vd;
			if (vd >= R_(0.0)) {
				if (t >= R_(0.0) && t < t1) {
					t1 = t;
					f1 = 0;
				}
			}
			else {
				if (t > t0) {
					t0 = t;
					f0 = 0;
				}
			}
		}
		if (t1 < t0)
			return false;
	}

	if (t0 < R_(0.0)) {
		// ray originates inside;  ignore t0 because it's behind the ray.
		p.t      = t1;
		p.normal = plane[f1].getNormal();
	}
	else {
		// ray originates outside or on
		p.t      = t0;
		p.normal = plane[f0].getNormal();
	}
	return true;
}

void					ShapePyramid::getRandomPoint(Vec3& p) const
{
	Real f;
	do {
		f = bzfrand();
	} while (bzfrand() > f * f);

	p[0] = x * f * (R_(2.0) * bzfrand() - R_(1.0));
	p[1] = y * f * (R_(2.0) * bzfrand() - R_(1.0));
	p[2] = z * (R_(0.75) - f);
}

void					ShapePyramid::getSupportPoint(
								SupportPoint& supportPoint,
								const Vec3& vector) const
{
	// pick best base vertex
	supportPoint.type = 0;
	if (vector[0] >= R_(0.0))
		supportPoint.type |= 1;
	if (vector[1] >= R_(0.0))
		supportPoint.type |= 2;

	// compare base vertex to tip.  use tip if it's better.
	if (vertex[supportPoint.type] * vector < vertex[4] * vector)
		supportPoint.type = 4;
	
	// set support point
	supportPoint.point = vertex[supportPoint.type];
}

ContactSurface*			ShapePyramid::getCollision(
								const ContactSimplex& simplex,
								const Plane& separationPlane,
								Real /*epsilon*/) const
{
	// map of opposing vertex index sums to face index.  6 means no face.
	static const unsigned int s_cornerToSideFaceList[4][4] = {
							{ 5, 2, 0, 5 },
							{ 2, 5, 5, 1 },
							{ 0, 5, 5, 3 },
							{ 5, 1, 3, 5 }
						};

	switch (simplex.size()) {
		case 1: {
			// vertex.  see if any adjacent face is (almost) parallel
			// to the separating plane.
			const unsigned int* faces = getAdjacentFaces(simplex[0].type);
			unsigned int face = getParallelFace(separationPlane, faces, 4);
			if (face != 4)
				if (z >= R_(0.0))
					return new ContactSurfacePolygon(
									s_faceListZP[faces[face]][0],
									vertex, s_faceListZP[faces[face]] + 1);
				else
					return new ContactSurfacePolygon(
									s_faceListZN[faces[face]][0],
									vertex, s_faceListZN[faces[face]] + 1);

			// see if an adjacent edge is parallel to the separating
			// plane.
			const unsigned int* edges = getAdjacentEdges(simplex[0].type);
			unsigned int edge = getParallelEdge(separationPlane, edges, 4);
			if (edge != 4) {
				const unsigned int* faces = getAdjacentFaces(
												edges[2 * edge + 0],
												edges[2 * edge + 1]);
				assert(faces[0] != 5);
				Vec3 n = plane[faces[0]].getNormal();
				if (faces[1] != 5) {
					n += plane[faces[1]].getNormal();
					n *= R_(0.5);
				}
				return new ContactSurfaceEdge(
									vertex[edges[2 * edge + 0]],
									vertex[edges[2 * edge + 1]], n);
			}

			// use vertex
			return new ContactSurfacePoint(vertex[simplex[0].type],
								separationPlane.getNormal());
		}

		case 2: {
			// edge.  see if either adjacent face is (almost) parallel
			// to the separating plane.  use the more parallel face.

			// first figure out which faces are adjacent
			const unsigned int* faces = getAdjacentFaces(
											simplex[0].type, simplex[1].type);

			// pick best face (if any is good enough)
			unsigned int face = getParallelFace(separationPlane, faces, 2);
			if (face != 2)
				if (z >= R_(0.0))
					return new ContactSurfacePolygon(
									s_faceListZP[faces[face]][0],
									vertex, s_faceListZP[faces[face]] + 1);
				else
					return new ContactSurfacePolygon(
									s_faceListZN[faces[face]][0],
									vertex, s_faceListZN[faces[face]] + 1);

			// use edge
			assert(faces[0] != 5);
			Vec3 n = plane[faces[0]].getNormal();
			if (faces[1] != 5) {
				n += plane[faces[1]].getNormal();
				n *= R_(0.5);
			}
			return new ContactSurfaceEdge(
									vertex[simplex[0].type],
									vertex[simplex[1].type], n);
		}

		case 3: {
			// face
			int face = 5;
			if (simplex[0].type == 4) {
				assert(simplex[1].type < 4);
				assert(simplex[2].type < 4);
				face = s_cornerToSideFaceList[simplex[1].type][simplex[2].type];
			}
			else if (simplex[1].type == 4) {
				assert(simplex[0].type < 4);
				assert(simplex[2].type < 4);
				face = s_cornerToSideFaceList[simplex[0].type][simplex[2].type];
			}
			else if (simplex[2].type == 4) {
				assert(simplex[0].type < 4);
				assert(simplex[1].type < 4);
				face = s_cornerToSideFaceList[simplex[0].type][simplex[1].type];
			}
			else {
				// base
				face = 4;
			}

			// return the face
			assert(face != 5);
			if (z >= R_(0.0))
				return new ContactSurfacePolygon(
									s_faceListZP[face][0],
									vertex, s_faceListZP[face] + 1);
			else
				return new ContactSurfacePolygon(
									s_faceListZN[face][0],
									vertex, s_faceListZN[face] + 1);
		}
	}

	assert(0 && "bad simplex dimension");
	return NULL;
}

void					ShapePyramid::getDumpPoints(
								std::vector<Vec3>& points) const
{
	for (unsigned int i = 0; i < 5; ++i)
		points.push_back(vertex[i]);
}

const unsigned int*		ShapePyramid::getAdjacentFaces(
								unsigned int index) const
{
	// map a vertex to its adjacent faces
	static const unsigned int s_vertexToFaceList[5][4] = {
							{ 0, 2, 4, 5 },
							{ 1, 2, 4, 5 },
							{ 0, 3, 4, 5 },
							{ 1, 3, 4, 5 },
							{ 0, 1, 2, 3 }
						};

	return s_vertexToFaceList[index];
}

const unsigned int*		ShapePyramid::getAdjacentFaces(
								unsigned int index1, unsigned int index2) const
{
	// vertex pair to adjacent face map.  [5,5] means no face.
	static const unsigned int s_edgeToFaceList[5][5][2] = {
							{ { 5, 5 }, { 2, 4 }, { 0, 4 }, { 4, 5 }, { 0, 2 } },
							{ { 2, 4 }, { 5, 5 }, { 4, 5 }, { 1, 4 }, { 1, 2 } },
							{ { 0, 4 }, { 4, 5 }, { 5, 5 }, { 3, 4 }, { 0, 3 } },
							{ { 4, 5 }, { 1, 4 }, { 3, 4 }, { 5, 5 }, { 1, 3 } },
							{ { 0, 2 }, { 1, 2 }, { 0, 3 }, { 1, 3 }, { 5, 5 } }
						};

	// return face list
	return s_edgeToFaceList[index1][index2];
}

const unsigned int*		ShapePyramid::getAdjacentEdges(
								unsigned int index) const
{
	static const unsigned int s_vertexToEdgeList[5][8] = {
							{ 0, 1,  0, 2,  0, 4,  5, 5 },
							{ 1, 0,  1, 3,  1, 4,  5, 5 },
							{ 2, 0,  2, 3,  2, 4,  5, 5 },
							{ 3, 1,  3, 2,  3, 4,  5, 5 },
							{ 4, 0,  4, 1,  4, 2,  4, 3 },
						};

	return s_vertexToEdgeList[index];
};

unsigned int			ShapePyramid::getParallelEdge(
								const Plane& plane,
								const unsigned int* edgePairList,
								unsigned int nPairs) const
{
	// find the edge that's most parallel to the plane.  choose no
	// edge if none is within some tolerance of being parellel.
	// note that anti-parallel is not the same as parallel.

	// the angle between the edge vector and the face normal, alpha,
	// must satisfy abs(cos(alpha)) < s_maxAlignCosine to be considered
	// parallel.
	static const Real s_maxAlignCosine = R_(0.01745240644); // 89 degrees

	assert(edgePairList != NULL);
	assert(nPairs > 0);
	assert(edgePairList[0] != 5);
	assert(edgePairList[1] != 5);

	// pick first edge by default
	unsigned int bestEdge = 0;
	Vec3 tmp              = vertex[edgePairList[0]] - vertex[edgePairList[1]];
	Real bestAngle        = fabsf(tmp * plane.getNormal()) / tmp.length();

	// compare against the other edges
	for (unsigned int i = 1; i < nPairs; ++i) {
		if (edgePairList[2 * i + 0] == 5)
			continue;
		tmp  = vertex[edgePairList[2 * i + 0]] - vertex[edgePairList[2 * i + 1]];
		Real angle = fabsf(tmp * plane.getNormal()) / tmp.length();
		if (angle < bestAngle) {
			bestEdge  = i;
			bestAngle = angle;
		}
	}

	// if angle is good enough then return the (index of the) best edge
	if (bestAngle < s_maxAlignCosine)
		return bestEdge;

	// no satisfactory edge
	return nPairs;
}

unsigned int			ShapePyramid::getParallelFace(
								const Plane& plane2,
								const unsigned int* faceList,
								unsigned int n) const
{
	// find the face that's most parallel to the plane.  choose no
	// face if none is within some tolerance of being parellel.
	// note that anti-parallel is not the same as parallel.

	// the angle between face normals, alpha, must satisfy
	// cos(alpha) > s_minAlignCosine to be considered parallel.
	static const Real s_minAlignCosine = R_(0.9998476952); // 1 degree

	assert(faceList != NULL);
	assert(n > 0);
	assert(faceList[0] != 5);

	// pick first face by default
	unsigned int bestFace = 0;
	Real bestAngle        = plane[faceList[0]].getNormal() * plane2.getNormal();

	// compare against the other faces
	for (unsigned int i = 1; i < n; ++i) {
		if (faceList[i] == 5)
			continue;
		Real angle = plane[faceList[i]].getNormal() * plane2.getNormal();
		if (angle > bestAngle) {
			bestFace  = i;
			bestAngle = angle;
		}
	}

	// if angle is good enough then return the (index of the) best face
	if (bestAngle > s_minAlignCosine)
		return bestFace;

	// no satisfactory face
	return n;
}
