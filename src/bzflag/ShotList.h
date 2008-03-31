/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef __SHOT_LIST_H__
#define __SHOT_LIST_H__

#include "common.h"

/* system interface headers */
#include <map>
#include <vector>

#include "ShotPath.h"
#include "ShotUpdate.h"

class ShotList
{
public:
  ShotList();
  ~ShotList();

  ShotPath* getShot ( int GUID );
  std::vector<ShotPath*> getShotList ( void );
  std::vector<int> getExpiredShotList ( void );

  int addLocalShot ( FiringInfo * info );
  int addShot ( int GUID, FiringInfo * info );
  int updateShot( int GUID, int param, FiringInfo * info );
  
  bool removeShot ( int GUID );

  void updateAllShots ( float dt );

protected:
  std::map<int,ShotPath*> shots;

  int lastLocalShot;
};

#endif /* __SHOT_LIST_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
