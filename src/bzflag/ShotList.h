/* bzflag
* Copyright (c) 1993 - 2009 Tim Riker
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

/* common interface headers */
#include "Singleton.h"
#include "ShotPath.h"
#include "ShotUpdate.h"

class ShotList : public Singleton<ShotList>
{
public:
  ShotPath* getShot ( int GUID );
  std::vector<ShotPath*> getShotList ( void );
  std::vector<ShotPath*> getLocalShotList ( void );
  std::vector<int> getExpiredShotList ( void );

  int addLocalShot ( FiringInfo * info );
  int addShot ( int GUID, FiringInfo * info );
  int updateShot( int GUID, int param, FiringInfo * info );

  bool removeShot ( int GUID );

  void updateAllShots ( float dt );
  void flushExpiredShots ( void );

  void clear ( void );

protected:
  friend class Singleton<ShotList>;

  std::map<int,ShotPath*> shots;

  int lastLocalShot;

private:
  // default constructor/destructor protected because of singleton
  ShotList();
  ~ShotList();

};

#endif /* __SHOT_LIST_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
