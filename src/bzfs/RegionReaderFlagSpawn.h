/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_REGION_READER_FLAG_SPAWN_H
#define BZF_REGION_READER_FLAG_SPAWN_H

#include "common.h"
#include <set>
#include <string>
#include "RegionReader.h"

class RegionReaderFlagSpawn : public RegionReader {
public:
	RegionReaderFlagSpawn();
	virtual ~RegionReaderFlagSpawn();

	// ConfigFileReader overrides
	virtual ConfigFileReader*
						clone();
	virtual void		parse(XMLTree::iterator);

private:
	typedef std::set<unsigned int> FlagSet;

	void				parseFlags(FlagSet&,
								std::string&, XMLTree::iterator) const;
};

#endif
