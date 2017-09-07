/* bzflag
 * Copyright (c) 1993-2017 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"
#include "PNGImageFile.h"
#include <iostream>
#include <string.h>
#include "bzfio.h"
#include <png.h>

//
// PNGImageFile
//

/*
PNGImageFile::PNGImageFile(std::istream* stream)

  validates that the file is in fact a png file and initializes the size information for it
*/

PNGImageFile::PNGImageFile(std::istream* input, std::string filename) : ImageFile(input)
{
  logDebugMessage(4, "PNGImageFile starting to read %s\n", filename.c_str());

  // Try to read the signature
  png_byte signature[8];
  input->read((char*)signature, 8);

  if (!input->good()) {
    return;
  }

  // Verify that the signature is correct
  if (png_sig_cmp(signature, 0, 8) != 0) {
    return;
  }

  // Create the read structure
  png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) {
    return;
  }

  // Create the info structure
  pnginfo = png_create_info_struct(png);
  if (!pnginfo) {
    png_destroy_read_struct(&png, (png_infopp)0, (png_infopp)0);
    return;
  }

  rowPtrs = NULL;

  if (setjmp(png_jmpbuf(png))) {
    png_destroy_read_struct(&png, &pnginfo, (png_infopp)0);
    if (rowPtrs != NULL)
      delete[] (png_bytep)rowPtrs;
    return;
  }

  png_set_read_fn(png, input,
    [](png_structp _png, png_bytep _data, png_size_t _length) -> void {
      std::istream* _input = (std::istream*)(png_get_io_ptr(_png));
      try {
	_input->read((char*)_data, _length);
      }
      catch (std::ios_base::failure e) {
	logDebugMessage(0, e.what());
	png_error(_png, "error");
      }
    }
  );

  // Tell libpng that we've already read 8 bytes
  png_set_sig_bytes(png, 8);

  // Read the rest of the headers
  png_read_info(png, pnginfo);

  // Gather some information about the file
  bitDepth = png_get_bit_depth(png, pnginfo);
  colorType = png_get_color_type(png, pnginfo);
  compressionMethod = png_get_compression_type(png, pnginfo);
  filterMethod = png_get_filter_type(png, pnginfo);
  interlaceMethod = png_get_interlace_type(png, pnginfo);
  png_uint_32 myWidth, myHeight;
  myWidth = png_get_image_width(png, pnginfo);
  myHeight = png_get_image_height(png, pnginfo);
  png_uint_32 channels = png_get_channels(png, pnginfo);

  if (filterMethod != PNG_FILTER_TYPE_BASE || interlaceMethod != PNG_INTERLACE_NONE)
    return;

  // If we have a palleted image, convert to RGB
  if (colorType == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png);
    channels = 3;
  }

  // If we have a grayscale PNG with a bit depth of less than 8 bits per channel, convert to 8 bits
  if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8) {
    png_set_expand_gray_1_2_4_to_8(png);
    bitDepth = 8;
  }

  // If we have transparency information in a tRNS chunk, convert it to an alpha channel
  if (png_get_valid(png, pnginfo, PNG_INFO_tRNS)) {
    png_set_tRNS_to_alpha(png);
    channels += 1;
  }

  // Convert 16 bits per channel PNGs to 8 bits
  if (bitDepth == 16) {
    png_set_strip_16(png);
    bitDepth = 8;
  }

  // If after the above, the bit depth is not 8, bail.
  if (bitDepth != 8)
    return;

  // Read the updated information structs
  png_read_update_info(png, pnginfo);

  init(channels, myWidth, myHeight);

  logDebugMessage(4,"PNGImageFile finishing reading headers for %s: Width %d, Height %d, Bit depth %d, Color type %d, Filter Method %d, "
	 "Interlace Method %d, Channels %d.\n",
	 filename.c_str(), myWidth, myHeight, bitDepth, colorType, filterMethod, interlaceMethod, channels);

}

/*
PNGImageFile::~PNGImageFile()

  cleans up memory buffers
*/

PNGImageFile::~PNGImageFile()
{
  if (png != NULL) {
    if (pnginfo != NULL) {
      png_destroy_read_struct(&png, &pnginfo, (png_infopp)0);
    } else {
      png_destroy_read_struct(&png, (png_infopp)0, (png_infopp)0);
    }
  }

  if (rowPtrs != NULL)
    delete[] (png_bytep)rowPtrs;
}

/*
std::string	PNGImageFile::getExtension()

  returns the expected file extension of .png for files
*/

std::string				PNGImageFile::getExtension()
{
  return ".png";
}

/*
bool PNGImageFile::read(void* buffer)

  parses the file looking for PLTE or IDAT entries and converts to 8 bit data
  in 1, 2, 3, or 4 channels
*/

bool					PNGImageFile::read(void* buffer)
{
  // Get attributes
  unsigned int myHeight = getHeight();
  unsigned int myWidth = getWidth();
  unsigned int channels = getNumChannels();
  // Create an array of row pointers
  rowPtrs = new png_bytep[myHeight];

  const unsigned int stride = myWidth * bitDepth * channels / 8;

  for (size_t i = 0; i < myHeight; i++) {
    png_uint_32 q = (myHeight - i - 1) * stride;
    rowPtrs[i] = (png_bytep)buffer + q;
  }

  png_read_image(png, rowPtrs);

  return true;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
