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
#include "bzUnicode.h"

//
// HUDuiScrollListItemItem
//

HUDuiScrollListItem::HUDuiScrollListItem() : HUDuiControl(), stringValue(""), label(new HUDuiLabel)
{
	label->setString("");
	label->setFontFace(getFontFace());
}

HUDuiScrollListItem::HUDuiScrollListItem(std::string data) : HUDuiControl(), stringValue(data), label(new HUDuiLabel)
{
	label->setString(data);
	label->setFontFace(getFontFace());
}

HUDuiScrollListItem::HUDuiScrollListItem(HUDuiLabel* data) : HUDuiControl(), stringValue(data->getString()), label(data)
{
	label->setFontFace(getFontFace());
}

HUDuiScrollListItem::~HUDuiScrollListItem()
{
  // do nothing
}

void HUDuiScrollListItem::setFontSize(float size)
{
	HUDuiControl::setFontSize(size);
	label->setFontSize(size);
}

void HUDuiScrollListItem::setFontFace(const LocalFontFace* fontFace)
{
	HUDuiControl::setFontFace(fontFace);
	label->setFontFace(fontFace);
}

void HUDuiScrollListItem::setPosition(float x, float y)
{
	HUDuiControl::setPosition(x, y);
	label->setPosition(x, y);
}

std::string HUDuiScrollListItem::getValue()
{
	return stringValue;
}


// Shorten the item's label to fit
void HUDuiScrollListItem::shorten(float width)
{
	// Trim string to fit our available space
	FontManager &fm = FontManager::instance();
	std::string tempStr = stringValue;

	// Skip if it already fits
	if (fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), tempStr.c_str()) <= width)
		return;

	// Iterate through each character. Expensive.
	for (int i=0; i<(int)tempStr.size(); i++)
	{
		// Is it too big yet?
		if (fm.getStringWidth(label->getFontFace()->getFMFace(), label->getFontSize(), 
						tempStr.substr(0, i).c_str()) > width) {
			label->setString(tempStr.substr(0, i - 1));
			break;
		}
	}
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
