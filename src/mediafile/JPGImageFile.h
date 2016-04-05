/* bzflag
 * Copyright (c) 1993-2016 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_JPG_IMAGE_FILE_H
#define BZF_JPG_IMAGE_FILE_H

#include "ImageFile.h"

#include <jpeglib.h>

/** This class represents a JPG image file. It implements the read() function
    from ImageFile. */
class JPGImageFile : public ImageFile {
public:
  JPGImageFile(std::istream*);
  virtual ~JPGImageFile();

  /** This function returns the default extension of JPG image files. */
  static std::string	getExtension();

  /** Read image data from a PNG file. */
  virtual bool		read(void* buffer);
private:
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
};
#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
