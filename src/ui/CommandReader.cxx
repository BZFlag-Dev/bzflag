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

#include <string.h>
#include "CommandReader.h"
#include "CommandManager.h"
#include "ErrorHandler.h"

//
// CommandReader
//

CommandReader::CommandReader()
{
	// do nothing
}

CommandReader::~CommandReader()
{
	// do nothing
}

ConfigFileReader*		CommandReader::clone()
{
	return new CommandReader;
}

void					CommandReader::parse(XMLTree::iterator xml)
{
	XMLTree::sibling_iterator scan = xml.begin();
	XMLTree::sibling_iterator end  = xml.end();
	for (; scan != end; ++scan) {
		// only "command" tag is allowed
		if (scan->type != XMLNode::Tag || scan->value != "command")
			throw XMLIOException(scan->position, "expected `command' tag");

		// handle it
		parseCommand(scan);
	}
}

void					CommandReader::parseCommand(XMLTree::iterator xml)
{
	XMLTree::sibling_iterator scan = xml.begin();
	XMLTree::sibling_iterator end  = xml.end();
	for (; scan != end; ++scan) {
		// only data is allowed
		if (scan->type != XMLNode::Data)
			throw XMLIOException(scan->position, "unexpected entity");

		// skip it if nothing but whitespace
		if (strspn(scan->value.c_str(), " \n\r\t") == scan->value.size())
			continue;

		// run it
		std::string result = CMDMGR->run(scan->value);
		if (!result.empty())
			printError("%s", result.c_str());
	}
}
// ex: shiftwidth=4 tabstop=4
