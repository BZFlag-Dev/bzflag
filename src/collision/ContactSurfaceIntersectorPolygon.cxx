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

#include "ContactSurfaceIntersectorPolygon.h"
#include "ContactSurfacePoint.h"
#include "ContactSurfaceEdge.h"
#include "ContactSurfacePolygon.h"

//
// point/point
//

void					ContactSurfaceIntersectorPointPoint::doIntersect(
								ContactPoints& contacts,
								Body* aBody, Body* bBody,
								const ContactSurfacePoint* a,
								const ContactSurfacePoint* b) const
{
fprintf(stderr, "point/point\n");
fprintf(stderr, "  vertex/face\n");
	ContactPoint p;
	p.point = a->getVertex();
	p.setVertexFace(b->getNormal());
Vec3 d = a->getVertex();
d -= b->getVertex();
p.distance = d.length();
	p.a = aBody;
	p.b = bBody;
	contacts.push_back(p);
}


//
// point/edge
//

void					ContactSurfaceIntersectorPointEdge::doIntersect(
								ContactPoints& contacts,
								Body* aBody, Body* bBody,
								const ContactSurfacePoint* a,
								const ContactSurfaceEdge* /*b*/) const
{
fprintf(stderr, "point/edge\n");
fprintf(stderr, "  vertex/face\n");
	ContactPoint p;
	p.point = a->getVertex();
	p.setVertexFace(a->getNormal());
	p.normal.negate();
p.distance = R_(0.0); // FIXME
	p.a = aBody;
	p.b = bBody;
	contacts.push_back(p);
}


//
// point/polygon
//

void					ContactSurfaceIntersectorPointPolygon::doIntersect(
								ContactPoints& contacts,
								Body* aBody, Body* bBody,
								const ContactSurfacePoint* a,
								const ContactSurfacePolygon* b) const
{
fprintf(stderr, "point/polygon\n");
fprintf(stderr, "  vertex/face\n");
	ContactPoint p;
	p.point = a->getVertex();
	p.setVertexFace(b->getNormal());
p.distance = b->getPlane().distance(p.point);
	p.a = aBody;
	p.b = bBody;
	contacts.push_back(p);
}


//
// edge/edge
//

void					ContactSurfaceIntersectorEdgeEdge::doIntersect(
								ContactPoints& contacts,
								Body* aBody, Body* bBody,
								const ContactSurfaceEdge* a,
								const ContactSurfaceEdge* b) const
{
fprintf(stderr, "edge/edge\n");
	// get edge vectors
	Vec3 aVec = a->getVertex2() - a->getVertex1();
	Vec3 bVec = b->getVertex2() - b->getVertex1();

	// check if edges are parallel or skew
	Vec3 normal = aVec % bVec;
	Real l2 = normal.length2();
	if (l2 > R_(1.0e-10) * aVec.length2() * bVec.length2()) {
		// edges are skew.  contact point is point on a that's
		// closest to b.  compute parameter of that point.
		Vec3 diff = (b->getVertex1() - a->getVertex1()) % bVec;
		Real t    = (normal * diff) / l2;
		if (t < R_(0.0)) {
			// note -- point is off edge;  this shouldn't happen
			t = R_(0.0);
		}
		else if (t > R_(1.0)) {
			// note -- point is off edge;  this shouldn't happen
			t = R_(1.0);
		}

		// construct point
fprintf(stderr, "  edge/edge\n");
		ContactPoint p;
		p.point = a->getVertex1() + t * aVec;
		p.setEdgeEdge(aVec, bVec, b->getAverageNormal());
		p.a = aBody;
		p.b = bBody;

diff  = (b->getVertex1() - a->getVertex1()) % aVec;
t     = (normal * diff) / l2;
diff  = b->getVertex1() + t * bVec - p.point;
p.distance = diff.length();

		// add point
		contacts.push_back(p);
	}
	else {
		// edges are parallel.  project b onto a and use the endpoints
		// of the intersection.  first project b onto a.
		Real m     = a->getVertex1() * aVec;
		Real lInv2 = R_(1.0) / aVec.length2();
		Real t1    = lInv2 * ((b->getVertex1() * aVec) - m);
		Real t2    = lInv2 * ((b->getVertex2() * aVec) - m);
		if (t1 < t2) {
			Real tmp = t1;
			t1       = t2;
			t2       = tmp;
		}

		// compute the normal.  this is somewhat arbitrary since the
		// intersection of parallel edges is degenerate.
		Vec3 normal = b->getAverageNormal();

Real t = ((b->getVertex1() * bVec) - (a->getVertex1() * bVec)) / bVec.length2();
Vec3 pt = b->getVertex1() + t * bVec - a->getVertex1();
Real d = pt.length();

		// intersect
		if (t1 < R_(0.0))
			t1 = R_(0.0);
		if (t2 > R_(1.0))
			t2 = R_(1.0);
		if (t1 > R_(1.0)) {
			// note -- edges don't overlap;  this shouldn't happen
fprintf(stderr, "  vertex/face (no overlap, t1 > 1)\n");
			ContactPoint p;
			p.point = a->getVertex1() + aVec;
			p.setVertexFace(normal);
p.distance = d;
			contacts.push_back(p);
		}
		else if (t2 < R_(0.0)) {
			// note -- edges don't overlap;  this shouldn't happen
fprintf(stderr, "  vertex/face (no overlap, t2 < 0)\n");
			ContactPoint p;
			p.point = a->getVertex1();
			p.setVertexFace(normal);
			p.a = aBody;
			p.b = bBody;
p.distance = d;
			contacts.push_back(p);
		}
		else {
			// got overlap
fprintf(stderr, "  vertex/face\n");
fprintf(stderr, "  vertex/face\n");
			ContactPoint p;
			p.setVertexFace(normal);
			p.point = a->getVertex1() + t1 * aVec;
			p.a = aBody;
			p.b = bBody;
p.distance = d;
			contacts.push_back(p);
			p.point = a->getVertex1() + t2 * aVec;
p.distance = d;
			contacts.push_back(p);
		}
	}
}


