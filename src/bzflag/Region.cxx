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

#include <math.h>
#include "Region.h"

//
// RegionPoint
//

RegionPoint::RegionPoint(float x, float y)
{
	p[0] = x;
	p[1] = y;
}

RegionPoint::RegionPoint(const float v[2])
{
	p[0] = v[0];
	p[1] = v[1];
}

RegionPoint::~RegionPoint()
{
	// do nothing
}

const float*				RegionPoint::get() const
{
	return p;
}

//
// BzfRegion
//
// Note: edge from corners[0] to corners[1] has neighbor stored in
// neighbors[0].  all regions should have corners ordered counter-
// clockwise.
//

BzfRegion::BzfRegion() : mailbox(0), target(0), A(0.0, 0.0)
{
}

BzfRegion::BzfRegion(int sides, const float p[][2]) :
								mailbox(0), target(0), A(0.0, 0.0)
{
	for (int i = 0; i < sides; i++) {
		corners.push_back(RegionPoint(p[i]));
		neighbors.push_back((BzfRegion*)0);
	}
}

BzfRegion::~BzfRegion()
{
	// tell neighbors I'm going away
	const int count = corners.size();
	for (int i = 0; i < count; i++)
		if (neighbors[i])
			neighbors[i]->setNeighbor(this, 0);
}

bool					BzfRegion::isInside(const float p[2]) const
{
	// see if testPoint is inside my edges
	const int count = corners.size();
	if (count < 3) return false;
	bool inside = false;
	const float* p1 = corners[count - 1].get();
	const float* p2 = NULL;
	for (int i = 0; i < count; p1 = p2, i++) {
		p2 = corners[i].get();
		if (p1[1] >= p[1] && p2[1] >= p[1] || p1[1] < p[1] && p2[1] < p[1])
			continue;
		if (p1[0] < p[0] && p2[0] < p[0])
			continue;
		if (p1[0] >= p[0] && p2[0] >= p[0]) inside = !inside;
		else {
			float tolerance = (p1[1] < p2[1]) ? -0.001f : 0.001f;
			if ((p[1] - p1[1]) * (p2[0] - p1[0]) / (p2[1] - p1[1]) >=
												p[0] - p1[0] + tolerance)
				inside = !inside;
		}
	}
	return inside;
}

int						BzfRegion::classify(const float e1[2],
												const float e2[2]) const
{
	// return true if all points lie to right side of edge
	const float dx = e2[0] - e1[0];
	const float dy = e2[1] - e1[1];
	const float d = dy * e1[0] - dx * e1[1];
	int toRight = 0, onEdge = 0;
	const int count = corners.size();
	for (int i = 0; i < count; i++) {
		const float* p = corners[i].get();
		const float e = -dy * p[0] + dx * p[1] + d;
		if (e < -0.00001) toRight++;
		else if (e <= 0.00001) onEdge++;
	}
	if (toRight + onEdge == count) return 1;		// all to right
	if (toRight != 0) return 0;					// not all to left -- split
	return -1;										// all to left
}

int						BzfRegion::getNumSides() const
{
	return neighbors.size();
}

const RegionPoint&		BzfRegion::getCorner(int index) const
{
	return corners[index];
}

BzfRegion*				BzfRegion::getNeighbor(int index) const
{
	return neighbors[index];
}

