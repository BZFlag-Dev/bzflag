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

#include "XVisual.h"
#include <string.h>
#include <GL/glx.h>

XVisual::XVisual(const XDisplay* _display) :
				display(_display->getRep()),
				attributeCount(0),
				visual(NULL)
{
  display->ref();
  attributes[attributeCount] = None;
}

XVisual::~XVisual()
{
  if (visual) XFree(visual);
  display->unref();
}

// NOTE:  to keep searching simple I cheat.  All boolean attributes
//	are followed by GLX_USE_GL (which is a boolean and ignored)
//	so that interesting attributes always fall on even indices.

void			XVisual::setLevel(int level)
{
  int index = findAttribute(GLX_LEVEL);
  if (index == -1) appendAttribute(GLX_LEVEL, level);
  else editAttribute(index, level);
}

void			XVisual::setDoubleBuffer(boolean on)
{
  int index = findAttribute(GLX_DOUBLEBUFFER);
  if (!on) {
    if (index != -1) removeAttribute(index);
  }
  else {
    if (index == -1) appendAttribute(GLX_DOUBLEBUFFER, GLX_USE_GL);
  }
}

void			XVisual::setIndex(int minDepth)
{
  int index = findAttribute(GLX_RGBA);
  if (index != -1) removeAttribute(index);
  index = findAttribute(GLX_BUFFER_SIZE);
  if (index == -1) appendAttribute(GLX_BUFFER_SIZE, minDepth);
  else editAttribute(index, minDepth);
}

void			XVisual::setRGBA(int minRed, int minGreen,
						int minBlue, int minAlpha)
{
  int index = findAttribute(GLX_RGBA);
  if (index == -1) appendAttribute(GLX_RGBA, GLX_USE_GL);
  index = findAttribute(GLX_RED_SIZE);
  if (index == -1) appendAttribute(GLX_RED_SIZE, minRed);
  else editAttribute(index, minRed);
  index = findAttribute(GLX_GREEN_SIZE);
  if (index == -1) appendAttribute(GLX_GREEN_SIZE, minGreen);
  else editAttribute(index, minGreen);
  index = findAttribute(GLX_BLUE_SIZE);
  if (index == -1) appendAttribute(GLX_BLUE_SIZE, minBlue);
  else editAttribute(index, minBlue);
  index = findAttribute(GLX_ALPHA_SIZE);
  if (minAlpha == 0) {
    if (index != -1) removeAttribute(index);
  }
  else if (index == -1) appendAttribute(GLX_ALPHA_SIZE, minAlpha);
  else editAttribute(index, minAlpha);
}

void			XVisual::setDepth(int minDepth)
{
  int index = findAttribute(GLX_DEPTH_SIZE);
  if (minDepth == 0) {
    if (index != -1) removeAttribute(index);
  }
  else if (index == -1) appendAttribute(GLX_DEPTH_SIZE, minDepth);
  else editAttribute(index, minDepth);
}

void			XVisual::setStencil(int minDepth)

{
  int index = findAttribute(GLX_STENCIL_SIZE);
  if (minDepth == 0) {
    if (index != -1) removeAttribute(index);
  }
  else if (index == -1) appendAttribute(GLX_STENCIL_SIZE, minDepth);
  else editAttribute(index, minDepth);
}

void			XVisual::setAccum(int minRed, int minGreen,
						int minBlue, int minAlpha)
{
  int index = findAttribute(GLX_ACCUM_RED_SIZE);
  if (index == -1) appendAttribute(GLX_ACCUM_RED_SIZE, minRed);
  else editAttribute(index, minRed);
  index = findAttribute(GLX_ACCUM_GREEN_SIZE);
  if (index == -1) appendAttribute(GLX_ACCUM_GREEN_SIZE, minGreen);
  else editAttribute(index, minGreen);
  index = findAttribute(GLX_ACCUM_BLUE_SIZE);
  if (index == -1) appendAttribute(GLX_ACCUM_BLUE_SIZE, minBlue);
  else editAttribute(index, minBlue);
  index = findAttribute(GLX_ACCUM_ALPHA_SIZE);
  if (index == -1) appendAttribute(GLX_ACCUM_ALPHA_SIZE, minAlpha);
  else editAttribute(index, minAlpha);
}

void			XVisual::setStereo(boolean on)
{
  int index = findAttribute(GLX_STEREO);
  if (!on) {
    if (index != -1) removeAttribute(index);
  }
  else {
    if (index == -1) appendAttribute(GLX_STEREO, GLX_USE_GL);
  }
}

void			XVisual::setMultisample(int minSamples)
{
#if defined(GLX_SAMPLES_SGIS) && defined(GLX_SGIS_multisample)
  int index = findAttribute(GLX_SAMPLES_SGIS);
  if (index == -1) appendAttribute(GLX_SAMPLES_SGIS, minSamples);
  else editAttribute(index, minSamples);
#endif
}

int			XVisual::findAttribute(int attribute) const
{
  for (int i = 0; i < attributeCount; i += 2)
    if (attributes[i] == attribute)
      return i;
  return -1;
}

void			XVisual::appendAttribute(int attribute,
								int value)
{
  attributes[attributeCount] = attribute;
  attributes[attributeCount+1] = value;
  attributeCount += 2;
  attributes[attributeCount] = None;
}

void			XVisual::removeAttribute(int index)
{
  attributeCount -= 2;
  attributes[index] = attributes[attributeCount];
  attributes[index+1] = attributes[attributeCount+1];
  attributes[attributeCount] = None;
}

void			XVisual::editAttribute(int index, int value)
{
  attributes[index+1] = value;
}

boolean			XVisual::build()
{
  if (!visual) visual = glXChooseVisual(display->getDisplay(),
					display->getScreen(), attributes);
  return visual != NULL;
}

XVisualInfo*		XVisual::get()
{
  if (!build()) return NULL;
  return visual;
}
