/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* Obstacle:
 *	Interface for all obstacles in the game environment,
 *	including boxes, pyramids, and teleporters.
 *
 * isInside(const float*, float) is a rough test that considers
 *	the tank as a circle
 * isInside(const float*, float, float, float) is a careful test
 *	that considers the tank as a rectangle
 */

#ifndef	BZF_OBSTACLE_H
#define	BZF_OBSTACLE_H

#include "common.h"
#include "Ray.h"
#include <string>

class ObstacleSceneNodeGenerator;
class WallSceneNode;

class Obstacle {
  public:
			Obstacle();
			Obstacle(const float* pos, float rotation,
				float hwidth, float hbreadth, float height, bool drive = false, bool shoot = false);
    virtual		~Obstacle();

    virtual std::string	getType() const = 0;

    const float*	getPosition() const;
    float		getRotation() const;
    float		getWidth() const;		// half width
    float		getBreadth() const;		// half breadth
    float		getHeight() const;		// full height

    virtual float	intersect(const Ray&) const = 0;
    virtual void	getNormal(const float* p, float* n) const = 0;
    virtual void	get3DNormal(const float* p, float* n) const;
    virtual bool	isInside(const float* p, float radius) const = 0;
    virtual bool	isInside(const float* p, float angle,
				float halfWidth, float halfBreadth) const = 0;
    virtual bool	isCrossing(const float* p, float angle,
				float halfWidth, float halfBreadth,
				float* plane) const;
    virtual bool	getHitNormal(
				const float* pos1, float azimuth1,
				const float* pos2, float azimuth2,
				float halfWidth, float halfBreadth,
				float* normal) const = 0;

    virtual ObstacleSceneNodeGenerator*	newSceneNodeGenerator() const = 0;

	bool isDriveThrough() const;
	bool isShootThrough() const;

	void setZFlip ( void );
	bool getZFlip ( void ) const;

  protected:
    float		getHitNormal(
				const float* pos1, float azimuth1,
				const float* pos2, float azimuth2,
				float halfWidth, float halfBreadth,
				const float* oPos, float oAzimuth,
				float oWidth, float oBreadth, float oHeight,
				float* normal) const;

  protected:
    float		pos[3];
    float		angle;
    float		width;
    float		breadth;
    float		height;
	bool		driveThrough;
	bool		shootThrough;
	bool		ZFlip;
};

class ObstacleSceneNodeGenerator {
  public:
    virtual		~ObstacleSceneNodeGenerator();

    virtual WallSceneNode* getNextNode(float uRepeats, float vRepeats,
							bool lod) = 0;

  protected:
			ObstacleSceneNodeGenerator();
    int			getNodeNumber() const;
    int			incNodeNumber();

  private:
    // no duplication
			ObstacleSceneNodeGenerator(const
					ObstacleSceneNodeGenerator&);
    ObstacleSceneNodeGenerator&	operator=(const ObstacleSceneNodeGenerator&);

  private:
    int			node;
};

class EmptySceneNodeGenerator : public ObstacleSceneNodeGenerator {
  public:
    virtual		~EmptySceneNodeGenerator();

    virtual WallSceneNode* getNextNode(float uRepeats, float vRepeats,
				       bool lod);
};

//
// Obstacle
//

inline const float*	Obstacle::getPosition() const
{
  return pos;
}

inline float		Obstacle::getRotation() const
{
  return angle;
}

inline float		Obstacle::getWidth() const
{
  return width;
}

inline float		Obstacle::getBreadth() const
{
  return breadth;
}

inline float		Obstacle::getHeight() const
{
  return height;
}

inline void		Obstacle::get3DNormal(const float *p, float *n) const
{
  getNormal(p, n);
}

//
// ObstacleSceneNodeGenerator
//

inline int		ObstacleSceneNodeGenerator::getNodeNumber() const
{
  return node;
}

inline int		ObstacleSceneNodeGenerator::incNodeNumber()
{
  return ++node;
}

#endif // BZF_OBSTACLE_H
// ex: shiftwidth=2 tabstop=8
