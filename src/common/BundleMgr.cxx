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
  char	fileName[255], *end = NULL;
  
  do {
	  
#if (defined(_WIN32) || defined(WIN32))

    // Prepare the wildcarded file path to search for and copy it to fileName
    sprintf(fileName, "%s\\l10n\\bzflag_*.po", bundlePath.c_str());
    
    HANDLE		hFoundFile	= NULL;
    WIN32_FIND_DATA	data;

    hFoundFile = FindFirstFile((LPCTSTR) fileName, &data);
    if (hFoundFile == INVALID_HANDLE_VALUE) break;	// Invalid path

    do {
      strcpy(fileName, &(data.cFileName[7]));
      if (strcmp(&(fileName[strlen(fileName) - 3]), ".po") != 0)
	continue;	// Doesnt end in ".po". Should not happen

      fileName[strlen(fileName) - 3] = '\0';

      if (strcmp(fileName, "xx") != 0)	// Dont add the xx locale
	list->push_back(fileName);

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
      // dirinfo points to an item in the directory
      const int length = strlen(dirinfo->d_name);
      if (length < 11) continue;  // Name is to short to contain bzflag_...po
      if (strncmp(dirinfo->d_name, "bzflag_", 7) ||
          strncmp(dirinfo->d_name + length - 3, ".po", 3))
        continue;  // The name doesnt start with bzflag_ or end with .po

      // This is a (hopefully) valid bzflag locale file
      // Copy the filename and strip out the trailing .po
      strncpy(fileName, &(dirinfo->d_name[7]), length - 7);
      fileName[length - 7] = '\0';
      end = strrchr(fileName, '.');
      if (end == NULL) continue;	// This should not be able to happen
      *end = '\0';
      
      // fileName should now contain our locale name
      if (strcmp(fileName, "xx") != 0)	// Dont add the xx locale
        list->push_back(fileName);
    }

    closedir(localedir);
	
#endif

  } while (0);

  return ((int)list->size() > initSize) ? true : false;
}
// ex: shiftwidth=2 tabstop=8
