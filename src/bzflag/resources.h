/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Module to load resources
 */

#ifndef BZF_RESOURCES_H
#define	BZF_RESOURCES_H

#ifdef _WIN32
#pragma warning( 4 : 4786 )
#endif

#include "common.h"
#include "bzfio.h"
#include <string>
#include <vector>

class ResourceDatabase {
  public:
			ResourceDatabase();
			~ResourceDatabase();

    bool		hasValue(const std::string& name) const;
    std::string		getValue(const std::string& name) const;

    void		addValue(const std::string& name, const std::string& value);
    void		removeValue(const std::string& name);

    friend istream&	operator>>(istream&, ResourceDatabase&);
    friend ostream&	operator<<(ostream&, const ResourceDatabase&);

  private:
			ResourceDatabase(const ResourceDatabase&);
    ResourceDatabase&	operator=(const ResourceDatabase&);

    int			getNameIndex(const std::string& name) const;

  public:
    std::vector<std::string>	names;
    std::vector<std::string>	values;
};

#endif // BZF_RESOURCES_H
// ex: shiftwidth=2 tabstop=8
