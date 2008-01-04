/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_SGI_IMAGE_FILE_H
#define BZF_SGI_IMAGE_FILE_H

#include "ImageFile.h"


/** This class represents a SGI image file. It implements read() from
    ImageFile. */
class SGIImageFile : public ImageFile {
public:
  SGIImageFile(std::istream*);
  virtual ~SGIImageFile();

  /** This function returns the default extension of SGI image files. */
  static std::string	getExtension();

  /** This function reads image data from a SGI image file. */
  virtual bool		read(void* buffer);

protected:
  bool			readVerbatim(void* buffer);
  bool			readRLE(void* buffer);

private:
  bool			isVerbatim;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
