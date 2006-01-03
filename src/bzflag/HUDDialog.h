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
class HUDuiControl;
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

    HUDuiControl*		getFocus() const;
    void			setFocus(HUDuiControl*);

    void			initNavigation(std::vector<HUDuiControl*> &list, int start, int end);



  protected:
    const std::vector<HUDuiControl*>&	getControls() const { return list; }
    std::vector<HUDuiControl*>&		getControls() { return list; }

    int				getHeight() const { return height; }
    int				getWidth() const { return width; }

  protected:
    int				height, width;

  private:
    std::vector<HUDuiControl*>	list;
    HUDuiControl*		focus;
};


#endif /* __HUDDIALOG_H__ */

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

