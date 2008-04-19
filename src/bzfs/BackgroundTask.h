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
#ifndef __BACKGROUND_TASK_H__
#define __BACKGROUND_TASK_H__

#include "common.h"
#include "Singleton.h"
#include "bzfsAPI.h"

#include <vector>

class BackgroundTask
{
public:
  virtual ~BackgroundTask(){};
  virtual bool process ( void *param ) = 0;
};

typedef bool (*BackgroundTaskFunc) ( void *param );

class BackgroundTaskManager: public Singleton<BackgroundTaskManager> , bz_EventHandler
{
public:

  virtual void process ( bz_EventData *eventData );

  void addTask ( BackgroundTask *task, void *param );
  void addTask ( BackgroundTaskFunc task, void *param );

  void removeTask ( BackgroundTask *task, void *param );
  void removeTask ( BackgroundTaskFunc task, void *param );

  void processTasks ( void );

protected:
  friend class Singleton<BackgroundTaskManager>;

private:
  BackgroundTaskManager();
  ~BackgroundTaskManager();

  typedef struct 
  {
    BackgroundTask* classCB;
    BackgroundTaskFunc funcCB;
    void *	      param;
  }Task;
  std::vector<Task> tasks;
};

#endif /* __BACKGROUND_TASK_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
