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

/*
 * Module to load resources
 */

#ifndef BZF_RESOURCES_H
#define	BZF_RESOURCES_H

#include "common.h"
#include "BzfString.h"
#include "bzfio.h"

class ResourceDatabase {
  public:
			ResourceDatabase();
			~ResourceDatabase();

    boolean		hasValue(const BzfString& name) const;
    BzfString		getValue(const BzfString& name) const;

    void		addValue(const BzfString& name, const BzfString& value);
    void		removeValue(const BzfString& name);

    friend istream&	operator>>(istream&, ResourceDatabase&);
    friend ostream&	operator<<(ostream&, const ResourceDatabase&);

  private:
			ResourceDatabase(const ResourceDatabase&);
    ResourceDatabase&	operator=(const ResourceDatabase&);

    int			getNameIndex(const BzfString& name) const;

  public:
    BzfStringAList	names;
    BzfStringAList	values;
};

#endif // BZF_RESOURCES_H
// ex: shiftwidth=2 tabstop=8
