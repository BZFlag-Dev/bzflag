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

#include "ShapeBox.h"
#include "ContactSurfacePoint.h"
#include "ContactSurfaceEdge.h"
#include "ContactSurfacePolygon.h"

//
// ShapeBox
//

ShapeBox::ShapeBox(Real x_, Real y_, Real z_) :
								x(x_),
								y(y_),
								z(z_)
{
	// set vertices
	for (unsigned int i = 0; i < 8; ++i) {
		vertex[i][0] = (i & 1) ? x : -x;
		vertex[i][1] = (i & 2) ? y : -y;
		vertex[i][2] = (i & 4) ? z : -z;
	}

	// set normals
	normal[0].set(R_(-1.0), R_( 0.0), R_( 0.0));
	normal[1].set(R_( 1.0), R_( 0.0), R_( 0.0));
	normal[2].set(R_( 0.0), R_(-1.0), R_( 0.0));
	normal[3].set(R_( 0.0), R_( 1.0), R_( 0.0));
	normal[4].set(R_( 0.0), R_( 0.0), R_(-1.0));
	normal[5].set(R_( 0.0), R_( 0.0), R_( 1.0));
}

ShapeBox::~ShapeBox()
{
	// do nothing
}

Real					ShapeBox::getVolume() const
{
	return R_(8.0) * x * y * z;
}

void					ShapeBox::getInertia(Matrix& i) const
{
	// inertia
	Real s = getVolume() / R_(3.0);
	i.identity();
	i[0]  = s * (y * y + z * z);
	i[5]  = s * (x * x + z * z);
	i[10] = s * (x * x + y * y);
}

bool					ShapeBox::isInside(const Vec3& p) const
{
	return (p[0] >= -x && p[0] <= x &&
			p[1] >= -y && p[1] <= y &&
			p[2] >= -z && p[2] <= z);
}

bool					ShapeBox::intersect(const Ray& r) const
{
	IntersectionPoint p;
	return intersect(p, r);
}

bool					ShapeBox::intersect(
								IntersectionPoint& p, const Ray& r) const
{
	Real t0 = -R_MAX, t1 = R_MAX;
	unsigned int f0 = 0, f1 = 0;

	// compare ray against each side.  if the ray hits the back
	// of the side then update t1 and f1, if it hits the front
	// then update t0 and f0.
	Real vn, vd, t;

	// -x face
	vn = -r.getOrigin()[0] - x;
	vd = -r.getDirection()[0];
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

	// +x face
	vn = r.getOrigin()[0] - x;
	vd = r.getDirection()[0];
	if (vd == R_(0.0)) {
		if (vn > R_(0.0))
			return false;
	}
	else {
		t  = -vn / vd;
		if (vd >= R_(0.0)) {
			if (t >= R_(0.0) && t < t1) {
				t1 = t;
				f1 = 1;
			}
		}
		else {
			if (t > t0) {
				t0 = t;
				f0 = 1;
			}
		}
	}
	if (t1 < t0)
		return false;

	// -y face
	vn = -r.getOrigin()[1] - y;
	vd = -r.getDirection()[1];
	if (vd == R_(0.0)) {
		if (vn > R_(0.0))
			return false;
	}
	else {
		t  = -vn / vd;
		if (vd >= R_(0.0)) {
			if (t >= R_(0.0) && t < t1) {
				t1 = t;
				f1 = 2;
			}
		}
		else {
			if (t > t0) {
				t0 = t;
				f0 = 2;
			}
		}
	}
	if (t1 < t0)
		return false;

	// +y face
	vn = r.getOrigin()[1] - y;
	vd = r.getDirection()[1];
	if (vd == R_(0.0)) {
		if (vn > R_(0.0))
			return false;
	}
	else {
		t  = -vn / vd;
		if (vd >= R_(0.0)) {
			if (t >= R_(0.0) && t < t1) {
				t1 = t;
				f1 = 3;
			}
		}
		else {
			if (t > t0) {
				t0 = t;
				f0 = 3;
			}
		}
	}
	if (t1 < t0)
		return false;

	// -z face
	vn = -r.getOrigin()[2] - z;
	vd = -r.getDirection()[2];
	if (vd == R_(0.0)) {
		if (vn > R_(0.0))
			return false;
	}
	else {
		t  = -vn / vd;
		if (vd >= R_(0.0)) {
			if (t >= R_(0.0) && t < t1) {
				t1 = t;
				f1 = 4;
			}
		}
		else {
			if (t > t0) {
				t0 = t;
				f0 = 4;
			}
		}
	}
	if (t1 < t0)
		return false;

	// +z face
	vn = r.getOrigin()[2] - z;
	vd = r.getDirection()[2];
	if (vd == R_(0.0)) {
		if (vn > R_(0.0))
			return false;
	}
	else {
		t  = -vn / vd;
		if (vd >= R_(0.0)) {
			if (t >= R_(0.0) && t < t1) {
				t1 = t;
				f1 = 5;
			}
		}
		else {
			if (t > t0) {
				t0 = t;
				f0 = 5;
			}
		}
	}
	if (t1 < t0)
		return false;

	if (t0 < R_(0.0)) {
		// ray originates inside box;  ignore t0 because it's behind the ray.
		p.t      = t1;
		p.normal = normal[f1];
	}
	else {
		// ray originates outside or on box
		p.t      = t0;
		p.normal = normal[f0];
	}
	return true;
}

