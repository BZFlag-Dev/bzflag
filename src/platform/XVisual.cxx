/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "XVisual.h"
#include <assert.h>
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

// NOTE:  to keep searching simple I cheat.  All bool attributes
//	are followed by GLX_USE_GL (which is a bool and ignored)
//	so that interesting attributes always fall on even indices.

void			XVisual::setLevel(int level)
{
  int index = findAttribute(GLX_LEVEL);
  if (index == -1) appendAttribute(GLX_LEVEL, level);
  else editAttribute(index, level);
}

void			XVisual::setDoubleBuffer(bool on)
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

void			XVisual::setStereo(bool on)
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
  (void)minSamples;  // quiet compiler if ifdef'd code not used
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

void			XVisual::appendAttribute(int attribute, int value)
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

bool			XVisual::build()
{
  if (!visual && getenv("MESA_RGB_VISUAL") == NULL) {
    // check each available visual looking for the best match.
    // we prefer deeper and dynamic (rather than static) visuals.

    // first get the list of all visuals by making a template to
    // match any visual on the screen and matching it.
    const long visualMask = VisualScreenMask;
    XVisualInfo visualTemplate;
    visualTemplate.screen = display->getScreen();
    int numVisuals;
    XVisualInfo* visualList = XGetVisualInfo(display->getDisplay(),
					visualMask,
					&visualTemplate,
					&numVisuals);
    if (numVisuals > 0 && visualList != NULL) {
      // no best visual so far
      int bestVisual = -1;
      int attrib;

      // now check each visual
      for (int i = 0; i < numVisuals; i++) {
	// ignore visuals that aren't deep enough
	if (visualList[i].depth < 8)
	  continue;

	// ignore visuals that glX can't use
	if (glXGetConfig(display->getDisplay(), visualList + i,
					GLX_USE_GL, &attrib) != 0 ||
					attrib == GL_FALSE)
	  continue;

	// ignore visuals that don't satisfy our requirements
	if (!matchRequirements(visualList + i))
	  continue;

	// use visual if it's better than the existing one.  some visual
	// is always better than none at all.
	if (bestVisual == -1) {
	  bestVisual = i;
	  continue;
	}

	// DirectColor is better than other visual classes, then
	// PseudoColor, then TrueColor, then StaticColor, then GreyScale.
	if (visualClassIsBetter(visualList[i].c_class,
				visualList[bestVisual].c_class)) {
	  bestVisual = i;
	  continue;
	}

	// if visual class wasn't better and isn't the same then it
	// must be worse.
	if (visualList[i].c_class != visualList[bestVisual].c_class)
	  continue;

	// deeper is better
	if (visualList[i].depth > visualList[bestVisual].depth) {
	  bestVisual = i;
	  continue;
	}

	// not better
      }

      // save best visual, if one was found
      if (bestVisual != -1) {
	visual = XGetVisualInfo(display->getDisplay(),
				VisualAllMask,
				visualList + bestVisual,
				&numVisuals);
	if (numVisuals == 0)
	  visual = NULL;
      }

      // done with visuals
      XFree(visualList);
    }
  }

  // emergency backup plan -- let glXChooseVisual choose for us
  if (!visual) {
    visual = glXChooseVisual(display->getDisplay(),
					display->getScreen(), attributes);
  }

  return visual != NULL;
}

bool			XVisual::matchRequirements(XVisualInfo* v) const
{
  // check RGBA, DOUBLEBUFFER, and STEREO
  int value;
  if (glXGetConfig(display->getDisplay(), v, GLX_RGBA, &value) != 0 ||
			(findAttribute(GLX_RGBA) != -1) != value)
    return false;
  if (glXGetConfig(display->getDisplay(), v, GLX_DOUBLEBUFFER, &value) != 0 ||
			(findAttribute(GLX_DOUBLEBUFFER) != -1) != value)
    return false;
  if (glXGetConfig(display->getDisplay(), v, GLX_STEREO, &value) != 0 ||
			(findAttribute(GLX_STEREO) != -1) != value)
    return false;

  // check the rest
  for (int i = 0; i < attributeCount; i += 2) {
    // get value of desired attribute from visual
    if (glXGetConfig(display->getDisplay(), v, attributes[i], &value) != 0)
      return false;

    // compare to desired value
    switch (attributes[i]) {
      case GLX_RGBA:
      case GLX_DOUBLEBUFFER:
      case GLX_STEREO:
	// skip these
	break;

      case GLX_LEVEL:
	if (value != attributes[i + 1])
	  return false;
	break;

      case GLX_BUFFER_SIZE:
      case GLX_RED_SIZE:
      case GLX_GREEN_SIZE:
      case GLX_BLUE_SIZE:
      case GLX_ALPHA_SIZE:
      case GLX_DEPTH_SIZE:
      case GLX_STENCIL_SIZE:
      case GLX_ACCUM_RED_SIZE:
      case GLX_ACCUM_GREEN_SIZE:
      case GLX_ACCUM_BLUE_SIZE:
      case GLX_ACCUM_ALPHA_SIZE:
#if defined(GLX_SAMPLES_SGIS) && defined(GLX_SGIS_multisample)
      case GLX_SAMPLES_SGIS:
#endif
	if (value < attributes[i + 1])
	  return false;
	break;

      default:
	assert(0 && "unexpected GLX attribute");
    }
  }

  return true;
}

bool			XVisual::visualClassIsBetter(int a, int b)
{
    // not better if the same
    if (a == b)
	return false;

    // direct color is best
    if (a == DirectColor)
	return true;
    if (b == DirectColor)
	return false;

    // then pseudo color (because we can adjust it)
    if (a == PseudoColor)
	return true;
    if (b == PseudoColor)
	return false;

    // then true color
    if (a == TrueColor)
	return true;
    if (b == TrueColor)
	return false;

    // then static color
    if (a == StaticColor)
	return true;
    if (b == StaticColor)
	return false;

    // then gray scale
    if (a == GrayScale)
	return true;

    return false;
}

XVisualInfo*		XVisual::get()
{
  if (!build()) return NULL;
  return visual;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

