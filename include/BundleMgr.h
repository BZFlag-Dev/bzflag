#ifndef	BZF_BUNDLEMGR_H
#define	BZF_BUNDLEMGR_H

#ifdef WIN32
#pragma warning(4:4786)
#endif

#include <string>
#include <map>

class Bundle;

typedef std::map<std::string,Bundle *> BundleMap;

class BundleMgr
{
public:
	BundleMgr(const std::string &path, const std::string &bundleName);
	BundleMgr::~BundleMgr();
	Bundle *getBundle(const std::string &locale);
private:
	BundleMgr(const BundleMgr &xBundleMgr);
	BundleMgr& operator=(const BundleMgr &xBundleMgr);

	std::string bundlePath;
	std::string bundleName;
	BundleMap bundles;
};

#endif
// ex: shiftwidth=2 tabstop=8
