/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* BaseResources:
 *	Interface for resource classes
 */

#ifndef BZF_BASE_RESOURCES_H
#define	BZF_BASE_RESOURCES_H

#include "common.h"

#include <vector>
#include <string>

class std::ostream;

class BaseResources {
  public:
			BaseResources();
			BaseResources(const BaseResources&);
			~BaseResources();
    BaseResources&	operator=(const BaseResources&);

    bool		hasName(const std::string&) const;
    const std::vector<std::string>& getNames() const;

    void		addName(const std::string&);

  protected:
  static std::ostream&	print(std::ostream&, const std::string& name,
					const char* format, ...); // const
    static bool	match(const std::string& wildName,
					const std::string& name); // const
  private:
    static int		doMatch(const char* pattern,
					const char* string); // const
    static int		matchStar(const char* pattern,
					const char* string); // const

  private:
    std::vector<std::string>	names;
};

//
// BaseResources
//

inline const std::vector<std::string>&	BaseResources::getNames() const
{
  return names;
}

#endif // BZF_BASE_RESOURCES_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
