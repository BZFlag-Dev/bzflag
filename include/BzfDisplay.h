/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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

    virtual boolean	isValid() const = 0;
    virtual boolean	isEventPending() const = 0;
    virtual boolean	getEvent(BzfEvent&) const = 0;

    int			getWidth() const;
    int			getHeight() const;

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
    boolean		setResolution(int index);
    int			getDefaultResolution() const;
    int			findResolution(const char* name) const;
    boolean		isValidResolution(int index) const;

  protected:
    void		initResolutions(ResInfo**, int num, int current);

  private:
			BzfDisplay(const BzfDisplay&);
    BzfDisplay&		operator=(const BzfDisplay&);

    virtual boolean	doSetResolution(int) = 0;

  private:
    int			numResolutions;
    int			defaultResolution;
    int			currentResolution;
    ResInfo**		resolutions;
};

#endif // BZF_DISPLAY_H
