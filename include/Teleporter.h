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

#ifndef	BZF_TELEPORTER_H
#define	BZF_TELEPORTER_H

#include "AList.h"
#include "Obstacle.h"

class Teleporter : public Obstacle {
  public:
			Teleporter(const float* pos, float rotation,
				float width, float breadth, float height,
				float borderSize);
			~Teleporter();

    BzfString		getType() const;
    static BzfString	getClassName(); // const

    float		getBorder() const;

    float		intersect(const Ray&) const;
    void		getNormal(const float* p, float* n) const;
    boolean		isInside(const float* p, float radius) const;
    boolean		isInside(const float* p, float angle,
				float halfWidth, float halfBreadth) const;
    boolean		isCrossing(const float* p, float angle,
				float halfWidth, float halfBreadth,
				float* plane) const;
    boolean		getHitNormal(
				const float* pos1, float azimuth1,
				const float* pos2, float azimuth2,
				float halfWidth, float halfBreadth,
				float* normal) const;

    float		isTeleported(const Ray&, int& face) const;
    float		getProximity(const float* p, float radius) const;
    boolean		hasCrossed(const float* p1, const float* p2,
							int& face) const;
    void		getPointWRT(const Teleporter& t2, int face1, int face2,
				const float* pIn, const float* dIn, float aIn,
				float* pOut, float* dOut, float* aOut) const;

    ObstacleSceneNodeGenerator*	newSceneNodeGenerator() const;

  private:
    float		border;
    static BzfString	typeName;
};

BZF_DEFINE_ALIST(Teleporters, Teleporter);

class TeleporterSceneNodeGenerator : public ObstacleSceneNodeGenerator {
  friend class Teleporter;
  public:
			~TeleporterSceneNodeGenerator();

    WallSceneNode*	getNextNode(float, float, boolean);

  protected:
			TeleporterSceneNodeGenerator(const Teleporter*);

  private:
    const Teleporter*	teleporter;
};

//
// Teleporter
//

inline float		Teleporter::getBorder() const
{
  return border;
}

#endif // BZF_TELEPORTER_H
// ex: shiftwidth=2 tabstop=8
