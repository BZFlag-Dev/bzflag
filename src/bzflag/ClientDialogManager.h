/* bzflag
* Copyright (c) 1993-2014 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef BZF_CLIENTDIALOGMANAGER_H
#define BZF_CLIENTDIALOGMANAGER_H

#include "common.h"

#include <string>
#include <map>

#include "DialogData.h"

#include "HUDuiControl.h"

#include "BzfDisplay.h"
#include "MainWindow.h"
#include "SceneRenderer.h"

class ClientDialogControlSet {
public:
  const std::vector<HUDuiControl*>&	getControls() const { return controls; }
  std::vector<HUDuiControl*>&		getControls() { return controls; }

  std::vector<HUDuiControl*> controls;
  HUDuiControl*	focus;
};

class ClientDialogManager {
public:
  std::map<uint32_t, DialogData*> dialogData;
  std::map<uint32_t, ClientDialogControlSet*> dialogControls;
  DialogData* activeModalData;

  ClientDialogManager(const BzfDisplay* _display, const SceneRenderer& renderer);
  ~ClientDialogManager();

  uint32_t unpackDialogCreate(const void * msg);
  uint32_t unpackDialogDestroy(const void * msg);

  void render();
  void resize();
  int width, height;

private:
  static void	resizeCallback(void*);

  const BzfDisplay*	display;
  MainWindow&		window;
};


#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
