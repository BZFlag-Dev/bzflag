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

/* Teleporter:
 *	Encapsulates a box in the game environment.
 */

#ifndef BZF_TELEPORTER_H
#define BZF_TELEPORTER_H

#include "Obstacle.h"
#include <vector>

class Teleporter : public Obstacle {
public:
	Teleporter(const float* pos, float rotation,
							float width, float breadth, float height,
							float borderSize);
	~Teleporter();

	std::string			getType() const;
	static std::string	getClassName(); // const

	float				getBorder() const;

	float				intersect(const Ray&) const;
	void				getNormal(const float* p, float* n) const;
	bool				isInside(const float* p, float radius) const;
	bool				isInside(const float* p, float angle,
							float halfWidth, float halfBreadth) const;
	bool				isCrossing(const float* p, float angle,
							float halfWidth, float halfBreadth,
							float* plane) const;
	bool				getHitNormal(
							const float* pos1, float azimuth1,
							const float* pos2, float azimuth2,
							float halfWidth, float halfBreadth,
							float* normal) const;

	float				isTeleported(const Ray&, int& face) const;
	float				getProximity(const float* p, float radius) const;
	bool				hasCrossed(
							const float* p1, const float* p2, int& face) const;
	void				getPointWRT(const Teleporter& t2, int face1, int face2,
							const float* pIn, const float* dIn, float aIn,
							float* pOut, float* dOut, float* aOut) const;

private:
	float				border;
	static std::string	typeName;
};

typedef std::vector<Teleporter> Teleporters;

//
// Teleporter
//

inline
float					Teleporter::getBorder() const
{
	return border;
}

#endif // BZF_TELEPORTER_H
// ex: shiftwidth=4 tabstop=4
