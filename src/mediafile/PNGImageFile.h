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

#ifndef BZF_PNG_IMAGE_FILE_H
#define BZF_PNG_IMAGE_FILE_H

#include "ImageFile.h"

class PNGPalette;
class PNGChunk;


/** This class represents a PNG image file. It implements the read() function
    from ImageFile. */
class PNGImageFile : public ImageFile {
public:
  PNGImageFile(std::istream*);
  virtual ~PNGImageFile();

  /** This function returns the default extension of PNG image files. */
  static std::string	getExtension();

  /** Read image data from a PNG file. */
  virtual bool		read(void* buffer);
private:
  PNGPalette* readPalette(PNGChunk *c);
  unsigned char *getLineBuffer(bool active=true);
  void switchLineBuffers();
  bool filter();
  bool expand(unsigned char * destination);

  static unsigned char			PNGHEADER[8];
  static const unsigned char		MAX_COMPONENTS;
  static const unsigned char		FILTER_NONE;
  static const unsigned char		FILTER_SUB;
  static const unsigned char		FILTER_UP;
  static const unsigned char		FILTER_AVERAGE;
  static const unsigned char		FILTER_PAETH;

  PNGPalette*						palette;
  unsigned char					bitDepth;
  unsigned char					colorDepth;
  unsigned char					compressionMethod;
  unsigned char					filterMethod;
  unsigned char					interlaceMethod;
  unsigned char					*lineBuffers[2];
  int								activeBufferIndex;
  int								lineBufferSize;
  int								realBufferSize;
};

/** This class represents a RGB color value used by the PNG reader. */
class PNGRGB
{
 public:
  PNGRGB();
  PNGRGB(unsigned char r, unsigned char g, unsigned char b);
  unsigned char red;
  unsigned char green;
  unsigned char blue;
};

/** This class represents a PNG color palette and is used by the PNG reader.
    It contains an indexed list of PNGRGB objects. */
class PNGPalette
{
 public:
  PNGPalette( int numColors );
  ~PNGPalette();
  void add( PNGRGB &color );
  PNGRGB& get(int index);
 private:
  int		curColor;
  int		numColors;
  PNGRGB	*colors;
};

/** This class represents a chunk of PNG data. It is used by the PNG reader. */
class PNGChunk
{
 public:
  static PNGChunk *readChunk(std::istream *stream);
  ~PNGChunk();
  int getLength();
  int getType();
  unsigned char* getData();

  static int		IHDR;
  static int		PLTE;
  static int		IDAT;
  static int		IEND;
 private:
  PNGChunk();
  int			length;
  int			type;
  unsigned char		*data;
  int			crc;

};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
