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

/* BzfDisplay:
 *	Encapsulates a display -- input and output devices.
 */

#ifndef BZF_DISPLAY_H
#define	BZF_DISPLAY_H

#include "common.h"

class BzfEvent;

class BzfDisplay {
  public:
			BzfDisplay();
    virtual		~BzfDisplay();

    virtual bool	isValid() const = 0;
    virtual bool	isEventPending() const = 0;
    virtual bool	getEvent(BzfEvent&) const = 0;
    virtual bool	peekEvent(BzfEvent&) const = 0;

    virtual bool	hasGetKeyMode() {return false;};
    virtual void	getModState(bool &shift, bool &control, bool &alt) {
      shift = false; control = false; alt = false;};

    int			getWidth() const;
    int			getHeight() const;
    void		setFullScreenFormat(int);

    void		setPassthroughSize(int w, int h);
    int			getPassthroughWidth() const;
    int			getPassthroughHeight() const;

  public:
    class ResInfo {
      public:
			ResInfo(const char* name, int w, int h, int r);
			~ResInfo();
      public:
	char*		name;
	int		width;
	int		height;
	int		refresh;
    };

    int			getNumResolutions() const;
    const ResInfo*	getResolution(int index) const;
    int			getResolution() const;
    int			getDefaultResolution() const;
    bool		setResolution(int index);
    bool		setDefaultResolution();
    int			findResolution(const char* name) const;
    bool		isValidResolution(int index) const;

  protected:
    void		initResolutions(ResInfo**, int num, int current);

  private:
			BzfDisplay(const BzfDisplay&);
    BzfDisplay&		operator=(const BzfDisplay&);

    virtual bool	doSetResolution(int) = 0;
    virtual bool	doSetDefaultResolution();

  private:
    int			passWidth, passHeight;
    int			numResolutions;
    int			defaultResolution;
    int			currentResolution;
    ResInfo**		resolutions;
  protected:
    int		 modeIndex;
};

#endif // BZF_DISPLAY_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

