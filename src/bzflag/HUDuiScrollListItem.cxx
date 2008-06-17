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

// interface headers
#include "HUDuiScrollListItem.h"

// common implementation headers
#include "BundleMgr.h"
#include "Bundle.h"
#include "FontManager.h"
#include "LocalFontFace.h"
#include "HUDui.h"

//
// HUDuiScrollListItemItem
//

HUDuiScrollListItem::HUDuiScrollListItem(std::string data) : HUDuiControl(), stringValue(data), label(new HUDuiLabel)
{
  label->setString(data);
}

HUDuiScrollListItem::HUDuiScrollListItem(HUDuiLabel* data) : HUDuiControl(), stringValue(data->getString()), label(data)
{
  // do nothing
}

HUDuiScrollListItem::~HUDuiScrollListItem()
{
  // do nothing
}

// Update our scrollable list whe the font size is changed
void HUDuiScrollListItem::setFontSize(float size)
{
	HUDuiControl::setFontSize(size);
	label->setFontSize(size);
}

std::string HUDuiScrollListItem::getValue()
{
	return stringValue;
}

// Change our label sizes to match any changes to our scrollable list
void HUDuiScrollListItem::shorten(float width)
{
	FontManager &fm = FontManager::instance();

	// Determine how far right we can display
	//float width = getWidth();
	float itemWidth = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), "D");
	int numVisibleChars = width/itemWidth;
	
	std::string tempString = stringValue;
		
	// Shorten the label to fit within the scrollable list
	if (tempString.length() > numVisibleChars) {
		tempString = tempString.substr(0, numVisibleChars);
	}
		
	label->setString(tempString);
}

void HUDuiScrollListItem::doRender()
{
	label->render();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
