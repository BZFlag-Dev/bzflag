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

/* interface header */
#include "LoginMenu.h"

/* common implementation headers */
#include "FontManager.h"

/* local implementation headers */
#include "HUDui.h"
#include "MainMenu.h"
#include "MenuDefaultKey.h"
#include "ClientAuthentication.h"

LoginMenu::LoginMenu()
{
	// cache font face ID
	int fontFace = MainMenu::getFontFace();

	// add controls
	std::vector < HUDuiControl * >  &list = getControls();

	HUDuiLabel *label = new HUDuiLabel;
	label->setFontFace( fontFace );
	label->setString( "Login" );
	list.push_back( label );

	username = new HUDuiTypeIn;
	username->setFontFace( fontFace );
	username->setLabel( "Username:" );
	username->setMaxLength( 20 );
	list.push_back( username );

	password = new HUDuiTypeIn;
	password->setFontFace( fontFace );
	password->setLabel( "Password:" );
	password->setMaxLength( 20 );
	list.push_back( password );

	login = new HUDuiLabel;
	login->setFontFace( fontFace );
	login->setString( "Login" );
	list.push_back( login );

	initNavigation( list, 1, ( int )list.size() - 1 );
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

LoginMenu::~LoginMenu(){}

HUDuiDefaultKey *LoginMenu::getDefaultKey()
{
	return MenuDefaultKey::getInstance();
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

void LoginMenu::execute()
{
	HUDuiControl *focus = HUDui::getFocus();
	if( focus == login )
	{
		// Initializing Authentication
		ClientAuthentication::login( username->getString().c_str(), password->getString().c_str());
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

void LoginMenu::resize( int width, int height )
{
	int i;
	HUDDialog::resize( width, height );

	// use a big font for title, smaller font for the rest
	const float titleFontSize = ( float )height / 15.0f;
	const float fontSize = ( float )height / 30.0f;

	FontManager &fm = FontManager::instance();

	// reposition title
	std::vector < HUDuiControl * >  &list = getControls();
	HUDuiLabel *title = ( HUDuiLabel* )list[0];
	title->setFontSize( titleFontSize );
	const float titleWidth = fm.getStrLength( MainMenu::getFontFace(), titleFontSize, title->getString());
	const float titleHeight = fm.getStrHeight( MainMenu::getFontFace(), titleFontSize, "" );
	float x = 0.5f *(( float )width - titleWidth );
	float y = ( float )height - titleHeight;
	title->setPosition( x, y );

	// reposition options
	x = 0.5f *(( float )width - 0.5f * titleWidth );
	y -= 0.6f * titleHeight;
	const int count = ( const int )list.size();
	const float h = fm.getStrHeight( MainMenu::getFontFace(), fontSize, " " );
	for( i = 1; i < count; i++ )
	{
		list[i]->setFontSize( fontSize );
		list[i]->setPosition( x, y );
		y -= 1.0f * h;
	}
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
