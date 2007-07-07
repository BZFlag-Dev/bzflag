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

/*
 * BZRobot: A class for simulation and implementation of a Frontend robot.
 */

#ifndef	BZROBOT_H_20070707
#define	BZROBOT_H_20070707

#include "BZAdvancedRobot.h"

class BZRobot :public BZAdvancedRobot
{
    protected:
        void ahead(double distance);
        void doNothing();
        void fire();
        // TODO: Implement 'Bullet fireBullet();' ?
        void resume();
        void stop();
        void stop(bool overwrite);
        // TODO: void scan(); ?
        void turnLeft(double degrees);
    public:

};

#endif
