#ifdef WIN32
#pragma warning(4:4786)
#endif

#include <string>
#include "BundleMgr.h"
#include "Bundle.h"

BundleMgr::BundleMgr(const std::string &path, const std::string &name)
{
	bundlePath = path;
	bundleName = name;
}

BundleMgr::~BundleMgr()
{
	for (BundleMap::iterator it = bundles.begin(); it != bundles.end(); ++it)
		delete it->second;
	bundles.clear();
}

Bundle *BundleMgr::getBundle(const std::string &locale)
{
	BundleMap::iterator it = bundles.find(locale);
	if (it != bundles.end())
		return it->second;

	Bundle *parentBundle = NULL;
	if (locale.length() > 0) {
		std::string parentLocale = locale;

		int underPos = parentLocale.find_last_of('_');
		if (underPos >= 0)
			parentLocale = parentLocale.substr(0,underPos);
		else
			parentLocale.resize(0);
		parentBundle = getBundle(parentLocale);
	}

	Bundle *pB = new Bundle(parentBundle);

	std::string path = bundlePath + "/l10n/" + bundleName;
	if (locale.length() > 0)
		path += "_" + locale;
	path += ".po";
	pB->load(path);

	bundles.insert(std::pair<std::string,Bundle*>(locale, pB));

	return pB;
}