BzfRegion*				BzfRegion::orphanSplitRegion(const float e1[2],
												const float e2[2])
{
	// if edge p1,p2 intersects me then split myself along that edge.
	// return new region (the other half of the split), or NULL if no
	// split occured.  the new region (if it exists) will be to the
	// right of the cutting edge (when moving from e1 to e2).
	const int count = corners.size();
	if (count == 0) return NULL;
	int i, split = 0, edge[2];
	float tsplit[2], etsplit[2];
	const float dx = e2[0] - e1[0];
	const float dy = e2[1] - e1[1];
	const float d = dy * e1[0] - dx * e1[1];
	const float* p1 = corners[0].get();
	float lastDistance = dy * p1[0] - dx * p1[1];
	for (i = 0; split < 2 && i < count; i++) {
		const float* p2 = corners[(i + 1) % count].get();
		const float distance = dy * p2[0] - dx * p2[1];

		// if both ends of region edge are on same side of cutting edge then
		// can't intersect the edge.
		if (lastDistance >= d && distance < d || lastDistance < d && distance >= d){
			// compute distance along region edge where intersection occurs
			tsplit[split] = (d - lastDistance) / (distance - lastDistance);
			if (tsplit[split] >= 0.0 && tsplit[split] <= 1.0) {
				edge[split] = i;
				etsplit[split] = ((p1[1] - p2[1]) * (p1[0] - e1[0]) +
						(p2[0] - p1[0]) * (p1[1] - e1[1])) /
						(distance - lastDistance);
				split++;
			}
		}
		lastDistance = distance;
		p1 = p2;
	}

	// done if no intersections
	if (split != 2 ||
		(etsplit[0] <= 0.0 && etsplit[1] <= 0.0) ||
		(etsplit[0] >= 1.0 && etsplit[1] >= 1.0))
		return 0;

	// make new corners and region
	p1 = corners[edge[1]].get();
	const float* p2 = corners[(edge[1]+1) % count].get();
	RegionPoint n2(p1[0] + tsplit[1] * (p2[0] - p1[0]),
				p1[1] + tsplit[1] * (p2[1] - p1[1]));
	p1 = corners[edge[0]].get();
	p2 = corners[(edge[0]+1) % count].get();
	RegionPoint n1(p1[0] + tsplit[0] * (p2[0] - p1[0]),
				p1[1] + tsplit[0] * (p2[1] - p1[1]));
	BzfRegion* newRegion = new BzfRegion;

	// add sides to new region and remove them from me.  the new region
	// must be to the right of the edge.  see if the corner after the
	// split on the first split edge is to the right or left of the
	// cutting edge.  if ambiguous, scan around until it's not.
	float t = 0.0;
	for (i = edge[0] + 1; fabs(t) < 1.0e-5 && i <= edge[1]; i++) {
		p1 = corners[i].get();
		t = dy * (e1[0] - p1[0]) - dx * (e1[1] - p1[1]);
	}

	if (dy * (e1[0] - p1[0]) - dx * (e1[1] - p1[1]) < 0.0) {
		// corner is to right of cutting edge -- new region between edge 0 and 1
		// add sides to new region
		newRegion->addSide(n1, neighbors[edge[0]]);
		for (i = edge[0] + 1; i < edge[1]; i++) {
			newRegion->addSide(corners[i], neighbors[i]);
			if (neighbors[i]) neighbors[i]->setNeighbor(this, newRegion);
		}
		newRegion->addSide(corners[edge[1]], neighbors[edge[1]]);
		newRegion->addSide(n2, this);

		// tell neighbors on split edges to split the shared edge at the
		// same place and to point to their new neighbor
		// tell them about their new neighbor.
		if (neighbors[edge[0]])
			neighbors[edge[0]]->splitEdge(this, newRegion, n1, true);
		if (neighbors[edge[1]])
			neighbors[edge[1]]->splitEdge(this, newRegion, n2, false);

		// remove old edges from myself and add new ones
		RegionPointList newCorners;
		RegionList newNeighbors;
		newCorners.push_back(n2);
		newNeighbors.push_back(neighbors[edge[1]]);
		for (i = edge[1] + 1; i < count; i++) {
			newCorners.push_back(corners[i]);
			newNeighbors.push_back(neighbors[i]);
		}
		for (i = 0; i <= edge[0]; i++) {
			newCorners.push_back(corners[i]);
			newNeighbors.push_back(neighbors[i]);
		}
		newCorners.push_back(n1);
		newNeighbors.push_back(newRegion);
		corners = newCorners;
		neighbors = newNeighbors;
	}
	else {
		// corner to left -- new region before edge 0 and after edge 1
		// add sides to new region
		newRegion->addSide(n2, neighbors[edge[1]]);
		for (i = edge[1] + 1; i < count; i++) {
			newRegion->addSide(corners[i], neighbors[i]);
			if (neighbors[i]) neighbors[i]->setNeighbor(this, newRegion);
		}
		for (i = 0; i < edge[0]; i++) {
			newRegion->addSide(corners[i], neighbors[i]);
			if (neighbors[i]) neighbors[i]->setNeighbor(this, newRegion);
		}
		newRegion->addSide(corners[edge[0]], neighbors[edge[0]]);
		newRegion->addSide(n1, this);

		// tell neighbors on split edges to split the shared edge at the
		// same place and to point to their new neighbor
		// tell them about their new neighbor.
		if (neighbors[edge[0]])
			neighbors[edge[0]]->splitEdge(this, newRegion, n1, false);
		if (neighbors[edge[1]])
			neighbors[edge[1]]->splitEdge(this, newRegion, n2, true);

		// remove old edges from myself and add new ones
		RegionPointList newCorners;
		RegionList newNeighbors;
		newCorners.push_back(n1);
		newNeighbors.push_back(neighbors[edge[0]]);
		for (i = edge[0] + 1; i <= edge[1]; i++) {
			newCorners.push_back(corners[i]);
			newNeighbors.push_back(neighbors[i]);
		}
		newCorners.push_back(n2);
		newNeighbors.push_back(newRegion);
		corners = newCorners;
		neighbors = newNeighbors;
	}

	// throw out degenerate edges in myself and newRegion
	tidy();
	newRegion->tidy();

	// all done
	return newRegion;
}

