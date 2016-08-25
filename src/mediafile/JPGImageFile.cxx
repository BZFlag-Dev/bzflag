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

#include "common.h"
#include "JPGImageFile.h"
#include <iostream>
#include <string.h>
#include "Pack.h"
#include "bzfio.h"
#include <zconf.h>
#include <zlib.h>

#ifndef _WIN32
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif


const static size_t JPEG_BUF_SIZE = 16384;

struct JpegStream {
  struct jpeg_source_mgr pub;
  std::istream* is;
  JOCTET*       buffer;
};

static void my_init_source(j_decompress_ptr)
{
}

static boolean my_fill_input_buffer(j_decompress_ptr cinfo) {
  JpegStream* src = (JpegStream *)cinfo->src;

  src->is->read((char*)src->buffer, JPEG_BUF_SIZE);
  size_t bytes = src->is->gcount();
  if (bytes == 0) {
    /* Insert a fake EOI marker */
    src->buffer[0] = (JOCTET) 0xFF;
    src->buffer[1] = (JOCTET) JPEG_EOI;
    bytes = 2;
  }
  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = bytes;
  return TRUE;
}

static void my_skip_input_data(j_decompress_ptr cinfo, long num_bytes) {
  JpegStream *src = (JpegStream *)cinfo->src;
  if (num_bytes > 0) {
    while (num_bytes > (long)src->pub.bytes_in_buffer) {
      num_bytes -= (long)src->pub.bytes_in_buffer;
      my_fill_input_buffer(cinfo);
    }
    src->pub.next_input_byte += num_bytes;
    src->pub.bytes_in_buffer -= num_bytes;
  }
}

static void my_term_source(j_decompress_ptr cinfo) {
  // must seek backward so that future reads will start at correct place.
  JpegStream *src = (JpegStream *)cinfo->src;
  src->is->clear();
  src->is->seekg(src->is->tellg() - (std::streampos)src->pub.bytes_in_buffer);
}

/*
JPGImageFile::PNGImageFile(std::istream* stream)

  validates that the file is in fact a jpg file and initializes the size information for it
*/

JPGImageFile::JPGImageFile(std::istream* input) : ImageFile(input)
{
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  JpegStream *src = (JpegStream *)(*cinfo.mem->alloc_small)
    ((j_common_ptr) &cinfo, JPOOL_PERMANENT, sizeof(JpegStream));

  cinfo.src = (jpeg_source_mgr *)src;

  src->buffer = (JOCTET *)(*cinfo.mem->alloc_small)
    ((j_common_ptr) &cinfo, JPOOL_PERMANENT, JPEG_BUF_SIZE * sizeof(JOCTET));

  src->is = input;
  src->pub.init_source = my_init_source;
  src->pub.fill_input_buffer = my_fill_input_buffer;
  src->pub.skip_input_data = my_skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = my_term_source;
  src->pub.bytes_in_buffer = 0;
  src->pub.next_input_byte = 0;

  // read info from header.
  int r = jpeg_read_header(&cinfo, TRUE);
  if (r != JPEG_HEADER_OK)
    return;
  jpeg_start_decompress(&cinfo);
  init(cinfo.output_components, cinfo.output_width, cinfo.output_height);
}

/*
JPGImageFile::~JPGImageFile()

  cleans up memory buffers
*/

JPGImageFile::~JPGImageFile()
{
  jpeg_destroy_decompress(&cinfo);
}

/*
std::string	JPGImageFile::getExtension()

  returns the expected file extension of .jpg for files
*/

std::string				JPGImageFile::getExtension()
{
  return ".jpg";
}

bool                                    JPGImageFile::read(void *buffer)
{
  JSAMPROW row = (JSAMPROW)buffer;
  JDIMENSION size;
  const int lineDim = cinfo.output_width * cinfo.output_components;

  while (cinfo.output_scanline < cinfo.output_height) {
    size = jpeg_read_scanlines(&cinfo, &row, 1);
    row += size * lineDim;
  }
  jpeg_finish_decompress(&cinfo);    
  return true;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
