#ifndef	BZF_BUNDLEMGR_H
#define	BZF_BUNDLEMGR_H

#ifdef WIN32
#pragma warning(4:4786)
#endif

#include <string>
#include <vector>
#include <map>
#include "common.h"

#ifdef _MACOSX_
#include <CoreFoundation/CoreFoundation.h>
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
// ex: shiftwidth=2 tabstop=8
