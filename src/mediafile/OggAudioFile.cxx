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

#include <iostream>
#include <fstream>
#include "OggAudioFile.h"

OggAudioFile::OggAudioFile(std::istream* in) : AudioFile(in)
{
  stream = -1;

  ov_callbacks cb;
  cb.read_func = OAFRead;
  cb.seek_func = OAFSeek;
  cb.close_func = OAFClose;
  cb.tell_func = OAFTell;

  OAFInputBundle* bundle = new OAFInputBundle;
  in->seekg(0, std::ios::end);
  bundle->input = in; bundle->length = std::streamoff(in->tellg());
  in->seekg(0, std::ios::beg);

  if(ov_open_callbacks(bundle, &file, NULL, 0, cb) < 0) {
    std::cout << "OggAudioFile() failed: call to ov_open_callbacks failed\n";
  } else {
    info = ov_info(&file, -1);
    int samples = ov_pcm_total(&file, -1);
    init(info->rate, info->channels, samples, 2);
  }
}

OggAudioFile::~OggAudioFile()
{
  ov_clear(&file);
}

std::string	OggAudioFile::getExtension()
{
  return ".ogg";
}

bool		OggAudioFile::read(void* buffer, int frameCount)
{
  int result;
  long bytestotal = frameCount * info->channels * 2;
  long filelength = bytestotal;
  while (bytestotal > 0) {
    long oldoff = ov_pcm_tell(&file) * info->channels * 2;
    long bytesleft = filelength - oldoff;
#ifdef WIN32
    result = ov_read(&file, ((char*) buffer) + oldoff, bytesleft, 0, 2, 1, &stream);
#else
#if BYTE_ORDER == BIG_ENDIAN
    result = ov_read(&file, ((char*) buffer) + oldoff, bytesleft, 1, 2, 1, &stream);
#else
    result = ov_read(&file, ((char*) buffer) + oldoff, bytesleft, 0, 2, 1, &stream);
#endif
#endif
    long newoff = ov_pcm_tell(&file) * info->channels * 2;
    long bytesread = newoff - oldoff;
    if (result == OV_EBADLINK)
      return false;
    else if (result == 0)
      return false;
    bytestotal -= bytesread;
  }
  return true;
}

size_t	OAFRead(void* ptr, size_t size, size_t nmemb, void* datasource)
{
  OAFInputBundle* bundle = (OAFInputBundle*) datasource;
  std::streamoff pos1 = std::streamoff(bundle->input->tellg());
  std::streamsize read = size * nmemb;

  if (pos1 + read > bundle->length) {
    read = bundle->length - pos1;
  }
  if (pos1 == bundle->length)
    // EOF
    return 0;
  bundle->input->read((char*) ptr, read);

  return read;
}

int		OAFSeek(void* datasource, ogg_int64_t offset, int whence)
{
  OAFInputBundle* bundle = (OAFInputBundle*) datasource;
  switch (whence) {
    case SEEK_SET:
      bundle->input->seekg(offset, std::ios::beg);
      break;
    case SEEK_CUR:
      bundle->input->seekg(offset, std::ios::cur);
      break;
    case SEEK_END:
      bundle->input->seekg(offset, std::ios::end);
      break;
  }
  return 0;
}

int		OAFClose(void* datasource)
{
  // technically we should close here, but this is handled outside
  OAFInputBundle* bundle = (OAFInputBundle*) datasource;
  delete bundle;
  return 0;
}

long		OAFTell(void* datasource)
{
  OAFInputBundle* bundle = (OAFInputBundle*) datasource;
  return bundle->input->tellg();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
