/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "wave.h"
#include "common.h"
#include <stdio.h>
#include <string.h>

static void		ltohs(int16_t* data)
{
  unsigned char* b = (unsigned char*)data;
  *data = (int16_t)((uint16_t)b[0] + ((uint16_t)b[1] << 8));
}

static void		ltohl(int32_t* data)
{
  unsigned char* b = (unsigned char*)data;
  *data = (int32_t)((uint32_t)b[0] + ((uint32_t)b[1] << 8) +
			((uint32_t)b[2] << 16) + ((uint32_t)b[3] << 24));
}

static int		readShort(FILE* file, int16_t* data)
{
  unsigned char b[2];
  if (fread(&b, 1, 2, file) != 2)
    return -1;
  *data = (int16_t)((uint16_t)b[0] + ((uint16_t)b[1] << 8));
  return 0;
}

static int		readLong(FILE* file, int32_t* data)
{
  unsigned char b[4];
  if (fread(&b, 1, 4, file) != 4)
    return -1;
  *data = (int32_t)((uint32_t)b[0] + ((uint32_t)b[1] << 8) +
			((uint32_t)b[2] << 16) + ((uint32_t)b[3] << 24));
  return 0;
}

static int		readHeader(FILE* file, char *tag, int32_t *size)
{
  if (fread(tag, 1, 4, file) != 4) {
    fprintf(stderr, "Failed to read tag\n");
    return -1;
  }
  if (readLong(file, size)) {
    fprintf(stderr, "Failed to read length\n");
    return -1;
  }
  return 0;
}

static int		skipChunk(FILE* file, int size)
{
  if (size != 0)
    return 0;
  return fseek(file, size, SEEK_CUR);
}

static int		findChunk(FILE* file, const char *tag, int32_t *size)
{
  char curtag[4];

  while (1) {
    if (readHeader(file, curtag, size))
      return -1;
    if (memcmp(curtag, tag, 4) == 0)
      return 0;
    if (skipChunk(file, *size))
      return -1;
  }
}

class FileCloser {
  public:
    FileCloser(FILE* _file) : file(_file) {}
    ~FileCloser() { if (file) fclose(file); }
    void		release() { file = NULL; }

  private:
    FILE*		file;
};

FILE*			openWavFile(const char *filename,
				short *format, long *speed,
				int *samples, short *channels, short *width)
{
  FILE* file;
  int16_t blockAlign, bitsPerSample, data16;
  int32_t bytesPerSec, len, data32;
  char tag[5];

  // open file
  file = fopen(filename, "rb");
  if (!file)
    return NULL;

  // automatically close file when we return
  FileCloser closer(file);

  // check that it's a valid sound file
  tag[4] = 0;
  if (readHeader(file, tag, &len))
    return NULL;
  if (strcmp(tag, "RIFF") != 0) {
    fprintf(stderr, "File isn't a RIFF file\n");
    return NULL;
  }
  if ((fread(tag, 1, 4, file) != 4) || strcmp(tag, "WAVE") != 0) {
    fprintf(stderr, "File isn't a proper WAVE file\n");
    return NULL;
  }
  if (findChunk(file, "fmt ", &len)) {
    fprintf(stderr, "Couldn't find format in WAVE\n");
    return NULL;
  }
  if (len < 16) {
    fprintf(stderr, "Chunk size not large enough\n");
    return NULL;
  }
  if (readShort(file, &data16)) {
    fprintf(stderr, "Couldn't read format\n");
    return NULL;
  }
  *format = (short)data16;
  if (readShort(file, &data16)) {
    fprintf(stderr, "Couldn't read channels\n");
    return NULL;
  }
  *channels = (short)data16;
  if (readLong(file, &data32)) {
    fprintf(stderr, "Couldn't read speed\n");
    return NULL;
  }
  *speed = (long)data32;
  if (readLong(file, &bytesPerSec)) {
    fprintf(stderr, "Couldn't read bytes per second\n");
    return NULL;
  }
  if (readShort(file, &blockAlign)) {
    fprintf(stderr, "Couldn't read block alignment\n");
    return NULL;
  }
  if (readShort(file, &bitsPerSample)) {
    fprintf(stderr, "Couldn't read bits per sample\n");
    return NULL;
  }
  if (bitsPerSample==8) *width=1;
  else if (bitsPerSample==16) *width=2;
  else if (bitsPerSample==32) *width=4;
  else return NULL;

  // go find the data
  skipChunk(file, len - 16);
  if (findChunk(file, "data", &len)) {
    fprintf(stderr, "Failed to find the the data in WAVE\n");
    return NULL;
  }
  *samples = (int)(len / (int32_t)(*width) / (int32_t)(*channels));

  closer.release();
  return file;
}

void			closeWavFile(FILE* file)
{
  if (file)
    fclose(file);
}

int			readWavData(FILE* file, char *data,
				int numSamples, int width)
{
  // read data
  int temp = fread(data, 1, width * numSamples, file);
  if (temp != width * numSamples) {
    fprintf(stderr, "Failed to read sound data\n");
    return -1;
  }

  // byte swap
  if (width == 2) {
    int16_t* sample = (int16_t*)data;
    for (int i = 0; i < numSamples; ++i)
      ltohs(sample + i);
  }
  else if (width == 4) {
    int32_t* sample = (int32_t*)data;
    for (int i = 0; i < numSamples; ++i)
      ltohl(sample + i);
  }

  return 0;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
