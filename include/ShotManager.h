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

/*
* ShotPath:
*	Encapsulates the path a shot follows.  Most paths can
*	be computed at the instant of firing (though they may
*	terminate early because of a hit).  Some paths need
*	to be updated continuously during flight.
*
* RemoteShotPath:
*	A ShotPath acting as a proxy for a remote ShotPath.
*	Created by a LocalPlayer on behalf of a RemotePlayer.
*/

#ifndef	__SHOTMANGER_H__
#define	__SHOTMANGER_H__

#include "common.h"

/* system interface headers */
#include <map>
#include <vector>

/* common interface headers */
#include "Singleton.h"
#include "ShotUpdate.h"

class ShotEventCallbacks
{
public:
  virtual ~ShotEventCallbacks(){};

  virtual void shotEnded ( int id ) = 0;
  virtual void shotStarted ( int id ) = 0;
  virtual void shotUpdated ( int id ) = 0;
};

class ShotManager  : public Singleton<ShotManager>
{
public:
  int newShot ( FiringInfo *info, int param );
  void update ( double dt );

  void addEventHandler ( ShotEventCallbacks *cb );
  void removeEventHandler ( ShotEventCallbacks *cb );

  class Shot
  {
  public:
    Shot(FiringInfo* info, int GUID, int p = 0);

    typedef enum
    {
      Stop,
      Ignore,
      Reflect
    }ObstacleMode;

  protected:
    int param;
    int id;

   // TeamColor team;
  //  FlagType flag;
 //   ShotType type;
    ObstacleMode mode;

    double startTime;
    double lastUpdateTime;

    double lifetime;
    double range;
    
    float pos[3];
    float vec[3];
  };

protected:
  friend class Singleton<ShotManager>;
 
private:
  ShotManager();
  ~ShotManager();

  std::map<int,Shot> shots;

  std::vector<ShotEventCallbacks*> callbacks;
};

#endif /* __SHOTMANAGERH__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
