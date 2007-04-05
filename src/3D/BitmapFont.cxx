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

// Interface header
#include "BitmapFont.h"

BitmapFont::BitmapFont()
{
  g3d = csQueryRegistry<iGraphics3D>
    (csApplicationFramework::GetObjectRegistry());
  if (!g3d)
    csApplicationFramework::ReportError("Failed to locate 3D renderer!");
  if (g3d)
    font = g3d->GetDriver2D()->GetFontServer()->LoadFont(CSFONT_LARGE);
}

BitmapFont::~BitmapFont()
{
}

void BitmapFont::build(void)
{
}


void BitmapFont::free(void)
{
}

void BitmapFont::filter(bool /*dofilter*/)
{
}

void BitmapFont::drawString(int x, int y, GLfloat color[4], const char *str,
			    int len)
{
  if (!str)
    return;

  char myString[128];
  int  myLen = len;

  if (myLen > 127)
    myLen = 127;

  memcpy(myString, str, myLen);
  myString[myLen] = '\0';
  
  int fg = 0;
  if (color[0] >= 0)
    fg = g3d->GetDriver2D()->FindRGB(int(color[0] * 255.0f),
				     int(color[1] * 255.0f),
				     int(color[2] * 255.0f),
				     int(color[3] * 255.0f));

  g3d->GetDriver2D()->Write(font, x, y, fg, -1, myString);
}

void BitmapFont::drawString(float, GLfloat [], const char *, int)
{
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
