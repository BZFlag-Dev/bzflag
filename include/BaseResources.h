/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* BaseResources:
 *	Interface for resource classes
 */

#ifndef BZF_BASE_RESOURCES_H
#define	BZF_BASE_RESOURCES_H

#include "common.h"
#include "BzfString.h"

class ostream;

class BaseResources {
  public:
			BaseResources();
			BaseResources(const BaseResources&);
			~BaseResources();
    BaseResources&	operator=(const BaseResources&);

    boolean		hasName(const BzfString&) const;
    const BzfStringAList& getNames() const;

    void		addName(const BzfString&);

  protected:
    static ostream&	print(ostream&, const BzfString& name,
					const char* format, ...); // const
    static boolean	match(const BzfString& wildName,
					const BzfString& name); // const
  private:
    static int		doMatch(const char* pattern,
					const char* string); // const
    static int		matchStar(const char* pattern,
					const char* string); // const

  private:
    BzfStringAList	names;
};

//
// BaseResources
//

inline const BzfStringAList&	BaseResources::getNames() const
{
  return names;
}

#endif // BZF_BASE_RESOURCES_H
