/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	__HUDDIALOG_H__
#define	__HUDDIALOG_H__

/* common */
#include "common.h"

#if defined(_MSC_VER)
	#pragma warning(disable: 4786)
#endif

/* system headers */
#include <vector>

/* local interface headers */
#include "HUDNavigationQueue.h"
class HUDuiControl;
class HUDuiElement;
class HUDuiDefaultKey;

/**
 * HUDDialog:
 *	A dialog of HUDuiControls.
 */
class HUDDialog {
  public:
			HUDDialog();
    virtual		~HUDDialog();

    void			render();

    virtual HUDuiDefaultKey*	getDefaultKey() = 0;
    virtual void		show() { }
    virtual void		execute() = 0;
    virtual void		dismiss() { }
    virtual void		resize(int _width, int _height);
    virtual void		setFailedMessage(const char *) {;};
    
    HUDuiControl*		getFocus() const { return navList.get(); }

  protected:
    void			addControl(HUDuiElement* element);
    void			addControl(HUDuiControl* control, bool navigable = true);

    const HUDNavigationQueue&	getNav() const { return navList; }
    HUDNavigationQueue&		getNav() { return navList; }

    const std::vector<HUDuiElement*>&	getElements() const { return renderList; }
    std::vector<HUDuiElement*>&		getElements() { return renderList; }

    int				getHeight() const { return height; }
    int				getWidth() const { return width; }

    void			initNavigation();

  protected:
    int				height, width;

  private:
    /* renderList contains all elements which are to be rendered.
     * navList contains all elements which the user can navigate to.
     * "Ordinary" controls will typically be in both lists.
     * Elements which are on not on the render list will not be automatically
     * deleted.
     */
    std::vector<HUDuiElement*>	renderList;
    HUDNavigationQueue		navList;
    HUDuiControl*		focus;
};


#endif /* __HUDDIALOG_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
