/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	BZF_OBSTACLE_MODIFIER_H
#define	BZF_OBSTACLE_MODIFIER_H


class Obstacle;
class GroupInstance;

class ObstacleModifier {
  public:
    ObstacleModifier();
    ObstacleModifier(const ObstacleModifier& obsMod,
                     const GroupInstance& grpinst);
    ~ObstacleModifier();
        
    void execute(Obstacle* obstacle) const;

  private:
    bool modifyTeam;
    int team;
    bool modifyColor;
    float tint[4];
    bool modifyPhysicsDriver;
    int phydrv;
};


#endif // BZF_OBSTACLE_MODIFIER_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

