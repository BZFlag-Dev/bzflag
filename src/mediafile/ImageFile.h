/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_IMAGE_FILE_H
#define BZF_IMAGE_FILE_H

#include "MediaFile.h"

/** This ABC represents an image file. It has subclasses for different image
    formats. */
class ImageFile : public MediaFile {
public:
  /** Close the image file.  This does *not* destroy the stream. */
  virtual ~ImageFile();

  // note -- all concrete ImageFile types should have a method to
  // return the default file extension for files in the format:
  // static std::string getExtension();

  /** Pixels are stored I, IA, RGB, or RGBA, depending on the number
      of channels.  Rows are stored left to right, bottom to top.
      Buffer must be at least getNumChannels() * getWidth() * getHeight()
      bytes. */
  virtual bool read(void* buffer) = 0;

  /** Returns true if the stream was successfully opened as an image file. */
  bool isOpen() const;

  /** Get the number of channels in the image file. Channels are 8 bits. */
  int			getNumChannels() const;

  /** Get the width of the image file. */
  int			getWidth() const;

  /** Get the height of the image file. */
  int			getHeight() const;

protected:
  ImageFile(std::istream*);

  /** Save info about the stream.  Called by the derived c'tor. */
  void init(int numChannels, int width, int height);

private:
  bool			open;
  int			numChannels;
  int			width, height;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
