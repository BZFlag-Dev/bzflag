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

#include "ConfigFileManager.h"
#include "ConfigFileReader.h"
#include "FileManager.h"
#include <iostream>

//
// ConfigFileManager
//

ConfigFileManager*		ConfigFileManager::mgr = NULL;

ConfigFileManager::ConfigFileManager()
{
	// do nothing
}

ConfigFileManager::~ConfigFileManager()
{
	// clean up
	for (Readers::iterator index = readers.begin();
							index != readers.end(); ++index)
		delete index->second;
	readers.clear();
	if (mgr == this)
		mgr = NULL;
}

ConfigFileManager*		ConfigFileManager::getInstance()
{
	if (mgr == NULL)
		mgr = new ConfigFileManager;
	return mgr;
}

void					ConfigFileManager::add(
							const std::string& tag,
							ConfigFileReader* adopted)
{
	remove(tag);
	readers.insert(std::make_pair(tag, adopted));
}

void					ConfigFileManager::remove(const std::string& tag)
{
	Readers::iterator index = readers.find(tag);
	if (index != readers.end()) {
		delete index->second;
		readers.erase(index);
	}
}

ConfigFileReader*		ConfigFileManager::get(const std::string& tag) const
{
	Readers::const_iterator index = readers.find(tag);
	if (index != readers.end())
		return index->second->clone();
	else
		return NULL;
}

void					ConfigFileManager::parse(XMLTree::iterator xml)
{
	XMLTree::sibling_iterator scan = xml.begin();
	XMLTree::sibling_iterator end  = xml.end();
	for (; scan != end; ++scan) {
		// ignore anything but a tag
		if (scan->type == XMLNode::Tag) {
			// get the appropriate reader
			ConfigFileReader* reader = get(scan->value);
			if (reader == NULL)
				throw XMLIOException(scan->position,
							string_util::format(
								"invalid tag `%s'",
								scan->value.c_str()));

			// let the reader parse
			try {
				reader->parse(scan);
				delete reader;
			}
			catch (...) {
				delete reader;
				throw;
			}
		}
	}
}

bool					ConfigFileManager::read(const std::string& filename)
{
	// try to open the file
	std::istream* stream = FILEMGR->createDataInStream(filename);
	if (stream == NULL)
		return false;

	// parse
	try {
		read(*stream, XMLStreamPosition(filename));
		delete stream;
		return true;
	}
	catch (...) {
		delete stream;
		throw;
	}
}

void					ConfigFileManager::read(std::istream& stream)
{
	read(stream, XMLStreamPosition());
}

void					ConfigFileManager::read(std::istream& stream,
							const XMLStreamPosition& position)
{
	// syntactic parsing
	XMLTree xmlTree;
	xmlTree.read(stream, position);

	// and semantic parsing
	parse(xmlTree.begin());
}
