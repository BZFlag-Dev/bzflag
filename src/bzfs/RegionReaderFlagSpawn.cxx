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

#include "RegionReaderFlagSpawn.h"
#include "RegionManagerFlagSpawn.h"
#include "Region.h"
#include "RegionShape.h"
#include "Flag.h"

//
// RegionReaderFlagSpawn
//

RegionReaderFlagSpawn::RegionReaderFlagSpawn()
{
	// do nothing
}

RegionReaderFlagSpawn::~RegionReaderFlagSpawn()
{
	// do nothing
}

ConfigFileReader*		RegionReaderFlagSpawn::clone()
{
	return new RegionReaderFlagSpawn(*this);
}

void					RegionReaderFlagSpawn::parse(XMLTree::iterator xml)
{
	RegionShape* shape = NULL;
	std::string flags;

	// get flag list
	FlagSet flagSet;
	if (!xml->getAttribute("flags", flags) || flags.empty()) {
		// default to all non-team flags
		for (unsigned int i = FirstSuperFlag; i <= LastSuperFlag; ++i)
			flagSet.insert(i);
	}
	else {
		// parse flags string
		parseFlags(flagSet, flags, xml);
	}

	// parse child tags
	XMLTree::sibling_iterator scan = xml.begin();
	XMLTree::sibling_iterator end  = xml.end();
	for (; scan != end; ++scan) {
		if (scan->type == XMLNode::Tag) {
			if (scan->value == "shape")
				shape = parseShape(scan);
			else
				throw XMLIOException(scan->position,
							string_util::format(
								"invalid tag `%s'",
								scan->value.c_str()));
		}
	}

	// must have a shape by now
	if (shape == NULL)
		throw XMLIOException(xml->position,
							string_util::format(
								"missing shape in `%s'",
								xml->value.c_str()));

	// if there are no flags then discard shape and we're done
	if (flagSet.empty()) {
		delete shape;
		return;
	}

	// add region for each flag
	for (FlagSet::const_iterator i = flagSet.begin(); i != flagSet.end(); ++i)
		RGNMGR_FLAG_SPAWN->insert(static_cast<FlagId>(*i), new Region(shape));
}

void					RegionReaderFlagSpawn::parseFlags(
								FlagSet& flagSet,
								std::string& flags,
								XMLTree::iterator xml) const
{
	const char* cflags = flags.c_str();

	// flags is a comma separated list of flag names or abbreviations.
	std::string::size_type b = 0;
	while (b < flags.size()) {
		// find start of name
		b = flags.find_first_not_of(" \t\n\0,", b);

		// find comma
		std::string::size_type e = flags.find(',', b);

		// truncate string
		if (e == std::string::npos)
			e = flags.size();
		else
			flags[e] = '\0';

		// look it up
		FlagId flag = Flag::getIDFromName(cflags + b);
		if (flag == NullFlag)
			flag = Flag::getIDFromAbbreviation(cflags + b);
		if (flag == NullFlag) {
			if (strnocasecmp(cflags + b, "good") == 0) {
				for (unsigned int i = FirstSuperFlag; i <= LastSuperFlag; ++i)
					if (Flag::getType(static_cast<FlagId>(i)) != FlagSticky)
						flagSet.insert(i);
			}
			else if (strnocasecmp(cflags + b, "bad") == 0) {
				for (unsigned int i = FirstSuperFlag; i <= LastSuperFlag; ++i)
					if (Flag::getType(static_cast<FlagId>(i)) == FlagSticky)
						flagSet.insert(i);
			}
			else {
				throw XMLIOException(xml->position,
							string_util::format(
								"unknown flag ID `%s'",
								cflags + b));
			}
		}
		else {
			flagSet.insert(flag);
		}

		// next
		b = e;
	}
}
