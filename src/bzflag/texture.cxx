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

#include <stdio.h>
#include <stdlib.h>
#include "texture.h"
#include "ErrorHandler.h"
#include "PlatformFactory.h"
#include "BzfMedia.h"

void			printFatalError(const char* fmt, ...);

unsigned char*		getTextureImage(const BzfString& file,
				int& width, int& height, int& depth)
{
  if (file.isNull()) return NULL;
  printError("loading %s...", (const char*)file);
  return PlatformFactory::getMedia()->readImage(file, width, height, depth);
}

unsigned char*		getTextImage(const BzfString& file,
				int& width, int& height)
{
  if (file.isNull()) return NULL;
  printError("loading %s...", (const char*)file);
  int depth;
  unsigned char* image = PlatformFactory::getMedia()->
			readImage(file, width, height, depth);
  return image;
}

OpenGLTexture		getTexture(const BzfString& file,
				int* _width, int* _height,
				OpenGLTexture::Filter filter,
				boolean repeat,
				boolean noError)
{
  if (file.isNull()) return OpenGLTexture();

  int width, height, depth;
  unsigned char* image = getTextureImage(file, width, height, depth);
  if (!image) {
    if (!noError) printError("cannot load texture: %s", (const char*)file);
    return OpenGLTexture();
  }

  if (_width) *_width = width;
  if (_height) *_height = height;
  OpenGLTexture tex(width, height, image, filter, repeat);
  delete[] image;

  return tex;
}

OpenGLTexture		getTexture(const BzfString& file,
				OpenGLTexture::Filter filter,
				boolean repeat,
				boolean noError)
{
  return getTexture(file, NULL, NULL, filter, repeat, noError);
}

void			printMissingDataDirectoryError(const char* msg)
{
  char buffer[4096];
  sprintf(buffer, "%s\nFailed to load resource because the 'data'\n"
		"directory can't be found.  Please run bzflag\n"
		"with the data directory as the current directory\n"
		"or provide the location of the data directory\n"
		"using the -directory command line option.", msg);
  printFatalError(buffer);
}

//
// TextureFont
//

static boolean		anyFontLoaded = False;
const char*		TextureFont::fontFileName[] = {
				"timesbr",
				"timesbi",
				"helvbr",
				"helvbi",
				"fixedmr",
				"fixedbr",
			};
OpenGLTexFont*		TextureFont::font[sizeof(TextureFont::fontFileName) /
					  sizeof(TextureFont::fontFileName[0])];

OpenGLTexFont		TextureFont::getTextureFont(Font index, boolean required)
{
  static boolean init = False;
  if (!init) {
    init = True;
    for (int i = 0; i < (int)(sizeof(fontFileName) /
					sizeof(fontFileName[0])); i++)
      font[i] = NULL;
  }

  if (!font[index]) {
    int width, height;
    unsigned char* image = getTextImage(fontFileName[index], width, height);
    if (image) {
      font[index] = new OpenGLTexFont(width, height, image);
      delete[] image;
      anyFontLoaded = True;
    }
    else if (required) {
      // can't print message usual way because we're going down
      char msg[256];
      sprintf(msg, "Can't continue without font: %s", fontFileName[index]);
      if (!anyFontLoaded) {
	// we haven't loaded any fonts yet so assume we can't find the
	// data directory.  print a message explaining the problem and
	// maybe i'll stop getting at least an email a week asking
	// about this.
	printMissingDataDirectoryError(msg);
      }
      else {
	printFatalError(msg);
      }
      exit(1);
    }
  }

  return font[index] ? *(font[index]) : OpenGLTexFont();
}
// ex: shiftwidth=2 tabstop=8
