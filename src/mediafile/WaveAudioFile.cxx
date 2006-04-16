/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "WaveAudioFile.h"
#include <string.h>

#define WAV_FORMAT_UNKNOWN	(0x0000)
#define WAV_FORMAT_PCM		(0x0001)
#define WAV_FORMAT_ADPCM	(0x0002)
#define WAV_FORMAT_ALAW		(0x0006)
#define WAV_FORMAT_MULAW	(0x0007)
#define WAV_FORMAT_OKI_ADPCM	(0x0010)
#define WAV_FORMAT_DIGISTD	(0x0015)
#define WAV_FORMAT_DIGIFIX	(0x0016)
#define IBM_FORMAT_MULAW	(0x0101)
#define IBM_FORMAT_ALAW		(0x0102)
#define IBM_FORMAT_ADPCM	(0x0103)

WaveAudioFile::WaveAudioFile( std::istream *input ): AudioFile( input )
{
	if( input == NULL )
		return ;

	char tag[4];
	uint32_t length;
	if( !readHeader( tag, &length ) || memcmp( tag, "RIFF", 4 ) != 0 )
	{
		// not a RIFF file
		return ;
	}

	readRaw( tag, 4 );
	if( !isOkay() || memcmp( tag, "WAVE", 4 ) != 0 )
	{
		// not a WAVE file
		return ;
	}

	if( !findChunk( "fmt ", &length ))
	{
		// couldn't find format
		return ;
	}
	if( length < 16 )
	{
		// chunk size not large enough
		return ;
	}

	uint16_t data16 = read16LE();
	if( !isOkay() || data16 != WAV_FORMAT_PCM )
	{
		// can't read format or format not PCM
		return ;
	}

	data16 = read16LE();
	if( !isOkay() || ( data16 != 1 && data16 != 2 ))
	{
		// can't read number of channels or unsupported number of channels
		return ;
	}
	const int _numChannels = static_cast < int > ( data16 );

	// frames per second
	uint32_t data32 = read32LE();
	if( !isOkay())
	{
		return ;
	}
	int _framesPerSecond = static_cast < int > ( data32 );

	// bytes per second
	read32LE();

	// block alignment
	read16LE();

	// bits per sample
	data16 = read16LE();
	if( !isOkay() || ( data16 != 8 && data16 != 16 && data16 != 32 ))
	{
		return ;
	}
	const int _sampWidth = static_cast < int > ( data16 / 8 );

	// go find the data
	skip( length - 16 );
	if( !findChunk( "data", &length ))
	{
		// can't find data
		return ;
	}

	// compute number of frames
	int _numFrames = ( length / ( _sampWidth *_numChannels ));

	// save info
	init( _framesPerSecond, _numChannels, _numFrames, _sampWidth );
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

WaveAudioFile::~WaveAudioFile()
{
	// do nothing
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

std::string WaveAudioFile::getExtension()
{
	return ".wav";
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

bool WaveAudioFile::read( void *buffer, int _numFrames )
{
	// read data
	const int width = getSampleWidth();
	const int numSamples = _numFrames * getNumChannels();
	readRaw( buffer, width *numSamples );
	if( !isOkay())
	{
		// failed to read data
		return false;
	}

	// byte swap
	switch( width )
	{
		case 1:
			// no swapping if only one byte per sample
			break;

		case 2:
			{
				uint16_t *samples = reinterpret_cast < uint16_t * > ( buffer );
				for( int i = 0; i < numSamples; ++i )
					swap16LE( samples + i );
				break;
			}

		case 4:
			{
				uint32_t *samples = reinterpret_cast < uint32_t * > ( buffer );
				for( int i = 0; i < numSamples; ++i )
					swap32LE( samples + i );
				break;
			}
	}

	return true;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

bool WaveAudioFile::readHeader( char *tag, uint32_t *length )
{
	readRaw( tag, 4 );
	*length = read32LE();
	return isOkay();
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

bool WaveAudioFile::findChunk( const char *tag, uint32_t *length )
{
	while( isOkay())
	{
		char curtag[4];
		if( !readHeader( curtag, length ))
			return false;
		if( memcmp( curtag, tag, 4 ) == 0 )
			return true;
		skip( *length );
		readHeader( curtag, length );
	}
	return false;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