void					ShapeBox::getRandomPoint(Vec3& p) const
{
	p[0] = x * (R_(2.0) * bzfrand() - R_(1.0));
	p[1] = y * (R_(2.0) * bzfrand() - R_(1.0));
	p[2] = z * (R_(2.0) * bzfrand() - R_(1.0));
}

void					ShapeBox::getSupportPoint(
								SupportPoint& supportPoint,
								const Vec3& vector) const
{
	supportPoint.type = 0;
	if (vector[0] >= R_(0.0))
		supportPoint.type |= 1;
	if (vector[1] >= R_(0.0))
		supportPoint.type |= 2;
	if (vector[2] >= R_(0.0))
		supportPoint.type |= 4;
	supportPoint.point = vertex[supportPoint.type];
}

ContactSurface*			ShapeBox::getCollision(
								const ContactSimplex& simplex,
								const Plane& separationPlane,
								Real /*epsilon*/) const
{
	// map face to vertices of face in CCW order (from the outside)
	static const unsigned int s_faceList[6][4] = {
							{ 0, 4, 6, 2 },		// -x
							{ 1, 3, 7, 5 },		// +x
							{ 0, 1, 5, 4 },		// -y
							{ 2, 6, 7, 3 },		// +y
							{ 0, 2, 3, 1 },		// -z
							{ 4, 5, 7, 6 }		// +z
						};

	// map of opposing vertex index sums to face index.  6 means no face.
	static const unsigned int s_cornerToFaceList[14] = {
							6,
							6,
							6,
							4,
							6,
							2,
							0,
							6,
							1,
							3,
							6,
							5,
							6,
							6
						};

	switch (simplex.size()) {
		case 1: {
			// vertex.  see if any adjacent face is (almost) parallel
			// to the separating plane.
			const unsigned int* faces = getAdjacentFaces(simplex[0].type);
			unsigned int face = getParallelFace(separationPlane, faces, 3);
			if (face != 3)
				return new ContactSurfacePolygon(4,
									vertex, s_faceList[faces[face]]);

			// see if an adjacent edge is parallel to the separating
			// plane.
			const unsigned int* edges = getAdjacentEdges(simplex[0].type);
			unsigned int edge = getParallelEdge(separationPlane, edges, 3);
			if (edge != 3) {
				const unsigned int* faces = getAdjacentFaces(
												edges[2 * edge + 0],
												edges[2 * edge + 1]);
				assert(faces[0] != 6);
				Vec3 n = normal[faces[0]];
				if (faces[1] != 6) {
					n += normal[faces[1]];
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
				return new ContactSurfacePolygon(4,
									vertex, s_faceList[faces[face]]);

			// use edge
			assert(faces[0] != 6);
			Vec3 n = normal[faces[0]];
			if (faces[1] != 6) {
				n += normal[faces[1]];
				n *= R_(0.5);
			}
			return new ContactSurfaceEdge(
									vertex[simplex[0].type],
									vertex[simplex[1].type], n);
		}

		case 3: {
			// face.  figure out which vertices do not share an edge.
			// with that we can figure out which face.
			unsigned int index = simplex[0].type ^ simplex[1].type;
			if (index == 3 || index == 5 || index == 6) {
				index = simplex[0].type + simplex[1].type;
			}
			else {
				index = simplex[0].type ^ simplex[2].type;
				if (index == 3 || index == 5 || index == 6) {
					index = simplex[0].type + simplex[2].type;
				}
				else {
					index = simplex[1].type ^ simplex[2].type;
					assert(index == 3 || index == 5 || index == 6);
					index = simplex[1].type + simplex[2].type;
				}
			}

			// return the face
			assert(s_cornerToFaceList[index] != 6);
			return new ContactSurfacePolygon(4,
								vertex, s_faceList[s_cornerToFaceList[index]]);
		}
	}

	assert(0 && "bad simplex dimension");
	return NULL;
}

void					ShapeBox::getDumpPoints(
								std::vector<Vec3>& points) const
{
	for (unsigned int i = 0; i < 8; ++i)
		points.push_back(vertex[i]);
}

const unsigned int*		ShapeBox::getAdjacentFaces(
								unsigned int index) const
{
	// map a vertex to its adjacent faces
	static const unsigned int s_vertexToFaceList[8][3] = {
							{ 0, 2, 4 },
							{ 1, 2, 4 },
							{ 0, 3, 4 },
							{ 1, 3, 4 },
							{ 0, 2, 5 },
							{ 1, 2, 5 },
							{ 0, 3, 5 },
							{ 1, 3, 5 },
						};

	return s_vertexToFaceList[index];
}

const unsigned int*		ShapeBox::getAdjacentFaces(
								unsigned int index1, unsigned int index2) const
{
	// vertex pair to adjacent face map.  [6,6] means no face.
	static const unsigned int s_edgeToFaceList[8][8][2] = {
							// impossible
							{ { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 },
							  { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 } },

							// edge aligned with x axis
							{ { 2, 4 }, { 6, 6 }, { 3, 4 }, { 6, 6 },
							  { 2, 5 }, { 6, 6 }, { 3, 5 }, { 6, 6 } },

							// edge aligned with y axis
							{ { 0, 4 }, { 1, 4 }, { 6, 6 }, { 6, 6 },
							  { 0, 5 }, { 1, 5 }, { 6, 6 }, { 6, 6 } },

							// edge crosses -z/+z face
							{ { 4, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 },
							  { 5, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 } },

							// edge aligned with z axis
							{ { 0, 2 }, { 1, 2 }, { 0, 3 }, { 1, 3 },
							  { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 } },

							// edge crosses -y/+y face
							{ { 2, 6 }, { 6, 6 }, { 3, 6 }, { 6, 6 },
							  { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 } },

							// edge crosses -x/+x face
							{ { 0, 6 }, { 1, 6 }, { 6, 6 }, { 6, 6 },
							  { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 } },

							// impossible (opposite corners)
							{ { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 },
							  { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 } }
						};

	// return face list
	return s_edgeToFaceList[index1 ^ index2][index1 & index2];
}

const unsigned int*		ShapeBox::getAdjacentEdges(
								unsigned int index) const
{
	static const unsigned int s_vertexToEdgeList[8][6] = {
							{ 0, 1,  0, 2,  0, 4 },
							{ 1, 0,  1, 3,  1, 5 },
							{ 2, 0,  2, 3,  2, 6 },
							{ 3, 1,  3, 2,  3, 7 },
							{ 4, 0,  4, 5,  4, 6 },
							{ 5, 1,  5, 4,  5, 7 },
							{ 6, 2,  6, 4,  6, 7 },
							{ 7, 3,  7, 5,  7, 6 },
						};

	return s_vertexToEdgeList[index];
};

unsigned int			ShapeBox::getParallelEdge(
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

	// pick first edge by default
	unsigned int bestEdge = 0;
	Vec3 tmp              = vertex[edgePairList[0]] - vertex[edgePairList[1]];
	Real bestAngle        = fabsf(tmp * plane.getNormal()) / tmp.length();

	// compare against the other edges
	for (unsigned int i = 1; i < nPairs; ++i) {
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

unsigned int			ShapeBox::getParallelFace(
								const Plane& plane,
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
	assert(faceList[0] != 6);

	// pick first face by default
	unsigned int bestFace = 0;
	Real bestAngle        = normal[faceList[0]] * plane.getNormal();

	// compare against the other faces
	for (unsigned int i = 1; i < n; ++i) {
		if (faceList[i] == 6)
			continue;
		Real angle = normal[faceList[i]] * plane.getNormal();
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
// ex: shiftwidth=4 tabstop=4
