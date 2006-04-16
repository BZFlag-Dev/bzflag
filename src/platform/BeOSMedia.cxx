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

#include "BeOSMedia.h"
#include "BzfMedia.h"
#include "ErrorHandler.h"
#include <MediaDefs.h>
#include <math.h>
#include <fcntl.h>
#include <endian.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <OS.h>

//#define HALF_RATE_AUDIO
#define NO_FLOAT

//#define DEBUG_TO_WAV
//#define DEBUG_TO_WAV2

//#define DBG(a) printf a
#define DBG(a)

#ifdef HALF_RATE_AUDIO
static const int defaultAudioRate = 11025;
#else 
static const int defaultAudioRate = 22050;
#endif 

#ifdef NO_FLOAT
	#define SAMPTYPE uint16
#else 
	#define SAMPTYPE float
#endif 

//#define USE_PIPE

#ifdef USE_PIPE
int p[2];
#endif 

#ifdef DEBUG_TO_WAV
// debug fd
FILE *debugWav = 0;
#endif 
#ifdef DEBUG_TO_WAV2
FILE *debugWav2 = 0;
#endif 

#define MYASSERT(con) if (!(con)) debugger("ASSERT: "#con);

#define AUDIO_BLOCK_FRAME_COUNT 1024
#define AUDIO_BLOCK_SIZE (AUDIO_BLOCK_FRAME_COUNT * sizeof(SAMPTYPE))
#define AUDIO_BLOCK_COUNT 4

#define AUDIO_BUFFER_SIZE (AUDIO_BLOCK_SIZE*AUDIO_BLOCK_COUNT)

