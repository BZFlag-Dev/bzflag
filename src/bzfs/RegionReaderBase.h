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

#ifndef BZF_REGION_READER_BASE_H
#define BZF_REGION_READER_BASE_H

#include "RegionReader.h"

class RegionReaderBase : public RegionReader {
public:
	RegionReaderBase();
	virtual ~RegionReaderBase();

	// ConfigFileReader overrides
	virtual ConfigFileReader*
						clone();
	virtual void		parse(XMLTree::iterator);
};

#endif
// ex: shiftwidth=4 tabstop=4