//
// edge/polygon
//

void					ContactSurfaceIntersectorEdgePolygon::doIntersect(
								ContactPoints& contacts,
								Body* aBody, Body* bBody,
								const ContactSurfaceEdge* a,
								const ContactSurfacePolygon* b) const
{
fprintf(stderr, "edge/polygon\n");
	// intersect edge against polygon

	// choose the plane to project coordinates onto so we've got a 2D
	// intersection problem rather than a 3D problem.
	const Vec3& normal = b->getNormal();
	if (fabsf(normal[0]) > fabsf(normal[1]))
		if (fabsf(normal[0]) > fabsf(normal[2]))
			doIntersect2D<1,2>(contacts, aBody, bBody, a, b,
												(normal[0] < R_(0.0)));
		else
			doIntersect2D<0,1>(contacts, aBody, bBody, a, b,
												(normal[2] < R_(0.0)));
	else
		if (fabsf(normal[1]) > fabsf(normal[2]))
			doIntersect2D<2,0>(contacts, aBody, bBody, a, b,
												(normal[1] < R_(0.0)));
		else
			doIntersect2D<0,1>(contacts, aBody, bBody, a, b,
												(normal[2] < R_(0.0)));
}

template <int x, int y>
void					ContactSurfaceIntersectorEdgePolygon::
							doIntersect2D(
								ContactPoints& contacts,
								Body* aBody, Body* bBody,
								const ContactSurfaceEdge* a,
								const ContactSurfacePolygon* b,
								bool flip) const
{
	// choose endpoints
	const Vec3& vertex1 = a->getVertex1();
	const Vec3& vertex2 = a->getVertex2();

	// compute (directed) edge vector
	Vec3 aVec = vertex2 - vertex1;

	// include entire edge by default
	Real t0 = R_(0.0), t1 = R_(1.0);

	// intersect.  conceptually, we insersect b with every (directed)
	// edge of the polygon.  then we take the intersection of all these
	// intersections.  in practice, we update t0 or t1 at each polygon
	// edge, incrementally computing the intersection of intersections.
	const ContactSurfacePolygon::VertexList& vertex = b->getVertices();
	const unsigned int n = vertex.size();
	unsigned int i0 = 0, i1 = 0;
	for (unsigned int i = 0, j = n - 1; i < n; j = i, ++i) {
		// intersect a against the polygon edge vertex[j],vertex[i].
		Real edge[3];
		edge[0] = vertex[j][y] - vertex[i][y];
		edge[1] = vertex[i][x] - vertex[j][x];
		edge[2] = -(vertex[j][x] * edge[0] + vertex[j][y] * edge[1]);

		Real vn = -(edge[0] * vertex1[x] +
					edge[1] * vertex1[y] + edge[2]);
		Real vd = edge[0] * aVec[x] + edge[1] * aVec[y];

		if ((!flip && vd < R_(0.0)) || (flip && vd > R_(0.0))) {
			// hit the inside of the edge
			Real t = vn / vd;
			if (t > R_(0.0) && t < t1) {
				t1 = t;
				i1 = i;
			}
		}
		else if ((!flip && vd > R_(0.0)) || (flip && vd < R_(0.0))) {
			// hit the outside of the edge
			Real t = vn / vd;
			if (t > t0) {
				t0 = t;
				i0 = i;
			}
		}
		else {
			// parallel;  ignore
			continue;
		}

		// if t0 > t1 then there's no intersection at all.  since we
		// expect an intersection we'll use the second endpoint and
		// assume we hit the polygon edge.
		if (t0 > t1) {
fprintf(stderr, "  edge/edge (degenerate)\n");
			ContactPoint p;
			p.point   = vertex2;
			p.setEdgeEdge(aVec, vertex[j] - vertex[i], b->getNormal());
			p.a       = aBody;
			p.b       = bBody;
p.distance = b->getPlane().distance(p.point);
			contacts.push_back(p);
			return;
		}
	}

	// two intersections (or one degenerate intersection).  t0 and t1
	// give points of intersection.  if t0 is not 0 (t1 is not 1) then
	// it's an edge/edge intersection, otherwise it's a vertex/face.
	ContactPoint p;
	p.point = vertex1 + t0 * aVec;
	p.a     = aBody;
	p.b     = bBody;
	if (t0 != R_(0.0)) {
fprintf(stderr, "  edge/edge\n");
		Vec3 bVec = vertex[i0] - vertex[(i0 + n - 1) % n];
		p.setEdgeEdge(aVec, bVec, b->getNormal());
p.distance = b->getPlane().distance(p.point);
	}
	else {
fprintf(stderr, "  vertex/face\n");
		p.setVertexFace(b->getNormal());
p.distance = b->getPlane().distance(p.point);
	}
	contacts.push_back(p);
	if (t1 != t0) {
fprintf(stderr, "  edge/edge\n");
		p.point = vertex1 + t1 * aVec;
		if (t1 != R_(1.0)) {
			Vec3 bVec = vertex[i1] - vertex[(i1 + n - 1) % n];
			p.setEdgeEdge(aVec, bVec, b->getNormal());
p.distance = b->getPlane().distance(p.point);
		}
		else {
fprintf(stderr, "  vertex/face\n");
			p.setVertexFace(b->getNormal());
p.distance = b->getPlane().distance(p.point);
		}
		contacts.push_back(p);
	}
}


