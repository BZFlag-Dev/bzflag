/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	BZF_BUNDLEMGR_H
#define	BZF_BUNDLEMGR_H

#ifdef WIN32
#pragma warning(4:4786)
#endif

#include <string>
#include <vector>
#include <map>
#include "common.h"

#ifdef __APPLE__
#import <CoreFoundation/CoreFoundation.h>
#endif

class Bundle;

typedef std::map<std::string,Bundle *> BundleMap;

class BundleMgr
{
public:
	BundleMgr(const std::string &path, const std::string &bundleName);
	BundleMgr::~BundleMgr();
	Bundle *getBundle(const std::string &locale, bool setcur = true);

	static Bundle *getCurrentBundle();
	static bool getLocaleList(std::vector<std::string> *list);

private:
	BundleMgr(const BundleMgr &xBundleMgr);
	BundleMgr& operator=(const BundleMgr &xBundleMgr);

	static std::string bundlePath;
	std::string bundleName;
	BundleMap bundles;

	static Bundle *currentBundle;
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

