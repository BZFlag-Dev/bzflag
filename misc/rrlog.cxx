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


//  RRLOG
//
//  This program can be used by server admins to
//  examine their record/replay files in detail.
//

// system headers
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#ifndef _WIN32
	#include <sys/time.h>
	#include <unistd.h>
typedef int64_t s64;
#else 
typedef __int64 s64;
#endif 

// common headers
#include "common.h"
#include "global.h"
#include "Protocol.h"
#include "Pack.h"
#include "TextUtils.h"
#include "version.h"

// bzfs headers
#include "RecordReplay.h"

// local headers
#include "MsgStrings.h"


// Type Definitions
// ----------------

typedef uint16_t u16;
typedef uint32_t u32;
typedef s64 RRtime;

typedef struct RRpacket
{
	struct RRpacket *next;
	struct RRpacket *prev;
	u16 mode;
	u16 code;
	u32 len;
	u32 nextFilePos;
	u32 prevFilePos;
	RRtime timestamp;
	char *data;
} RRpacket;

static const int RRpacketHdrSize = sizeof( RRpacket ) - ( 2 *sizeof( RRpacket* ) - sizeof( char* ));
typedef struct
{
	u32 magic; // record file type identifier
	u32 version; // record file version
	u32 offset; // length of the full header
	RRtime filetime; // amount of time in the file
	u32 player; // player that saved this record file
	u32 flagsSize; // size of the flags data
	u32 worldSize; // size of world database
	char callSign[CallSignLen]; // player's callsign
	char email[EmailLen]; // player's email
	char serverVersion[8]; // BZFS protocol version
	char appVersion[MessageLen]; // BZFS application version
	char realHash[64]; // hash of worldDatabase
	char worldSettings[4+WorldSettingsSize]; // the game settings
	char *flags; // a list of the flags types
	char *world; // the world
} ReplayHeader;

static const int ReplayHeaderSize = sizeof( ReplayHeader ) - ( 2 *sizeof( char* ));


// Function Prototypes
// -------------------

static void printHelp( const char *execName );
static bool loadHeader( ReplayHeader *h, FILE *f );
static RRpacket *loadPacket( FILE *f );
static void *nboUnpackRRtime( void *buf, RRtime &value );
static std::string strRRtime( RRtime timestamp );


int debugLevel = 0;
static int outputLevel = 0;


/****************************************************************************/

int main( int argc, char **argv )
{
	FILE *file = NULL;
	ReplayHeader header;
	RRpacket *p;
	const char *execName = argv[0];
	bool useColor = true;
	bool useEmail = true;
	bool onlyMessages = false;

	printf( "\nRRLOG-%s\nProtocol BZFS%s:  %i known packet types\n\n", getAppVersion(), getProtocolVersion(), MsgStrings::knownPacketTypes());

	if( argc < 2 )
	{
		printHelp( execName );
		exit( 1 );
	}

	while( argc > 1 )
	{
		if( strcmp( "-h", argv[1] ) == 0 )
		{
			printHelp( execName );
			exit( 0 );
		}
		else if( strcmp( "-o", argv[1] ) == 0 )
		{
			if( argc < 3 )
			{
				printf( "* Missing the -o parameter or the filename\n\n" );
				printHelp( execName );
				exit( 1 );
			}
			else
			{
				outputLevel = atoi( argv[2] );
				argc = argc - 2;
				argv = argv + 2;
			}
		}
		else if( strcmp( "-c", argv[1] ) == 0 )
		{
			useColor = false;
			argc--;
			argv++;
		}
		else if( strcmp( "-e", argv[1] ) == 0 )
		{
			useEmail = false;
			argc--;
			argv++;
		}
		else if( strcmp( "-m", argv[1] ) == 0 )
		{
			onlyMessages = true;
			argc--;
			argv++;
		}
		else
		{
			argc++;
			break;
		}
	}

	if( argc < 2 )
	{
		printf( "* Missing filename\n\n" );
		printHelp( execName );
		exit( 1 );
	}

	file = fopen( argv[1], "rb" );
	if( file == NULL )
	{
		perror( "fopen" );
		exit( 1 );
	}
	if( !loadHeader( &header, file ))
	{
		printf( "Couldn't load file header\n" );
		fclose( file );
		exit( 1 );
	}

	unsigned int secs = header.filetime / 1000000;
	unsigned int days = secs / ( 24 *60 * 60 );
	secs = secs % ( 24 *60 * 60 );
	unsigned int hours = secs / ( 60 *60 );
	secs = secs % ( 60 *60 );
	unsigned int minutes = secs / 60;
	secs = secs % 60;
	unsigned int usecs = header.filetime % 1000000;

	// load the first packet for its timestamp
	p = loadPacket( file );

	time_t startTime, endTime;
	if( p != NULL )
	{
		startTime = ( time_t )( p->timestamp / 1000000 );
		endTime = ( time_t )(( header.filetime + p->timestamp ) / 1000000 );
	}
	else
	{
		startTime = endTime = 0;
	}

	printf( "magic:     0x%04X\n", header.magic );
	printf( "replay:    version %i\n", header.version );
	printf( "offset:    %i\n", header.offset );
	printf( "length:    %-i days, %i hours, %i minutes, %i seconds, %i usecs\n", days, hours, minutes, secs, usecs );
	printf( "start:     %s", ctime( &startTime ));
	printf( "end:       %s", ctime( &endTime ));
	printf( "author:    %s  (%s)\n", header.callSign, header.email );
	printf( "bzfs:      bzfs-%s\n", header.appVersion );
	printf( "protocol:  %.8s\n", header.serverVersion );
	printf( "flagSize:  %i\n", header.flagsSize );
	printf( "worldSize: %i\n", header.worldSize );
	printf( "worldHash: %s\n", header.realHash );
	printf( "\n" );


	MsgStrings::init();
	MsgStrings::colorize( useColor );
	MsgStrings::showEmail( useEmail );
	//  MsgStrings::colorize(false);
	bool needUpdate = true;

	do
	{
		if( needUpdate && ( p->mode == RealPacket ))
		{
			needUpdate = false;
		}
		if(( p->mode == RealPacket ) || ( p->mode == HiddenPacket ) || (( p->mode == StatePacket ) && needUpdate ))
		{
			if( !onlyMessages || ( p->code == MsgMessage ))
			{
				int i, j;
				MsgStringList list = MsgStrings::msgFromServer( p->len, p->code, p->data );
				for( i = 0; i < ( int )list.size(); i++ )
				{
					if( list[i].level > outputLevel )
					{
						break;
					}
					if( i == 0 )
					{
						std::cout << strRRtime( p->timestamp ) << ": ";
					}
					for( j = 0; j < list[i].level; j++ )
					{
						std::cout << "  ";
					}
					std::cout << list[i].color;
					std::cout << list[i].text;
					if( useColor )
					{
						std::cout << "\033[0m";
					}
					std::cout << std::endl;
				}
			}
		}
		else if( p->mode == StatePacket )
		{
			MsgStrings::msgFromServer( p->len, p->code, p->data );
		}
		else if( p->mode == UpdatePacket )
		{
			std::cout << strRRtime( p->timestamp ) << ": UPDATE PACKET" << std::endl;
		}

		delete []p->data;
		delete p;
	}

	while(( p = loadPacket( file )) != NULL );

	delete []header.world;
	delete []header.flags;

	fclose( file )
		;

	return 0;
}

