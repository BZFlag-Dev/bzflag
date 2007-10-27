/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "BundleMgr.h"

// system headers
#if (!defined(_WIN32) && !defined(WIN32))
#include <sys/types.h>
#include <dirent.h>
#endif
#include <string>

// local implementation headers
#include "common.h"
#include "Bundle.h"

Bundle		*BundleMgr::currentBundle	= NULL;

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

  /* FIXME -- this needs to be in libplatform not here -- causes libcommon
   * to require corefoundation framework
   */
#if defined(__APPLE__)
  // This is MacOS X. Use the CoreFoundation resource location API
  // to find the correct language resource if 'default' is specified.
  if (locale.length() == 7 && locale.compare("default") == 0) {
    char	localePath[512];
    CFBundleRef	mainBundle = CFBundleGetMainBundle();
    CFArrayRef	locales	= NULL;
    CFURLRef	localeURL	= NULL;
    // Look for a resource in the main bundle by name and type.
    do {
      if (mainBundle == NULL) break;
      locales = CFBundleCopyResourceURLsOfType(mainBundle, CFSTR("po"), NULL);
      if (locales == NULL || CFArrayGetCount(locales) == 0) break;
      localeURL = (CFURLRef) CFArrayGetValueAtIndex(locales, 0);
      if(localeURL != NULL && ::CFURLGetFileSystemRepresentation(
	  localeURL, true, reinterpret_cast<UInt8 *>(localePath), sizeof(localePath))
	 ) {
	path = localePath;
      }
    } while(0);
    CFRelease(locales);
  }
#endif

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
  int	initSize = list->size();

  do {

#if (defined(_WIN32) || defined(WIN32))
    char fileName[255];

    // Prepare the wildcarded file path to search for and copy it to fileName
    snprintf(fileName, 255, "%s\\l10n\\bzflag_*.po", bundlePath.c_str());

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
    struct dirent	*dirinfo = NULL;
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

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
