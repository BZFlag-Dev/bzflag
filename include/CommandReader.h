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

#ifndef BZF_COMMAND_READER_H
#define BZF_COMMAND_READER_H

#include "ConfigFileReader.h"

class CommandReader : public ConfigFileReader {
public:
	CommandReader();
	~CommandReader();

	// ConfigFileReader overrides.  commands are executed, results to
	// printError().
	ConfigFileReader*	clone();
	void				parse(XMLTree::iterator);

private:
	void				parseCommand(XMLTree::iterator);
};

#endif
// ex: shiftwidth=4 tabstop=4
