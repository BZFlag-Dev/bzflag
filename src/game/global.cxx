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
	{ "gravity",			"-9.8",		false, StateDatabase::Locked,    NULL },
	{ "worldSize",			"800.0",	false, StateDatabase::Locked,    NULL },
	{ "tankLength",			"6.0",		false, StateDatabase::Locked,    NULL },
	{ "tankWidth",			"2.8",		false, StateDatabase::Locked,    NULL },
	{ "tankHeight",			"2.05",		false, StateDatabase::Locked,    NULL },
	{ "tankRadius",			"4.32",		false, StateDatabase::Locked,    NULL },
	{ "muzzleHeight",		"1.57",		false, StateDatabase::Locked,    NULL },
	{ "muzzleFront",		"4.42",		false, StateDatabase::Locked,    NULL },
	{ "tankSpeed",			"25.0",		false, StateDatabase::Locked,    NULL },
	{ "tankAngVel",			"0.785398", false, StateDatabase::Locked,    NULL },
	{ "shotSpeed",			"100.0",	false, StateDatabase::Locked,    NULL },
	{ "shotRange",			"350.0",	false, StateDatabase::Locked,    NULL },
	{ "reloadTime",			"3.5",		false, StateDatabase::Locked,    NULL },
	{ "explodeTime",		"5.0",		false, StateDatabase::Locked,    NULL },
	{ "teleportTime",		"1.0",		false, StateDatabase::Locked,    NULL },
	{ "flagAltitude",		"11.0",		false, StateDatabase::Locked,    NULL },
	{ "flagRadius",			"2.5",		false, StateDatabase::Locked,    NULL },
	{ "velocityAd",			"1.5",		false, StateDatabase::Locked,    NULL },
	{ "angularAd",			"1.5",		false, StateDatabase::Locked,    NULL },
	{ "rFireAdVel",			"1.5",		false, StateDatabase::Locked,    NULL },
	{ "rFireAdRate",		"2.0",		false, StateDatabase::Locked,    NULL },
	{ "rFireAdLife",		"0.5",		false, StateDatabase::Locked,    NULL },
	{ "mGunAdVel",			"1.5",		false, StateDatabase::Locked,    NULL },
	{ "mGunAdRate",			"10.0",		false, StateDatabase::Locked,    NULL },
	{ "mGunAdLife",			"0.1",		false, StateDatabase::Locked,    NULL },
	{ "laserAdVel",			"1000.0",	false, StateDatabase::Locked,    NULL },
	{ "laserAdRate",		"0.5",		false, StateDatabase::Locked,    NULL },
	{ "laserAdLife",		"0.1",		false, StateDatabase::Locked,    NULL },
	{ "gMissileAng",		"0.628319", false, StateDatabase::Locked,    NULL },
	{ "tinyFactor",			"0.4",		false, StateDatabase::Locked,    NULL },
	{ "shieldFlight",		"2.7",		false, StateDatabase::Locked,    NULL },
	{ "srRadiusMult",		"2.0",		false, StateDatabase::Locked,    NULL },
	{ "shockAdLife",		"0.2",		false, StateDatabase::Locked,    NULL },
	{ "shockInRadius",		"6.0",		false, StateDatabase::Locked,    NULL },
	{ "shockOutRadius",		"60.0",		false, StateDatabase::Locked,    NULL },
	{ "jumpVelocity",		"19.0",		false, StateDatabase::Locked,    NULL },
	{ "identifyRange",		"50.0",		false, StateDatabase::Locked,    NULL },
	{ "obeseFactor",		"2.5",		false, StateDatabase::Locked,    NULL },
	{ "wideAngleAng",		"1.745329", false, StateDatabase::Locked,    NULL },
	{ "thiefVelAd",			"1.67",		false, StateDatabase::Locked,    NULL },
	{ "thiefTinyFactor",	"0.5",		false, StateDatabase::Locked,    NULL },
	{ "thiefAdShotVel",		"1000.0",	false, StateDatabase::Locked,    NULL },
	{ "thiefAdRate",		"10.0",		false, StateDatabase::Locked,    NULL },
	{ "momentumLinAcc",		"1.0",		false, StateDatabase::Locked,    NULL },
	{ "momentumAngAcc",		"1.0",		false, StateDatabase::Locked,    NULL },
	{ "burrowDepth",		"-1.52",	false, StateDatabase::Locked,    NULL },
	{ "burrowSpeedAd",		"0.75",		false, StateDatabase::Locked,    NULL },
	{ "burrowAngularAd",	"0.33",		false, StateDatabase::Locked,    NULL }
};
// ex: shiftwidth=4 tabstop=4
