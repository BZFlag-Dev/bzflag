/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "ViewItems.h"
#include "ViewItemHUD.h"
#include "ViewItemPlayerScene.h"
#include "ViewItemRadar.h"
#include "ViewItemScoreboard.h"
#include "ViewManager.h"

//
// ViewItems
//

void					ViewItems::init()
{
	// register tag readers
	VIEWMGR->addReader("scene", new ViewItemPlayerSceneReader);
	VIEWMGR->addReader("radar", new ViewItemRadarReader);
	VIEWMGR->addReader("hud", new ViewItemHUDReader);
	VIEWMGR->addReader("score", new ViewItemScoreboardReader);
}

void					ViewItems::fini()
{
	// unregister tag readers
	VIEWMGR->removeReader("scene");
	VIEWMGR->removeReader("radar");
	VIEWMGR->removeReader("hud");
	VIEWMGR->removeReader("score");
}
