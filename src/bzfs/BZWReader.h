/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __BZWREADER_H__
#define __BZWREADER_H__

#include <iostream>
#include <vector>

class WorldFileObject;
class WorldInfo;


extern std::istream &readToken(std::istream& input, char *buffer, int n);
extern bool readWorldStream(std::istream& input, const char *location, std::vector<WorldFileObject*>& wlist);
extern WorldInfo *defineWorldFromFile(const char *filename);

#endif