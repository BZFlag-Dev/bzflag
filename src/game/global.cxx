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

#include "global.h"

GlobalDBItem				globalDBItems[48] = {
	{ "_gravity",			"-9.8",		false, StateDatabase::Locked},
	{ "_worldSize",			"800.0",	false, StateDatabase::Locked},
	{ "_tankLength",		"6.0",		false, StateDatabase::Locked},
	{ "_tankWidth",			"2.8",		false, StateDatabase::Locked},
	{ "_tankHeight",		"2.05",		false, StateDatabase::Locked},
	{ "_tankRadius",		"4.32",		false, StateDatabase::Locked},
	{ "_muzzleHeight",		"1.57",		false, StateDatabase::Locked},
	{ "_muzzleFront",		"4.42",		false, StateDatabase::Locked},
	{ "_tankSpeed",			"25.0",		false, StateDatabase::Locked},
	{ "_tankAngVel",		"0.785398", false, StateDatabase::Locked},
	{ "_shotSpeed",			"100.0",	false, StateDatabase::Locked},
	{ "_shotRange",			"350.0",	false, StateDatabase::Locked},
	{ "_reloadTime",		"3.5",		false, StateDatabase::Locked},
	{ "_explodeTime",		"5.0",		false, StateDatabase::Locked},
	{ "_teleportTime",		"1.0",		false, StateDatabase::Locked},
	{ "_flagAltitude",		"11.0",		false, StateDatabase::Locked},
	{ "_flagRadius",		"2.5",		false, StateDatabase::Locked},
	{ "_velocityAd",		"1.5",		false, StateDatabase::Locked},
	{ "_angularAd",			"1.5",		false, StateDatabase::Locked},
	{ "_rFireAdVel",		"1.5",		false, StateDatabase::Locked},
	{ "_rFireAdRate",		"2.0",		false, StateDatabase::Locked},
	{ "_rFireAdLife",		"0.5",		false, StateDatabase::Locked},
	{ "_mGunAdVel",			"1.5",		false, StateDatabase::Locked},
	{ "_mGunAdRate",		"10.0",		false, StateDatabase::Locked},
	{ "_mGunAdLife",		"0.1",		false, StateDatabase::Locked},
	{ "_laserAdVel",		"1000.0",	false, StateDatabase::Locked},
	{ "_laserAdRate",		"0.5",		false, StateDatabase::Locked},
	{ "_laserAdLife",		"0.1",		false, StateDatabase::Locked},
	{ "_gMissileAng",		"0.628319", false, StateDatabase::Locked},
	{ "_tinyFactor",		"0.4",		false, StateDatabase::Locked},
	{ "_shieldFlight",		"2.7",		false, StateDatabase::Locked},
	{ "_srRadiusMult",		"2.0",		false, StateDatabase::Locked},
	{ "_shockAdLife",		"0.2",		false, StateDatabase::Locked},
	{ "_shockInRadius",		"6.0",		false, StateDatabase::Locked},
	{ "_shockOutRadius",	"60.0",		false, StateDatabase::Locked},
	{ "_jumpVelocity",		"19.0",		false, StateDatabase::Locked},
	{ "_identifyRange",		"50.0",		false, StateDatabase::Locked},
	{ "_obeseFactor",		"2.5",		false, StateDatabase::Locked},
	{ "_wideAngleAng",		"1.745329", false, StateDatabase::Locked},
	{ "_thiefVelAd",		"1.67",		false, StateDatabase::Locked},
	{ "_thiefTinyFactor",	"0.5",		false, StateDatabase::Locked},
	{ "_thiefAdShotVel",	"1000.0",	false, StateDatabase::Locked},
	{ "_thiefAdRate",		"10.0",		false, StateDatabase::Locked},
	{ "_momentumLinAcc",	"1.0",		false, StateDatabase::Locked},
	{ "_momentumAngAcc",	"1.0",		false, StateDatabase::Locked},
	{ "_burrowDepth",		"-1.32",	false, StateDatabase::Locked},
	{ "_burrowSpeedAd",		"0.75",		false, StateDatabase::Locked},
	{ "_burrowAngularAd",	"0.33",		false, StateDatabase::Locked}
};
// ex: shiftwidth=4 tabstop=4
