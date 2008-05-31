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

HUDuiScrollList::HUDuiScrollList() : HUDuiControl(), index(-1), visiblePosition(0), numVisibleItems(0)
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
	stringList.clear();
	labelList.clear();
	setSelected(0);
}

void HUDuiScrollList::setSelected(int _index)
{
	if (_index < 0)
		_index = 0;
	else if (_index >= (int)labelList.size())
		_index = (int)labelList.size() - 1;
	
	// The new index falls within the portion of the list already on screen
	if ((_index >= index - visiblePosition)&&(_index < (index + (numVisibleItems - visiblePosition))))
	{
		visiblePosition = visiblePosition + (_index - index);
	}
	// Moving one down outside of list range
	else if (_index == (index + (numVisibleItems - visiblePosition)))
	{
		visiblePosition = numVisibleItems - 1;
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
	stringList.push_back(item->getString());
	labelList.push_back(item);
	update();
}

// BROKEN
void HUDuiScrollList::addItem(std::string item)
{
	HUDuiLabel* newLabel = new HUDuiLabel;
	newLabel->setFontFace(getFontFace());
	newLabel->setString(item);
	
	addItem(newLabel);
}
// BROKEN

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

void HUDuiScrollList::setSize(float width, float height)
{
	HUDuiControl::setSize(width, height);
	resizeLabels();
}

void HUDuiScrollList::setFontSize(float size)
{
	HUDuiControl::setFontSize(size);
	resizeLabels();
}

void HUDuiScrollList::resizeLabels()
{
	// Determine how many items are visible
	FontManager &fm = FontManager::instance();
	float itemHeight = fm.getStringHeight(getFontFace()->getFMFace(), getFontSize());
	float listHeight = getHeight();
	numVisibleItems = listHeight/itemHeight;

	// Determine how far right we can display
	float width = getWidth();
	float itemWidth = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), "D");
	numVisibleChars = width/itemWidth;
	
	std::string tempString;
	HUDuiLabel* item;
	
	for (int i = 0; i < stringList.size(); i++)
	{
		tempString = stringList.at(i);
		item = labelList.at(i);
		
		if (tempString.length() > numVisibleChars)
		{
			tempString = tempString.substr(0, numVisibleChars);
			item->setString(tempString);
		}
		else
		{
			item->setString(tempString);
		}
	}
}

void HUDuiScrollList::doRender()
{
	FontManager &fm = FontManager::instance();
	float itemHeight = fm.getStringHeight(getFontFace()->getFMFace(), getFontSize());
	
	for (int i = (getSelected() - visiblePosition); i<((numVisibleItems - visiblePosition) + getSelected()); i++)
	{
		if (i < labelList.size())
		{
			HUDuiLabel* item = labelList.at(i);
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
