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


#ifndef __PARSE_MATERIAL_H__
#define __PARSE_MATERIAL_H__

/* system interface headers */
#include <iostream>

/* common interface headers */
#include "BzMaterial.h"

extern bool parseMaterials(const char* cmd, std::istream& input,
			   BzMaterial* materials, int materialCount,
			   bool& error);

extern bool parseMaterialsByName(const char* cmd, std::istream& input,
				 BzMaterial* materials, const char** names,
				 int materialCount, bool& error);

#endif  /* __PARSE_MATERIAL_H__ */


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
