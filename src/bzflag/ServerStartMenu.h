/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __SERVERSTARTMENU_H__
	#define __SERVERSTARTMENU_H__

	#include "common.h"

/* interface header */
	#include "HUDDialog.h"

/* system interface headers */
	#include <vector>
	#include <string>
	#include <map>

/* local interface headers */
	#include "HUDuiDefaultKey.h"
	#include "HUDuiList.h"
	#include "HUDuiLabel.h"

class ServerStartMenu: public HUDDialog
{
public:
	ServerStartMenu();
	~ServerStartMenu();

	HUDuiDefaultKey *getDefaultKey();
	void execute();
	void show();
	void dismiss();
	void resize( int width, int height );

	static const char *getSettings()
	{
		return settings;
	} static void setSettings( const char* );

private:
	HUDuiList *createList( const char* );
	HUDuiLabel *createLabel( const char* );
	void setStatus( const char *, const std::vector < std::string >  *parms = NULL );
	void loadSettings();
	void scanWorldFiles( const std::string &searchDir, std::vector < std::string >  *items );

private:
	float center;
	HUDuiLabel *start;
	HUDuiLabel *status;
	HUDuiLabel *failedMessage;
	static char settings[];
	std::map < std::string, std::string > worldFiles;
};


#endif /* __SERVERSTARTMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
