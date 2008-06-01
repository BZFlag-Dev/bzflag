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

/*
 * HUDuiScrollList:
 *	FILL THIS IN
 */

#ifndef	__HUDUISCROLLLIST_H__
#define	__HUDUISCROLLLIST_H__

// ancestor class
#include "HUDuiControl.h"

#include "HUDuiLabel.h"

#include <string>
#include <vector>

#include "BzfEvent.h"

class HUDuiScrollList : public HUDuiControl {
	public:
			HUDuiScrollList();
			~HUDuiScrollList();
			HUDuiScrollList(bool paged);

		int getSelected() const;
		void setSelected(int _index);
		
		void addItem(HUDuiLabel* item);
		void addItem(std::string item); // BROKEN
		
		void update();
		void clear();
		
		void setSize(float width, float height);
		void setFontSize(float size);

		void setPaged(bool paged); // BROKEN

	protected:
		bool doKeyPress(const BzfKeyEvent&);
		bool doKeyRelease(const BzfKeyEvent&);
		void resizeLabels();
		void doRender();

	private:
		int	index;
		int visiblePosition;
		int numVisibleItems;
		int numVisibleChars;
		bool pagedList;
		
		HUDuiLabel* pageLabel;
		
		std::vector<HUDuiLabel*> labelList;
		std::vector<std::string> stringList;
};

#endif // __HUDUISCROLLLIST_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