/****************************************************************************/

static void printHelp( const char *execName )
{
	printf( "usage:\t%s [options] <filename>\n\n", execName );
	printf( "  -h	  : print help\n" );
	printf( "  -o <level>  : set output level\n" );
	printf( "  -c	  : disable printing ANSI colors\n" );
	//  printf("  -e	  : disable printing emails\n");
	//  printf("  -m	  : only print message packets\n");
	printf( "\n" );
	return ;
}

/****************************************************************************/

static bool loadHeader( ReplayHeader *h, FILE *f )
{
	char buffer[ReplayHeaderSize];
	void *buf;

	if( fread( buffer, ReplayHeaderSize, 1, f ) <= 0 )
	{
		return false;
	}

	buf = nboUnpackUInt( buffer, h->magic );
	buf = nboUnpackUInt( buf, h->version );
	buf = nboUnpackUInt( buf, h->offset );
	buf = nboUnpackRRtime( buf, h->filetime );
	buf = nboUnpackUInt( buf, h->player );
	buf = nboUnpackUInt( buf, h->flagsSize );
	buf = nboUnpackUInt( buf, h->worldSize );
	buf = nboUnpackString( buf, h->callSign, sizeof( h->callSign ));
	buf = nboUnpackString( buf, h->email, sizeof( h->email ));
	buf = nboUnpackString( buf, h->serverVersion, sizeof( h->serverVersion ));
	buf = nboUnpackString( buf, h->appVersion, sizeof( h->appVersion ));
	buf = nboUnpackString( buf, h->realHash, sizeof( h->realHash ));

	// load the flags, if there are any
	if( h->flagsSize > 0 )
	{
		h->flags = new char[h->flagsSize];
		if( fread( h->flags, h->flagsSize, 1, f ) == 0 )
		{
			return false;
		}
	}
	else
	{
		h->flags = NULL;
	}

	// load the world database
	h->world = new char[h->worldSize];
	if( fread( h->world, h->worldSize, 1, f ) == 0 )
	{
		return false;
	}

	return true;
}

/****************************************************************************/

static RRpacket *loadPacket( FILE *f )
{
	RRpacket *p;
	char bufStart[RRpacketHdrSize];
	void *buf;

	if( f == NULL )
	{
		return false;
	}

	p = new RRpacket;

	if( fread( bufStart, RRpacketHdrSize, 1, f ) <= 0 )
	{
		delete p;
		return NULL;
	}
	buf = nboUnpackUShort( bufStart, p->mode );
	buf = nboUnpackUShort( buf, p->code );
	buf = nboUnpackUInt( buf, p->len );
	buf = nboUnpackUInt( buf, p->nextFilePos );
	buf = nboUnpackUInt( buf, p->prevFilePos );
	buf = nboUnpackRRtime( buf, p->timestamp );

	if( p->len > ( MaxPacketLen - (( int )sizeof( u16 ) *2 )))
	{
		fprintf( stderr, "loadPacket: ERROR, packtlen = %i\n", p->len );
		delete p;
		return NULL;
	}

	if( p->len == 0 )
	{
		p->data = NULL;
	}
	else
	{
		p->data = new char[p->len];
		if( fread( p->data, p->len, 1, f ) <= 0 )
		{
			delete []p->data;
			delete p;
			return NULL;
		}
	}

	return p;
}

/****************************************************************************/

static void *nboUnpackRRtime( void *buf, RRtime &value )
{
	u32 msb, lsb;
	buf = nboUnpackUInt( buf, msb );
	buf = nboUnpackUInt( buf, lsb );
	value = (( RRtime )msb << 32 ) + ( RRtime )lsb;
	return buf;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

static std::string strRRtime( RRtime timestamp )
{
	time_t date = ( time_t )( timestamp / 1000000 );
	char buffer[32];

	strftime( buffer, 32, "%Y/%m/%d %T", gmtime( &date ));
	std::string str = buffer;
	unsigned int millisecs = ( timestamp % 1000000 ) / 1000;
	str += TextUtils::format( " (%03i ms)", millisecs );
	return str;
}

/****************************************************************************/

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
