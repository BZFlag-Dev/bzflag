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
#include "HUDuiScrollList.h"

// common implementation headers
#include "BundleMgr.h"
#include "Bundle.h"
#include "FontManager.h"
#include "LocalFontFace.h"

//
// HUDuiScrollList
//

HUDuiScrollList::HUDuiScrollList() : HUDuiControl(), index(-1), visiblePosition(0)
{
  // do nothing
}

HUDuiScrollList::~HUDuiScrollList()
{
  // do nothing
}

int HUDuiScrollList::getSelected() const
{
  return index;
}

void HUDuiScrollList::clear()
{
	list.clear();
	setSelected(0);
}

void HUDuiScrollList::setSelected(int _index)
{
	// Determine how many list items should be visible
	FontManager &fm = FontManager::instance();
	float height = getHeight();
	float itemHeight = fm.getStringHeight(getFontFace()->getFMFace(), getFontSize());
	int numItems = height/itemHeight;

	if (_index < 0)
		_index = 0;
	else if (_index >= (int)list.size())
		_index = (int)list.size() - 1;
	
	// The new index falls within the portion of the list already on screen
	if ((_index >= index - visiblePosition)&&(_index < (index + (numItems - visiblePosition))))
	{
		visiblePosition = visiblePosition + (_index - index);
	}
	// Moving one down outside of list range
	else if (_index == (index + (numItems - visiblePosition)))
	{
		visiblePosition = numItems - 1;
	}
	// The new index isn't already on screen
	else
	{
		// Jump to that part of the list and set the new index as first
		visiblePosition = 0;
	}
	
	index = _index;
}

void HUDuiScrollList::addItem(HUDuiLabel* item)
{
	list.push_back(item);
	update();
}

void HUDuiScrollList::update()
{
	setSelected(index);
}

bool HUDuiScrollList::doKeyPress(const BzfKeyEvent& key)
{
	if (key.chr == 0)
		switch (key.button) {
			case BzfKeyEvent::Up:
				if (index != -1) {
					setSelected(index - 1);
					doCallback();
				}
				break;
				
			case BzfKeyEvent::Down:
				if (index != -1) {
					setSelected(index + 1);
					doCallback();
				}
				break;

			default:
				return false;
	}

	switch (key.chr) {
		case 13:
		case 27:
			return false;
	}

	return true;
}

bool HUDuiScrollList::doKeyRelease(const BzfKeyEvent&)
{
  // ignore key releases
  return false;
}

/* Not working at the moment, not sure why
void HUDuiScrollList::setSize(float width, float height)
{
	HUDuiElement::setSize(width, height);

	FontManager &fm = FontManager::instance();
	float itemHeight = fm.getStringHeight(getFontFace()->getFMFace(), getFontSize());
	float listHeight = getHeight();
	numItems = listHeight/itemHeight;
}
*/

void HUDuiScrollList::doRender()
{
	FontManager &fm = FontManager::instance();
	float itemHeight = fm.getStringHeight(getFontFace()->getFMFace(), getFontSize());
	float height = getHeight();
	int numItems = height/itemHeight;
	
	for (int i = (getSelected() - visiblePosition); i<((numItems - visiblePosition) + getSelected()); i++)
	{
		if (i < list.size())
		{
			HUDuiLabel* item = list.at(i);
			item->setFontSize(getFontSize());
			item->setPosition(getX(), (getY() - itemHeight*(i-(getSelected() - visiblePosition))));
			item->setDarker(i != getSelected());
			item->render();
		}
	}
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
