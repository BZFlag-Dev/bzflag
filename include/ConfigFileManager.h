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

#ifndef BZF_CONFIG_FILE_MANAGER_H
#define BZF_CONFIG_FILE_MANAGER_H

#include "XMLTree.h"
#include <map>

#define CFGMGR (ConfigFileManager::getInstance())

class ConfigFileReader;

class ConfigFileManager {
public:
	ConfigFileManager();
	~ConfigFileManager();

	// add/remove configuration file reader.  the reader is adopted
	// by add().
	void				add(const std::string& tag, ConfigFileReader* adopted);
	void				remove(const std::string& tag);

	// clone and return a reader for a configuration file type.  returns
	// NULL if the type isn't known.  the client must delete the reader.
	ConfigFileReader*	get(const std::string& tag) const;

	// parse an XMLTree (i.e. a syntatically parsed config file).
	// throws XMLIOException if the tree cannot be parsed.  this
	// discards data and checks child tags against known readers.
	// for each recognized tag, a reader is created and used to
	// parse that child.
	void				parse(XMLTree::iterator);

	// read a configuration file.  read(filename) uses FileManager
	// to open the stream and returns false if the file cannot be
	// opened.  they all call parse().
	bool				read(const std::string& filename);
	void				read(std::istream&);
	void				read(std::istream&, const XMLStreamPosition&);

	// get the singleton instance
	static ConfigFileManager*	getInstance();

private:
	typedef std::map<std::string, ConfigFileReader*> Readers;

	Readers				readers;
	
	static ConfigFileManager*	mgr;
};

#endif
// ex: shiftwidth=4 tabstop=4