BeOSMedia::BeOSMedia(): BzfMedia(), audioReady( false ), audioBufferSize( AUDIO_BUFFER_SIZE ), audioLowWaterMark( AUDIO_BUFFER_SIZE / 3 ), audioQueuePort(  - 1 ), audioQueueMaxCmds( 10 ), outputBuffer( NULL ), childThreadID( 0 ), soundPlayer( NULL ), audioHasQuit( false ), checkLowWater( false ), lowWaterSem(  - 1 ), audioInputSem(  - 1 ), audioInputIndex( 0 ), audioOutputSem(  - 1 ), audioOutputIndex( 0 ), stopWatchStart( 0 )
{
	// do nothing
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

BeOSMedia::~BeOSMedia()
{
	// do nothing
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

bool BeOSMedia::openAudio()
{
	// don't re-initialize
	if( audioReady )
		return false;

#ifdef DEBUG_TO_WAV
	debugWav = fopen( "/boot/home/bzflagdbgwav.dat", "w" /*O_WRONLY|O_CREAT*/ );
#endif 
#ifdef DEBUG_TO_WAV2
	debugWav2 = fopen( "/boot/home/bzflagdbgwav2.dat", "w" /*O_WRONLY|O_CREAT*/ );
#endif 
#ifdef USE_PIPE
	pipe( p );
#endif 

	audioQueuePort = create_port( audioQueueMaxCmds, "bzflag_audio_cmd_port" );
	if( audioQueuePort < B_OK )
		return false;

	lowWaterSem = create_sem( 0, "bzflag_watermark" );
	if( lowWaterSem < B_OK )
	{
		closeAudio();
		return false;
	}

	media_raw_audio_format format;

	audioInputSem = create_sem( AUDIO_BUFFER_SIZE, "bzflag_ringbuffer_input" );
	if( audioInputSem < B_OK )
	{
		closeAudio();
		return false;
	}
	audioOutputSem = create_sem( 0, "bzflag_ringbuffer_output" );
	if( audioOutputSem < B_OK )
	{
		closeAudio();
		return false;
	}
	audioInputIndex = 0;
	audioOutputIndex = 0;
	//queued = 0;

	// make an output buffer
	outputBuffer = ( void* )malloc( audioBufferSize );
	if( outputBuffer == NULL )
	{
		closeAudio();
		return false;
	}


	format = media_raw_audio_format::wildcard;
#ifdef NO_FLOAT
	format.format = media_raw_audio_format::B_AUDIO_SHORT;
#else 
	format.format = media_raw_audio_format::B_AUDIO_FLOAT;
#endif 
	format.byte_order = B_HOST_IS_LENDIAN ? B_MEDIA_LITTLE_ENDIAN : B_MEDIA_BIG_ENDIAN;
	format.channel_count = 2;
	format.buffer_size = AUDIO_BLOCK_SIZE;
	format.frame_rate = defaultAudioRate;
	soundPlayer = new BSoundPlayer( &format, "bzflag output", audioplay_callback );
	if( soundPlayer->InitCheck() != B_OK )
	{
		closeAudio();
		return false;
	}
	soundPlayer->SetCookie(( void* )this );
	//  soundPlayer->SetVolume(1.0);
	soundPlayer->Start();
	soundPlayer->SetVolume( 0.1 );
	soundPlayer->SetHasData( true ); /* delay playback until the first frame */

	// Set default no thread
	childThreadID = 0;

	// ready to go
	audioReady = true;
	return true;
}

/* called back by BSoundPlayer */
void BeOSMedia::audioplay_callback( void *cookie, void *buffer, size_t bufferSize, const media_raw_audio_format &format )
{
	status_t err;
	BeOSMedia *s;
	size_t len, amount;
	unsigned char *buf = ( unsigned char* )buffer;

	DBG(( "audio_callback(, , %d, )\n", bufferSize ));
	s = ( BeOSMedia* )cookie;
	memset( buffer, 0, bufferSize );

#ifdef USE_PIPE
	if( s->checkLowWater )
	{
		//write_port(s->audioQueuePort, 'Late', NULL, 0);
		s->checkLowWater = false;
		release_sem( s->lowWaterSem );
	}
	struct stat st;
	fstat( p[0], &st );
	if( st.st_size < 1LL )
		return ;
	read( p[0], buffer, bufferSize );
	return ;
#endif 

	//return;//XXX test
	if( s->audioHasQuit )
		return ;
	while( bufferSize > 0 )
	{
		len = MIN( AUDIO_BLOCK_SIZE, bufferSize );
		len = MIN( len, ( AUDIO_BUFFER_SIZE - s->audioOutputIndex ));
		err = acquire_sem_etc( s->audioOutputSem, len, B_CAN_INTERRUPT | B_RELATIVE_TIMEOUT, 5000LL );
		//puts("audio callback unlocked");
		//DBG(("audio CB: acquire(%d), error 0x%08lx\n\n", len, err));
		if( err < B_OK )
		{
			if( err == B_TIMED_OUT )
			{
				DBG(( "audio callback: timed out.\n" ));
				/* tell we are late */
				if( s->checkLowWater )
				{
					//write_port(s->audioQueuePort, 'Late', NULL, 0);
					s->checkLowWater = false;
					release_sem( s->lowWaterSem );
				} return ;
				continue;
			}
			s->audioHasQuit = 1;
			s->soundPlayer->SetHasData( false );
			return ;
		}
		/* tell we are late */
		int32 sc = 0;
		if( s->checkLowWater && ( B_OK == get_sem_count( s->audioOutputSem, &sc )) && ( sc < s->audioLowWaterMark ))
		{
			//write_port(s->audioQueuePort, 'Late', NULL, 0);
			s->checkLowWater = false;
			release_sem( s->lowWaterSem );
			DBG(( "audio callback: 'Doctor Evil, I'm late !'\n" ));
		}
		DBG(( "################ audio avail = %ld\n", sc ));

		MYASSERT( s->audioOutputIndex >= 0 );
		MYASSERT( s->audioOutputIndex < AUDIO_BUFFER_SIZE );

		amount = len;
		DBG(( "AUDIO: reading %8ld bytes from %p\n", amount, &(( unsigned char* )s->outputBuffer )[s->audioOutputIndex] ));
		memcpy( buf, &(( unsigned char* )s->outputBuffer )[s->audioOutputIndex], amount );
		s->audioOutputIndex += amount;
		if( s->audioOutputIndex >= AUDIO_BUFFER_SIZE )
		//      s->audioOutputIndex %= AUDIO_BUFFER_SIZE;
			s->audioOutputIndex = 0;
		release_sem_etc( s->audioInputSem, len, 0 );
		//DBG(("audio CB: release(%d)\n\n", len));
#ifdef DEBUG_TO_WAV2
		fwrite( buf, sizeof( char ), len, debugWav2 );
#endif 
		buf += len;
		bufferSize -= len;
	}
	//  buf = (unsigned char *)buffer;
	//  printf("cbbuff= %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
	//  write(debugWav, buf, 2048);
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

void BeOSMedia::closeAudio()
{
	if( audioQueuePort > B_OK )
		delete_port( audioQueuePort );
	audioQueuePort =  - 1;
	if( lowWaterSem > B_OK )
		delete_sem( lowWaterSem );
	lowWaterSem =  - 1;
	if( audioInputSem > B_OK )
		delete_sem( audioInputSem );
	audioInputSem =  - 1;
	if( audioOutputSem > B_OK )
		delete_sem( audioOutputSem );
	audioHasQuit = true;

	if( soundPlayer )
	{
		soundPlayer->Stop();
		delete soundPlayer;
	}
	soundPlayer = NULL;

	if( outputBuffer )
		free( outputBuffer );
	outputBuffer = NULL;

	audioOutputSem =  - 1;
	audioInputIndex = 0;
	audioOutputIndex = 0;
#ifdef DEBUG_TO_WAV
	fclose( debugWav );
#endif 
#ifdef DEBUG_TO_WAV2
	fclose( debugWav2 );
#endif 
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

bool BeOSMedia::startAudioThread( void( *proc )( void* ), void *data )
{
	// if no audio thread then just call proc and return
	if( !hasAudioThread())
	{
		proc( data );
		return true;
	}

	// has an audio thread so fork and call proc
	if( childThreadID )
		return true;
	childThreadID = spawn_thread(( thread_func )proc, "bzflag_audio_thread", B_URGENT_DISPLAY_PRIORITY, data );
	if( childThreadID < B_OK )
		return false;
	resume_thread( childThreadID );
	return true;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

void BeOSMedia::stopAudioThread()
{
	if( childThreadID > B_OK )
	{
		status_t err;
		kill( childThreadID, SIGTERM );
		wait_for_thread( childThreadID, &err );
	}
	childThreadID = 0;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

bool BeOSMedia::hasAudioThread()const
{
#if defined(NO_AUDIO_THREAD)
	return false;
#else 
	return true;
#endif 
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

static void die( int )
{
	exit_thread( B_OK );
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

void BeOSMedia::audioThreadInit( void* )
{
	// parent will kill me when it wants me to quit.  catch the signal
	// and gracefully exit.  don't use PlatformFactory because that
	// doesn't distinguish between processes.
	signal( SIGTERM, die );
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

void BeOSMedia::writeSoundCommand( const void *cmd, int len )
{
	DBG(( "BeOSMedia::writeSoundCommand(, %d)\n", len ));
	if( port_count( audioQueuePort ) + 2 < audioQueueMaxCmds )
	{
		 /* we don't want to block */
		write_port( audioQueuePort, 'SndC', cmd, len );
		int32 semcnt = 0;
		get_sem_count( lowWaterSem, &semcnt );
		if( semcnt < 0 )
			release_sem( lowWaterSem );
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

bool BeOSMedia::readSoundCommand( void *cmd, int len )
{
	DBG(( "BeOSMedia::readSoundCommand(, %d)\n", len ));
	//  MYASSERT((size_t)len < sizeof(cmdBuffer));

	int32 what = 0;
	while( what != 'SndC' )
	{
		if( !port_count( audioQueuePort ))
			return false;
		read_port( audioQueuePort, &what, cmd, len );
	}
	return true;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

int BeOSMedia::getAudioOutputRate()const
{
	return audioOutputRate;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

int BeOSMedia::getAudioBufferSize()const
{
	return AUDIO_BLOCK_FRAME_COUNT *AUDIO_BLOCK_COUNT;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

int BeOSMedia::getAudioBufferChunkSize()const
{
	return AUDIO_BLOCK_FRAME_COUNT;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

bool BeOSMedia::isAudioTooEmpty()const
{
	return true;
	int32 semcnt = 0;
	//printf("BeOSMedia::isAudioTooEmpty()\n");
	get_sem_count( audioOutputSem, &semcnt ); // XXX: make sure
	//return (semcnt < -(AUDIO_BLOCK_FRAME_COUNT / 2));
	return ( semcnt < audioLowWaterMark );
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

void BeOSMedia::writeAudioFrames( const float *samples, int numFrames )
{
	status_t err = B_OK;
	int len, ret;
	int numSamples = 2 * numFrames;
	int size = numSamples * sizeof( SAMPTYPE );
	uint8 *buf = ( uint8* )samples;


	DBG(( "BeOSMedia::writeAudioFrames(, %d)\n", numFrames ));
	DBG(( "BeOSMedia::audioInputIndex = %d\n", audioInputIndex ));


	MYASSERT( audioInputIndex >= 0 );
	MYASSERT( audioInputIndex < AUDIO_BUFFER_SIZE );

	// convert the samples to a format we can handle:
	// float [-1:1]
	/*
	for (int j = 0; j < numSamples; j++) {
	float v = samples[j];
	if (v < -32767.0)
	v = -32767.0;
	if (v > 32767.0)
	v = 32767.0;
	samples[j] = v / 32767.0;
	#ifdef DEBUG_TO_WAV
	{
	uint16 s = short(v);
	write(debugWav, &s, sizeof(uint16));
	}
	#endif
	}*/

#ifdef DEBUG_TO_WAV
	//write(debugWav, samples, size);
#endif 
	//printf("inbuff= %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);

	//  if (!soundPlayer->HasData()) /* delay playback until the first frame */
	//    soundPlayer->SetHasData(true);

	//DBG(("snd %f %f %f %f %f %f %f %f\n", samples[0], samples[1], samples[2], samples[3], samples[4], samples[5], samples[6], samples[7]));
	//#ifdef USE_FLOAT
	/*
	float *s = (float *)samples;
	for (int i = numSamples; i; i--, s++)
	 *s /= 32767;
	     */ //#else
	//#error unsupported short audio format
	//#endif
	while( size > 0 )
	{
		int amount;
		SAMPTYPE *dst;
		len = MIN( size, AUDIO_BLOCK_SIZE );
		len = MIN( len, ( AUDIO_BUFFER_SIZE - audioInputIndex ));
		amount = len;
		//err = acquire_sem_etc(audioInputSem, len, B_CAN_INTERRUPT, 0LL);
		//    err = acquire_sem_etc(audioInputSem, len, B_CAN_INTERRUPT | B_RELATIVE_TIMEOUT, 5000LL);
		//DBG(("audio write: acquire(%d), error 0x%08lx\n\n", len, err));
		if( err < B_OK )
			return ;
		//memcpy(&((unsigned char *)outputBuffer)[audioInputIndex], buf, amount);
		dst = ( SAMPTYPE* )((( unsigned char* )outputBuffer ) + audioInputIndex );
		DBG(( "AUDIO: piping %8ld bytes at %p\n", amount, dst ));
		for( int i = amount / sizeof( SAMPTYPE ) - 1; i >= 0; i-- )
		{
			float v = MAX( MIN((( float* )buf )[i], 32767.0f ),  - 32767.0f );
			dst[i] = ( SAMPTYPE )v;
#ifdef DEBUG_TO_WAV
			/*    {
			uint16 s = short(dst[i]);
			fwrite(&s, sizeof(uint16), 1, debugWav);
			}*/
#endif 
#ifndef NO_FLOAT
			dst[i] /= 32767.0f;
#endif 
#ifdef DEBUG_TO_WAV
			//fwrite(&dst[i], sizeof(SAMPTYPE), 1, debugWav);
#endif 

			//dst[i] = MAX(MIN(dst[i], 1.0f), -1.0f);
		}
#ifdef DEBUG_TO_WAV
		fwrite( dst, amount, 1, debugWav );
#endif 
#ifdef USE_PIPE
		write( p[1], dst, amount );
#endif 
		//memcpy(dst, buf, amount);
		audioInputIndex += amount;
		if( audioInputIndex >= AUDIO_BUFFER_SIZE )
			audioInputIndex = 0;
#if 0
		if( audioInputIndex >= AUDIO_BUFFER_SIZE )
		{
			audioInputIndex %= AUDIO_BUFFER_SIZE;

			DBG(( "AUDIO: left: %ld, audioInputIndex: %ld\n", len - amount, audioInputIndex ));
			//memcpy(&((unsigned char *)outputBuffer)[audioInputIndex], buf + amount, len - amount);
			dst = ( SAMPTYPE* )( &(( unsigned char* )outputBuffer )[audioInputIndex] );
			DBG(( "AUDIO: piping %8ld bytes at %p\n", len - amount, dst ));
			for( int i = ( len - amount ) / sizeof( SAMPTYPE ); i; i-- )
			{
				float v = MAX( MIN((( float* )( buf + amount ))[i], 32767.0f ),  - 32767.0f );
				dst[i] = ( SAMPTYPE )v;
	#ifndef NO_FLOAT
				dst[i] = dst[i] / 32767;
	#endif 
				//dst[i] = MAX(MIN(dst[i], 1.0f), -1.0f);
			}
			audioInputIndex += len - amount;
			if( audioInputIndex >= AUDIO_BUFFER_SIZE )
				audioInputIndex %= AUDIO_BUFFER_SIZE;
		}
#endif 
		release_sem_etc( audioOutputSem, len, 0 );
		//DBG(("audio write: release(%d)\n\n", len));
		buf += len;
		size -= len;
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

void BeOSMedia::audioSleep( bool checkLowWater, double endTime )
{
	status_t err;
	DBG(( "BeOSMedia::audioSleep(%s, %f)\n", checkLowWater ? "true" : "false", ( float )endTime ));
	this->checkLowWater = checkLowWater;
	bigtime_t timeout = ( bigtime_t )( 1.0e6 *endTime );
	int32 flags = ( endTime < 0 ) ? 0 : B_RELATIVE_TIMEOUT;
	if( endTime < 0 )
		timeout = B_INFINITE_TIMEOUT;
	//port_buffer_size_etc(audioQueuePort, flags, timeout);
	err = acquire_sem_etc( lowWaterSem, 1, flags, timeout );
	DBG(( "audioSleep error 0x%08lx\n", err ));
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

double BeOSMedia::stopwatch( bool start )
{
	return BzfMedia::stopwatch( start ); // not better
	if( start )
	{
		stopWatchStart = system_time();
		return 0.0;
	}
	return (( double )( system_time() - stopWatchStart )) / 1.0e6;
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