//
// polygon/polygon
//

void					ContactSurfaceIntersectorPolygonPolygon::doIntersect(
								ContactPoints& contacts,
								Body* aBody, Body* bBody,
								const ContactSurfacePolygon* a,
								const ContactSurfacePolygon* b) const
{
fprintf(stderr, "polygon/polygon\n");
	// intersect polygon against polygon

	// choose the plane to project coordinates onto so we've got a 2D
	// intersection problem rather than a 3D problem.
	const Vec3& normal = b->getNormal();
	if (fabsf(normal[0]) > fabsf(normal[1]))
		if (fabsf(normal[0]) > fabsf(normal[2]))
			doIntersect2D<1,2>(contacts, aBody, bBody, a, b,
												(normal[0] < R_(0.0)));
		else
			doIntersect2D<0,1>(contacts, aBody, bBody, a, b,
												(normal[2] < R_(0.0)));
	else
		if (fabsf(normal[1]) > fabsf(normal[2]))
			doIntersect2D<2,0>(contacts, aBody, bBody, a, b,
												(normal[1] < R_(0.0)));
		else
			doIntersect2D<0,1>(contacts, aBody, bBody, a, b,
												(normal[2] < R_(0.0)));
}

class ClippedVertex {
public:
	ClippedVertex(const Vec3& v, bool a, unsigned int i) :
								aEdge(a), edgeIndex(i), vertex(v) { }

public:
	bool				aEdge;			// is edge from a (true) or b (false)
	unsigned int		edgeIndex;		// index of edge leaving vertex
	Vec3				vertex;
};
typedef std::vector<ClippedVertex> ClippedList;

