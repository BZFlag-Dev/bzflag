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

#ifndef BZF_CONFIG_FILE_READER_H
#define BZF_CONFIG_FILE_READER_H

#include "XMLTree.h"

class ConfigFileReader {
public:
	ConfigFileReader() { }
	virtual ~ConfigFileReader() { }

	// create a copy of this reader
	virtual ConfigFileReader*
						clone() = 0;

	// parse an XMLTree
	virtual void		parse(XMLTree::iterator) = 0;
};

#endif
