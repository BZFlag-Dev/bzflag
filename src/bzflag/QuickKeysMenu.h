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

#ifndef __QUICKKEYSMENU_H__
	#define __QUICKKEYSMENU_H__

// common - 1st one
	#include "common.h"

/* system interface headers */
	#include <string>

/* local interface headers */
	#include "HUDDialog.h"
	#include "HUDuiDefaultKey.h"
	#include "HUDuiLabel.h"
	#include "HUDuiTypeIn.h"

class QuickKeysMenu: public HUDDialog
{
public:
	QuickKeysMenu();
	~QuickKeysMenu();

	HUDuiDefaultKey *getDefaultKey();

	void show();
	void dismiss();
	void execute(){}
	void resize( int width, int height );

private:
	HUDuiLabel *createLabel( const std::string & );
	HUDuiTypeIn *createInput( const std::string & );
private:
	int firstKeyControl;
};


#endif /* __QUICKKEYSMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
