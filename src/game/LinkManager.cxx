/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


// common goes first
#include "common.h"

// implementation header
#include "LinkManager.h"

// system headers
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <vector>
#include <iostream>

// common headers
#include "bzglob.h"
#include "Pack.h"
#include "Teleporter.h"
#include "ObstacleMgr.h"


LinkManager::LinkManager()
{
	// do nothing
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


LinkManager::~LinkManager()
{
	clear();
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void LinkManager::clear()
{
	linkNames.clear();
	linkNumbers.clear();
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void LinkManager::makeLinkName( int number, std::string &name )
{
	name = "/t";
	char buffer[8];
	sprintf( buffer, "%i", ( number / 2 ));
	name += buffer;
	name += ":";
	if(( number % 2 ) == 0 )
	{
		name += "f";
	}
	else
	{
		name += "b";
	}
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void LinkManager::addLink( int src, int dst )
{
	LinkNameSet link;
	makeLinkName( src, link.src );
	makeLinkName( dst, link.dst );
	linkNames.push_back( link );
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void LinkManager::addLink( const std::string &src, const std::string &dst )
{
	LinkNameSet link;
	if(( src[0] >= '0' ) && ( src[0] <= '9' ))
	{
		int number = atoi( src.c_str());
		makeLinkName( number, link.src );
	}
	else
	{
		link.src = src;
	}
	if(( dst[0] >= '0' ) && ( dst[0] <= '9' ))
	{
		int number = atoi( dst.c_str());
		makeLinkName( number, link.dst );
	}
	else
	{
		link.dst = dst;
	}
	linkNames.push_back( link );
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


static bool inIntList( int value, const std::vector < int >  &list )
{
	for( unsigned int i = 0; i < list.size(); i++ )
	{
		if( value == list[i] )
		{
			return true;
		}
	}
	return false;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void LinkManager::doLinking()
{
	unsigned int i;

	// initial the destinations
	const ObstacleList &teles = OBSTACLEMGR.getTeles();
	// findTelesByName() can not return higher then this
	linkNumbers.resize( teles.size() *2 );
	for( i = 0; i < linkNumbers.size(); i++ )
	{
		linkNumbers[i].dsts.clear();
	}

	for( i = 0; i < linkNames.size(); i++ )
	{
		LinkNameSet &link = linkNames[i];
		std::vector < int > srcNumbers;
		std::vector < int > dstNumbers;
		findTelesByName( link.src, srcNumbers );
		findTelesByName( link.dst, dstNumbers );

		bool broken = false;
		if( srcNumbers.size() <= 0 )
		{
			broken = true;
			DEBUG1( "broken link src: %s\n", link.src.c_str());
		}
		if( dstNumbers.size() <= 0 )
		{
			broken = true;
			DEBUG1( "broken link dst: %s\n", link.dst.c_str());
		}
		if( broken )
		{
			continue;
		}

		for( unsigned s = 0; s < srcNumbers.size(); s++ )
		{
			for( unsigned d = 0; d < dstNumbers.size(); d++ )
			{
				std::vector < int >  &dstsList = linkNumbers[srcNumbers[s]].dsts;
				if( !inIntList( dstNumbers[d], dstsList ))
				{
					// no duplicates
					dstsList.push_back( dstNumbers[d] );
				}
			}
		}
	}

	// fill in the blanks (passthru linkage)
	for( i = 0; i < linkNumbers.size(); i++ )
	{
		std::vector < int >  &dstsList = linkNumbers[i].dsts;
		if( dstsList.size() <= 0 )
		{
			const int t = ( i / 2 ) *2; // tele number
			const int f = 1-( i % 2 ); // opposite link
			dstsList.push_back( t + f );
		}
	}

	if( debugLevel >= 4 )
	{
		for( i = 0; i < teles.size(); i++ )
		{
			Teleporter *tele = ( Teleporter* )teles[i];
			DEBUG0( "TELE(%i): %s\n", i, tele->getName().c_str());
		}
		for( i = 0; i < linkNames.size(); i++ )
		{
			LinkNameSet &link = linkNames[i];
			DEBUG0( "LINKSRC: %-32sLINKDST: %s\n", link.src.c_str(), link.dst.c_str());
		}
		for( i = 0; i < linkNumbers.size(); i++ )
		{
			DEBUG0( "SRC %3i%c:  DSTS", ( i / 2 ), (( i % 2 ) == 0 ) ? 'f' : 'b' );
			for( unsigned int j = 0; j < linkNumbers[i].dsts.size(); j++ )
			{
				int dst = linkNumbers[i].dsts[j];
				DEBUG0( " %i%c", ( dst / 2 ), (( dst % 2 ) == 0 ) ? 'f' : 'b' );
			}
			DEBUG0( "\n" );
		}
	}

	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void LinkManager::findTelesByName( const std::string &name, std::vector < int >  &list )const
{
	list.clear();

	std::string glob = name;

	// no chars, no service
	if( glob.size() <= 0 )
	{
		return ;
	}

	// a leading ':' might be used to indicate absolute linking if
	// links are ever included in group definintions. strip it here
	// for forwards compatibiliy.
	if( glob[0] == ':' )
	{
		glob.erase( 0, 1 ); // erase 1 char from position 0
	}

	// make the trailing face specification case-independent
	const unsigned int last = ( unsigned int )glob.size() - 1;
	if(( glob[last] == 'F' ) || ( glob[last] == 'B' ))
	{
		glob[last] = tolower( glob[last] );
	}

	// add all teleporters that have matching names
	const ObstacleList &teles = OBSTACLEMGR.getTeles();
	for( unsigned int i = 0; i < teles.size(); i++ )
	{
		Teleporter *tele = ( Teleporter* )teles[i];
		const std::string &teleName = tele->getName();

		std::string front = teleName;
		front += ":f";
		if( glob_match( glob, front ))
		{
			list.push_back(( int )( i *2 ) + 0 );
		}

		std::string back = teleName;
		back += ":b";
		if( glob_match( glob, back ))
		{
			list.push_back(( int )( i *2 ) + 1 );
		}
	}

	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


int LinkManager::getTeleportTarget( int source )const
{
	assert( source < ( int )( 2 *OBSTACLEMGR.getTeles().size()));

	const std::vector < int >  &dstsList = linkNumbers[source].dsts;

	if( dstsList.size() == 1 )
	{
		return dstsList[0];
	}
	else if( dstsList.size() > 1 )
	{
		int target = rand() % int( dstsList.size());
		return dstsList[target];
	}
	else
	{
		assert( false );
		return 0;
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


int LinkManager::getTeleportTarget( int source, unsigned int seed )const
{
	assert( source < ( int )( 2 *OBSTACLEMGR.getTeles().size()));

	const std::vector < int >  &dstsList = linkNumbers[source].dsts;

	if( dstsList.size() == 1 )
	{
		return dstsList[0];
	}
	else if( dstsList.size() > 1 )
	{
		seed = ( seed *1103515245+12345 ) >> 8; // from POSIX rand() example
		seed = seed % ( dstsList.size());
		return dstsList[seed];
	}
	else
	{
		assert( false );
		return 0;
	}
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void *LinkManager::pack( void *buf )const
{
	buf = nboPackUInt( buf, ( uint32_t )linkNames.size());
	for( unsigned int i = 0; i < linkNames.size(); i++ )
	{
		buf = nboPackStdString( buf, linkNames[i].src );
		buf = nboPackStdString( buf, linkNames[i].dst );
	}
	return buf;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void *LinkManager::unpack( void *buf )
{
	clear(); // just in case
	unsigned int i;
	uint32_t count;
	buf = nboUnpackUInt( buf, count );
	for( i = 0; i < count; i++ )
	{
		LinkNameSet link;
		buf = nboUnpackStdString( buf, link.src );
		buf = nboUnpackStdString( buf, link.dst );
		linkNames.push_back( link );
	}
	return buf;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


void LinkManager::print( std::ostream &out, const std::string &indent )const
{
	for( unsigned int i = 0; i < linkNames.size(); i++ )
	{
		const LinkNameSet &link = linkNames[i];
		out << indent << "link" << std::endl;
		out << indent << "  from " << link.src << std::endl;
		out << indent << "  to   " << link.dst << std::endl;
		out << indent << "end" << std::endl << std::endl;
	}
	return ;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------


int LinkManager::packSize()const
{
	int fullSize = sizeof( uint32_t );
	for( unsigned int i = 0; i < linkNames.size(); i++ )
	{
		fullSize += nboStdStringPackSize( linkNames[i].src );
		fullSize += nboStdStringPackSize( linkNames[i].dst );
	}
	return fullSize;
}


/******************************************************************************/


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
