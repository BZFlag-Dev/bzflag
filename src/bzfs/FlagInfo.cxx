/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* no header other than FlagInfo.h should be included here */

#ifdef _MSC_VER
#pragma warning( 4:4786)
#endif

#include "FlagInfo.h"


/* private */

/* protected */

/* public */

// flags list
FlagInfo *FlagInfo::flagList = NULL;

FlagInfo::FlagInfo()
{
  // prep flag
  flag.type               = Flags::Null;
  flag.status             = FlagNoExist;
  flag.endurance          = FlagNormal;
  flag.owner              = NoPlayer;
  flag.position[0]        = 0.0f;
  flag.position[1]        = 0.0f;
  flag.position[2]        = 0.0f;
  flag.launchPosition[0]  = 0.0f;
  flag.launchPosition[1]  = 0.0f;
  flag.launchPosition[2]  = 0.0f;
  flag.landingPosition[0] = 0.0f;
  flag.landingPosition[1] = 0.0f;
  flag.landingPosition[2] = 0.0f;
  flag.flightTime         = 0.0f;
  flag.flightEnd          = 0.0f;
  flag.initialVelocity    = 0.0f;
  player                  = -1;
  grabs                   = 0;
}

void FlagInfo::setSize(int numFlags)
{
  delete[] flagList;
  flagList = new FlagInfo[numFlags];
}

void FlagInfo::setRequiredFlag(FlagType *desc)
{
  required = true;
  flag.type = desc;
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

