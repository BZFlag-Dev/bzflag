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

#include "RegionReaderBase.h"
#include "RegionManagerBase.h"
#include "Team.h"

//
// RegionReaderBase
//

RegionReaderBase::RegionReaderBase()
{
	// do nothing
}

RegionReaderBase::~RegionReaderBase()
{
	// do nothing
}

ConfigFileReader*		RegionReaderBase::clone()
{
	return new RegionReaderBase(*this);
}

void					RegionReaderBase::parse(XMLTree::iterator xml)
{
	// get team
	std::string teamName;
	if (!xml->getAttribute("team", teamName))
		throw XMLIOException(xml->position,
							string_util::format(
								"missing team in `%s'",
								xml->value.c_str()));
	TeamColor team = Team::getEnum(teamName);
	if (team == NoTeam)
		throw XMLIOException(xml->position,
							string_util::format(
								"invalid team `%s'",
								xml->value.c_str()));

	// parse child tags
	XMLTree::sibling_iterator scan = xml.begin();
	XMLTree::sibling_iterator end  = xml.end();
	for (; scan != end; ++scan) {
		if (scan->type == XMLNode::Tag) {
			if (scan->value == "shape") {
				// what kind of shape is this?
				bool base  = true;
				bool spawn = true;
				scan->getAttribute("base", xmlParseEnum(
									s_xmlEnumBool, xmlSetVar(base)));
				scan->getAttribute("spawn", xmlParseEnum(
									s_xmlEnumBool, xmlSetVar(spawn)));

				// shape must be either or both a base or spawn region
				if (!base && !spawn)
					throw XMLIOException(scan->position,
							string_util::format(
								"shape must be a base and/or spawn region",
								scan->value.c_str()));

				// get shape (possibly twice)
				RegionShape* baseShape  = base  ? parseShape(scan) : NULL;
				RegionShape* spawnShape = spawn ? parseShape(scan) : NULL;

				// add to manager
				RGNMGR_BASE->insert(team, baseShape, false);
				RGNMGR_BASE->insert(team, spawnShape, true);
			}
			else if (scan->value == "safety") {
				float x = 0.0f, y = 0.0f, z = 0.0f;
				scan->getAttribute("x", xmlStrToFloat(xmlSetVar(x)));
				scan->getAttribute("y", xmlStrToFloat(xmlSetVar(y)));
				scan->getAttribute("z", xmlStrToFloat(xmlSetVar(z)));
				RGNMGR_BASE->setSafety(team, Vec3(x, y, z));
			}
			else {
				throw XMLIOException(scan->position,
							string_util::format(
								"invalid tag `%s'",
								scan->value.c_str()));
			}
		}
	}
}
