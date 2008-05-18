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

#ifndef __BZFSTREAM_H__
#define __BZFSTREAM_H__

/*
 * Include "bzfstream.h" instead of <fstream> to avoid a bug in VC's
 * stream libraries while running non-Latin character sets.
 */

#if defined(_MSC_VER)
#define IOSTREAM_WIDE_NAME_HACK 1
#endif

#include <fstream>

#ifdef IOSTREAM_WIDE_NAME_HACK
#include <TextUtils.h>
#endif

inline std::ifstream* createIFStream(const std::string& filename)
{
#ifdef IOSTREAM_WIDE_NAME_HACK
    return new std::ifstream(TextUtils::convert_to_wide(filename).c_str(), std::ios::in);
#else
    return new std::ifstream(filename.c_str(), std::ios::in);
#endif // _WIN32
}

inline std::ofstream* createOFStream(const std::string& filename)
{
#ifdef IOSTREAM_WIDE_NAME_HACK
    return new std::ofstream(TextUtils::convert_to_wide(filename).c_str(), std::ios::out);
#else
    return new std::ofstream(filename.c_str(), std::ios::out);
#endif // _WIN32
}

#endif /* __BZFSTREAM_H__ */

/*
 * Local Variables: ***
 * mode: C++ ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */
