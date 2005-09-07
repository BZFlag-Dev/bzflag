/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_WAVE
#define BZF_WAVE

#include <stdio.h>

/* A very simple wav file reader */

#define	WAV_FORMAT_UNKNOWN		(0x0000)
#define	WAV_FORMAT_PCM			(0x0001)
#define	WAV_FORMAT_ADPCM		(0x0002)
#define	WAV_FORMAT_ALAW			(0x0006)
#define	WAV_FORMAT_MULAW		(0x0007)
#define	WAV_FORMAT_OKI_ADPCM		(0x0010)
#define	WAV_FORMAT_DIGISTD		(0x0015)
#define	WAV_FORMAT_DIGIFIX		(0x0016)
#define	IBM_FORMAT_MULAW		(0x0101)
#define	IBM_FORMAT_ALAW			(0x0102)
#define	IBM_FORMAT_ADPCM		(0x0103)

/*
   Open the given filename as a wav file. Read the header and return the
   parameters in the rest of the arguments.
   Returns an open FILE if successful or NULL for error.
*/
FILE* openWavFile(const char *filename, short *format, long *speed,
		int *numFrames, short *numChannels, short *width);
/*
  Close the given wave file.
*/
void closeWavFile(FILE*);

/*
  Read the len bytes of wave file into the allocated data area.
  numSamples should be (*numFrames) * (*numChannels).
  Return 0 if successful or -1 for error.
*/
int readWavData(FILE*, char *data, int numSamples, int width);

#endif


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

