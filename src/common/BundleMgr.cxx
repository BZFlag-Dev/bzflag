#ifdef WIN32
#pragma warning(4:4786)
#endif

#if (!defined(_WIN32) && !defined(WIN32))
#include <sys/types.h>
#include <dirent.h>
#else
#include <windows.h>
#endif

#include <string>
#include "BundleMgr.h"
#include "Bundle.h"

Bundle 		*BundleMgr::currentBundle 	= NULL;

std::string	BundleMgr::bundlePath		= "./data";

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

Bundle *BundleMgr::getBundle(const std::string &locale, bool setcur /*= true*/)
{
  BundleMap::iterator it = bundles.find(locale);
  if (it != bundles.end()) {
    if (setcur) currentBundle = it->second;
    return it->second;
  }

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

  if (setcur) currentBundle = pB;
  
  return pB;
}

Bundle *BundleMgr::getCurrentBundle()
{
    return currentBundle;
}

bool BundleMgr::getLocaleList(std::vector<std::string> *list) {
  if (list == NULL) return false;
  // There could have been stuff added to the list
  // prior to this call. Save the list count.
  int 	initSize = list->size();
  
  do {
	  
#if (defined(_WIN32) || defined(WIN32))
    char fileName[255], *end = NULL;

    // Prepare the wildcarded file path to search for and copy it to fileName
    sprintf(fileName, "%s\\l10n\\bzflag_*.po", bundlePath.c_str());
    
    HANDLE		hFoundFile	= NULL;
    WIN32_FIND_DATA	data;

    hFoundFile = FindFirstFile((LPCTSTR) fileName, &data);
    if (hFoundFile == INVALID_HANDLE_VALUE) break;	// Invalid path

    do {
      std::string poFile = data.cFileName;
      int dotPos = poFile.find_first_of('.');
      if ((dotPos >= 0) && (poFile.substr(dotPos+1) == "po")) {
	 int underPos = poFile.find_first_of('_');
	 if (underPos >= 0) {
	   std::string locale = poFile.substr(underPos+1, dotPos-underPos-1);
	   if (locale != "xx")
	     list->push_back(locale);
	 }
      }
    } while (FindNextFile(hFoundFile, &data) == TRUE);

    FindClose(hFoundFile);
    break;
    
#else

    // This should work for most of the currently supported
    // non Windows platforms i believe.
    DIR *localedir = opendir((bundlePath + "/l10n/").c_str());
    if (localedir == NULL) break;
    
    struct dirent 	*dirinfo = NULL;
    while ((dirinfo = readdir(localedir)) != NULL) {

      std::string poFile = dirinfo->d_name;
      int dotPos = poFile.find_first_of('.');
      if ((dotPos >= 0) && (poFile.substr(dotPos+1) == "po")) {
	 int underPos = poFile.find_first_of('_');
	 if (underPos >= 0) {
	   std::string locale = poFile.substr(underPos+1, dotPos-underPos-1);
	   if (locale != "xx")
	     list->push_back(locale);
	 }
      }
    }

    closedir(localedir);
	
#endif

  } while (0);

  return ((int)list->size() > initSize) ? true : false;
}
// ex: shiftwidth=2 tabstop=8
