/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __OBSTACLESCENENODEGENERATOR_H__
#define __OBSTACLESCENENODEGENERATOR_H__

class WallSceneNode;

class ObstacleSceneNodeGenerator
{
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

inline int		ObstacleSceneNodeGenerator::getNodeNumber() const
{
  return node;
}

inline int		ObstacleSceneNodeGenerator::incNodeNumber()
{
  return ++node;
}


#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