template <int x, int y>
void					ContactSurfaceIntersectorPolygonPolygon::
							doIntersect2D(
								ContactPoints& contacts,
								Body* aBody, Body* bBody,
								const ContactSurfacePolygon* a,
								const ContactSurfacePolygon* b,
								bool flip) const
{
	// make a list of the original vertices in a.  we'll also track
	// the whether a vertex is on an edge.  initially, no vertex is
	// on an edge.
	ClippedList clipped;
	const ContactSurfacePolygon::VertexList& aVertex = a->getVertices();
	for (unsigned int i = aVertex.size(); i-- > 0; )
		clipped.push_back(ClippedVertex(aVertex[i], true, i));

	// clip edges
	const ContactSurfacePolygon::VertexList& bVertex = b->getVertices();
	for (unsigned int i = 0, j = bVertex.size() - 1;
								clipped.size() > 1 &&
								i < bVertex.size(); j = i, ++i) {
		// clip all edges in clipped against bVertex[j],bVertex[i].
		// we use the sutherland-hodgman algorithm:  classify each
		// point in clipped as being on one side (+) or the other
		// (-) of the edge then clip between any vertices that have
		// different signs.

		// compute the "plane" equation for the edge.  the inside of
		// the polygon is in the positive (negative) half-space if
		// flip is false (true).
		Real edge[3];
		edge[0] = bVertex[j][y] - bVertex[i][y];
		edge[1] = bVertex[i][x] - bVertex[j][x];
		edge[2] = -(bVertex[j][x] * edge[0] + bVertex[j][y] * edge[1]);

		// move clipped to previous
		ClippedList previous;
		previous.swap(clipped);

		// get sign of first vertex in previous
		unsigned int k = previous.size() - 1;
		Real vn   = edge[0] * previous[k].vertex[x] +
					edge[1] * previous[k].vertex[y] +
					edge[2];
		bool plus = ((vn >= R_(0.0)) != flip);

		// loop over edges in previous
		for (unsigned int h = 0; h < previous.size(); k = h, ++h) {
			// add point k if it's the positive half space
			if (plus)
				clipped.push_back(previous[k]);

			// get sign of vertex h
			Real vnNext   = edge[0] * previous[h].vertex[x] +
							edge[1] * previous[h].vertex[y] +
							edge[2];
			bool plusNext = ((vnNext >= R_(0.0)) != flip);

			// if there's a sign change then clip and add the new point
			if (plus != plusNext) {
				// compute intersection parameter along previous's edge
				Vec3 d  = previous[h].vertex - previous[k].vertex;
				Real vd = (edge[0] * d[x] + edge[1] * d[y]);
				Real t  = -vn / vd;

				// if inside edge (and we're not going to duplicate
				// a point) then add the intersection.
				if ((t > R_(0.0) && t < R_(1.0)) ||
					(t == R_(0.0) && !plus) ||
					(t == R_(1.0) &&  plus)) {
					Vec3 tmp = previous[k].vertex + t * d;

					// if this edge connects two edge points then the
					// new vertex is an internal point (and it is, in
					// fact, a vertex in bVertex).
					if (plusNext)
						clipped.push_back(ClippedVertex(tmp, previous[k].aEdge, previous[k].edgeIndex));
					else
						clipped.push_back(ClippedVertex(tmp, false, j));
				}

				// remember new sign
				plus = plusNext;
			}

			// prepare for next point
			vn = vnNext;
		}
	}

	// add all vertices in clipped.  if the edge intersection count is 1
	// then it's an edge/edge intersection, otherwise it's a vertex/face.
	ContactPoint p;
	const unsigned int n = clipped.size();
	for (unsigned int i = 0, j = n - 1; i < n; j = i, ++i) {
		p.point = clipped[i].vertex;
		if (clipped[i].aEdge != clipped[j].aEdge) {
			if (clipped[i].aEdge) {
				Vec3 aVec = bVertex[(clipped[j].edgeIndex + 1) % bVertex.size()] -
							bVertex[clipped[j].edgeIndex];
				Vec3 bVec = aVertex[(clipped[i].edgeIndex + aVertex.size() - 1) % aVertex.size()] -
							aVertex[clipped[i].edgeIndex];
				p.setEdgeEdge(aVec, bVec, b->getNormal());
p.distance = b->getPlane().distance(p.point);
			}
			else {
				Vec3 aVec = aVertex[(clipped[j].edgeIndex + aVertex.size() - 1) % aVertex.size()] -
							aVertex[clipped[j].edgeIndex];
				Vec3 bVec = bVertex[(clipped[i].edgeIndex + 1) % bVertex.size()] -
							bVertex[clipped[i].edgeIndex];
				p.setEdgeEdge(aVec, bVec, b->getNormal());
p.distance = b->getPlane().distance(p.point);
			}
			p.a = aBody;
			p.b = bBody;
fprintf(stderr, "  edge/edge: %g %g %g\n", p.normal[0], p.normal[1], p.normal[2]);
		}
		else if (clipped[i].aEdge) {
fprintf(stderr, "  vertex (a)\n");
			p.setVertexFace(b->getNormal());
p.normal2 = a->getNormal();
			p.a = aBody;
			p.b = bBody;
p.distance = b->getPlane().distance(p.point);
		}
		else {
fprintf(stderr, "  vertex (b)\n");
			p.setVertexFace(a->getNormal());
p.normal2 = b->getNormal();
			p.a = bBody;
			p.b = aBody;
p.distance = b->getPlane().distance(p.point);
		}
		contacts.push_back(p);
	}
}