void					BzfRegion::splitEdge(const BzfRegion* oldNeighbor,
												BzfRegion* newNeighbor,
												const RegionPoint& p,
												bool onRight)
{
	// split my edge which has neighbor oldNeighbor at point p.
	// set the neighbor for the edge on the right if onRight is true
	// or on the left if onRight is false to newNeighbor.
	const int count = corners.size();
	for (int i = 0; i < count; i++)
		if (neighbors[i] == oldNeighbor) {
			corners.insert(&corners[i + 1], p);
			if (onRight) neighbors.insert(&neighbors[i], newNeighbor);
			else neighbors.insert(&neighbors[i + 1], newNeighbor);
			tidy();
			break;
		}
}

void					BzfRegion::addSide(const RegionPoint& p,
												BzfRegion* neighbor)
{
	corners.push_back(p);
	neighbors.push_back(neighbor);
}

void					BzfRegion::setNeighbor(const BzfRegion* oldNeighbor,
												BzfRegion* newNeighbor)
{
	const int count = corners.size();
	for (int i = 0; i < count; i++)
		if (neighbors[i] == oldNeighbor) {
			neighbors[i] = newNeighbor;
			break;
		}
}

void					BzfRegion::tidy()
{
	// throw out degenerate edges
	int count = corners.size();
	for (int i = 0; i < count; i++) {
		const float* p1 = corners[i].get();
		const float* p2 = corners[(i+1)%count].get();
		if (fabs(p1[0] - p2[0]) < 1.0e-5 && fabs(p1[1] - p2[1]) < 1.0e-5) {
			corners.erase(&corners[i]);
			neighbors.erase(&neighbors[i]);
			i--;
			count--;
		}
	}
}

bool					BzfRegion::test(int mailboxIndex)
{
	return (mailbox != mailboxIndex);
}

void					BzfRegion::setPathStuff(float _distance,
										BzfRegion* _target,
										const float _a[2], int mailboxIndex)
{
	distance = _distance;
	target = _target;
	A = RegionPoint(_a);
	mailbox = mailboxIndex;
}

float					BzfRegion::getDistance() const
{
	return distance;
}

BzfRegion*				BzfRegion::getTarget() const
{
	return target;
}

const float*				BzfRegion::getA() const
{
	return A.get();
}

//
// RegionPriorityQueue
//
// FIXME -- use a heap

RegionPriorityQueue::Node::Node(BzfRegion* _region, float _priority) :
								next(0), region(_region), priority(_priority)
{
}

RegionPriorityQueue::RegionPriorityQueue() : head(0)
{
}

RegionPriorityQueue::~RegionPriorityQueue()
{
	removeAll();
}

void					RegionPriorityQueue::insert(BzfRegion* region,
														float priority)
{
	Node* node = new Node(region, priority);
	if (!head || priority < head->priority) {
		node->next = head;
		head = node;
	}
	else {
		Node* scan = head;
		while (scan->next && priority >= scan->next->priority)
			scan = scan->next;
		node->next = scan->next;
		scan->next = node;
	}
}

BzfRegion*				RegionPriorityQueue::remove()
{
	Node* tmp = head;
	head = head->next;
	BzfRegion* region = tmp->region;
	delete tmp;
	return region;
}

void					RegionPriorityQueue::removeAll()
{
	while (head) {
		Node* next = head->next;
		delete head;
		head = next;
	}
	head = 0;
}

bool					RegionPriorityQueue::isEmpty() const
{
	return (head == 0);
}
// ex: shiftwidth=4 tabstop=4
