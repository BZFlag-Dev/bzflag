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

#include "BzfDisplay.h"
#include <string.h>

//
// BzfDisplay::ResInfo
//

BzfDisplay::ResInfo::ResInfo(const char* _name, int w, int h, int r)
{
  name = new char[strlen(_name) + 1];
  strcpy(name, _name);
  width = w;
  height = h;
  refresh = r;
}

BzfDisplay::ResInfo::~ResInfo()
{
  delete[] name;
}

//
// BzfDisplay
//

BzfDisplay::BzfDisplay() : passWidth(640), passHeight(480),
				numResolutions(0),
				defaultResolution(-1),
				currentResolution(-1),
				resolutions(NULL)
{
  // do nothing
}

BzfDisplay::~BzfDisplay()
{
  for (int i = 0; i < numResolutions; i++)
    delete resolutions[i];
  delete[] resolutions;
}

int			BzfDisplay::getWidth() const
{
  if (currentResolution == -1) return 640;
  return resolutions[currentResolution]->width;
}

int			BzfDisplay::getHeight() const
{
  if (currentResolution == -1) return 480;
  return resolutions[currentResolution]->height;
}

void			BzfDisplay::setPassthroughSize(int w, int h)
{
  passWidth = w;
  passHeight = h;
}

int			BzfDisplay::getPassthroughWidth() const
{
  return passWidth;
}

int			BzfDisplay::getPassthroughHeight() const
{
  return passHeight;
}

int			BzfDisplay::getNumResolutions() const
{
  return numResolutions;
}

const BzfDisplay::ResInfo* BzfDisplay::getResolution(int index) const
{
  if (index < 0 || index >= numResolutions) return NULL;
  return resolutions[index];
}

int			BzfDisplay::getResolution() const
{
  return currentResolution;
}

int			BzfDisplay::getDefaultResolution() const
{
  return defaultResolution;
}

boolean			BzfDisplay::setResolution(int index)
{
  if (index < 0 || index >= numResolutions) return False;
  if (index == currentResolution) return True;
  if (!resolutions[index]) return False;
  if (!doSetResolution(index)) {
    delete resolutions[index];
    resolutions[index] = NULL;
    return False;
  }
  currentResolution = index;
  return True;
}

boolean			BzfDisplay::setDefaultResolution()
{
  const int oldResolution = currentResolution;
  currentResolution = -1;
  if (!doSetDefaultResolution()) {
    currentResolution = oldResolution;
    return False;
  }
  return True;
}

boolean			BzfDisplay::doSetDefaultResolution()
{
  if (numResolutions >= 2 && defaultResolution < numResolutions)
    return setResolution(defaultResolution);
  else
    return False;
}

int			BzfDisplay::findResolution(const char* name) const
{
  for (int i = 0; i < numResolutions; i++)
    if (strcmp(name, resolutions[i]->name) == 0)
      return i;
  return -1;
}

boolean			BzfDisplay::isValidResolution(int index) const
{
  if (index < 0 || index >= numResolutions) return False;
  return resolutions[index] != NULL;
}

void			BzfDisplay::initResolutions(ResInfo** _resolutions,
				int _numResolutions, int _currentResolution)
{
  resolutions = _resolutions;
  numResolutions = _numResolutions;
  currentResolution = _currentResolution;
  defaultResolution = currentResolution;
}
// ex: shiftwidth=2 tabstop=8
