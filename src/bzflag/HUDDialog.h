/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * HUDDialog:
 *	A dialog of HUDuiControls.
 */

#ifndef	BZF_HUDDIALOG_H
#define	BZF_HUDDIALOG_H

#include "HUDui.h"
#include <vector>

class HUDDialog {
  public:
			HUDDialog();
    virtual		~HUDDialog();

    void			render();

    virtual HUDuiDefaultKey*	getDefaultKey() = 0;
    virtual void		show() { }
    virtual void		execute() = 0;
    virtual void		dismiss() { }
    virtual void		resize(int width, int height) = 0;

    HUDuiControl*		getFocus() const;
    void			setFocus(HUDuiControl*);



  protected:
    const std::vector<HUDuiControl*>&	getControls() const { return list; }
    std::vector<HUDuiControl*>&		getControls() { return list; }

  private:
    std::vector<HUDuiControl*>	list;
    HUDuiControl*	focus;
};

class HUDDialogStack {
  public:
    static HUDDialogStack*	get();

    bool		isActive() const;
    HUDDialog*		top() const;
    void		push(HUDDialog*);
    void		pop();

    void		render();

			HUDDialogStack();
			~HUDDialogStack();

  private:
    static void		resize(void*);

  private:
    std::vector<HUDDialog*>	stack;
    static HUDDialogStack globalStack;
};

#endif /* BZF_HUDDIALOG_H */
// ex: shiftwidth=2 tabstop=8
