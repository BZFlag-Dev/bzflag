/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


// BZFlag common header
#include "common.h"

#include <sys/types.h>
#include <sys/stat.h>

#if defined(_WIN32)
#  include <shlobj.h>
#  include <direct.h>
#else
#  include <pwd.h>
#  include <dirent.h>
#endif /* defined(_WIN32) */

#include "clientConfig.h"

#include "version.h"
#include "StateDatabase.h"
#include "KeyManager.h"
#include "TextUtils.h"
#include "DirectoryNames.h"
#include "ErrorHandler.h"

std::vector<std::string> configQualityValues;
std::vector<std::string> configViewValues;

void initConfigData(void)
{
  configQualityValues.push_back(std::string("low"));
  configQualityValues.push_back(std::string("medium"));
  configQualityValues.push_back(std::string("high"));
  configQualityValues.push_back(std::string("experimental"));

  configViewValues.push_back(std::string("normal"));
  configViewValues.push_back(std::string("stereo"));
  configViewValues.push_back(std::string("stacked"));
  configViewValues.push_back(std::string("three"));
  configViewValues.push_back(std::string("anaglyph"));
  configViewValues.push_back(std::string("interlaced"));
}

std::string	getOldConfigFileName(void)
{
#if !defined(_WIN32)

  std::string name = getConfigDirName("2.0");
  name += "config.cfg";

  return name;

#elif defined(_WIN32) /* !defined(_WIN32) */

  std::string name("C:");
  char dir[MAX_PATH];
  ITEMIDLIST* idl;
  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &idl))) {
    if (SHGetPathFromIDList(idl, dir)) {
      struct stat statbuf;
      if (stat(dir, &statbuf) == 0 && (statbuf.st_mode & _S_IFDIR) != 0)
	name = dir;
    }

    IMalloc* shalloc;
    if (SUCCEEDED(SHGetMalloc(&shalloc))) {
      shalloc->Free(idl);
      shalloc->Release();
    }
  }

  name += "\\My BZFlag Files\\2.0\\config.cfg";
  return name;

#endif /* !defined(_WIN32) */
}

#if !defined(_WIN32)		// who uses this sucker any more?
static std::string	getReallyOldConfigFileName()
{
  std::string name = getConfigDirName();
  name += "config";
  return name;
}
#endif

std::string getCurrentConfigFileName(void)
{
  return getConfigDirName(BZ_CONFIG_DIR_VERSION) + BZ_CONFIG_FILE_NAME;
}

#if !defined(_WIN32)
static void copyConfigFile(const char *oldConfigName, std::string configName) {

  FILE *fp = fopen(oldConfigName, "rb");
  if (!fp)
    return;

  // there is an old config so lets copy it to the new dir and let the
  // update take care of it.
  mkdir(getConfigDirName(BZ_CONFIG_DIR_VERSION).c_str(), 0755);
  FILE *newFile = fopen(configName.c_str(), "wb");
  if (!newFile) {
    fclose(fp);
    return;
  }

  fseek(fp, 0, SEEK_END);
  const int len = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  unsigned char *temp = (unsigned char *)malloc(len);
  if (temp == NULL) {
    printError("Unsufficient Memory");
    fclose(fp);
    fclose(newFile);
    return;
  }

  size_t items_read = fread(temp, len, 1, fp);
  fclose(fp);
  if (items_read != 1)
    printError("Old config file is not readable");

  size_t items_written = fwrite(temp, len, 1, newFile);
  fclose(newFile);
  if (items_written != 1)
    printError("New config file is not writable");

  free(temp);
}
#endif

// this function will look for the config, if it's not there,
// it will TRY and find an old one and copy it
// so that the update function can upgrade it to the current version
// the assumption is that there is a unique config per version
void findConfigFile(void)
{
  // look for the current file
  std::string configName = getCurrentConfigFileName();
  FILE *fp = fopen(configName.c_str(), "rb");
  if (fp) {
    // we found the current file, nothing to do, just return
    fclose(fp);
    return;
  }

  // try and find the old file
  std::string oldConfigName = getOldConfigFileName();
#if defined(_WIN32)
  fp = fopen(oldConfigName.c_str(), "rb");
  if (fp) {
    // there is an old config so lets copy it to the new dir and
    // let the update take care of it.
    fclose(fp);
    // make the dir if we need to
    std::string configDir = getConfigDirName(BZ_CONFIG_DIR_VERSION);
    mkdir(configDir.c_str());
    // copy the old config to the new dir location with the new name
    CopyFile(oldConfigName.c_str(), configName.c_str(), true);
  }
#else	// the other OSs should do what they need to do
  copyConfigFile(oldConfigName.c_str(), configName);
#endif

  // try and find the REALLY old file
  // who uses this sucker any more?
#if !defined(_WIN32)
  std::string realyOldConfigName = getReallyOldConfigFileName();
  // apparently only linux needs this so do the magic
  copyConfigFile(realyOldConfigName.c_str(), configName);
#endif
}

void updateConfigFile(void)
{
  int		configVersion = 0;
  if (BZDB.isSet("config_version"))
    configVersion = BZDB.evalInt("config_version");

  BzfKeyEvent key;
  if (configVersion == 0) {
    // update from old unversioned config
    // roaming fixes - remove keys bound to "roam translate *" and "roam rotate *"
    KEYMGR.unbindCommand("roam translate left");
    KEYMGR.unbindCommand("roam translate right");
    KEYMGR.unbindCommand("roam translate up");
    KEYMGR.unbindCommand("roam translate down");
    KEYMGR.unbindCommand("roam translate forward");
    KEYMGR.unbindCommand("roam translate backward");
    KEYMGR.unbindCommand("roam rotate left");
    KEYMGR.unbindCommand("roam rotate right");
    KEYMGR.unbindCommand("roam rotate up");
    KEYMGR.unbindCommand("roam rotate down");
    KEYMGR.unbindCommand("roam rotate stop");

    // add new default keybindings if there's no conflict

    // iconify

    if (KEYMGR.stringToKeyEvent("F4", key) && (KEYMGR.get(key, true) == ""))
      KEYMGR.bind(key, true, "iconify");
    // toggle console & radar
    if (KEYMGR.stringToKeyEvent("Q", key) && (KEYMGR.get(key, true) == ""))
      KEYMGR.bind(key, true, "toggleRadar");
    if (KEYMGR.stringToKeyEvent("W", key) && (KEYMGR.get(key, true) == ""))
      KEYMGR.bind(key, true, "toggleConsole");
    // controlpanel tabs - all or nothing
    if (KEYMGR.stringToKeyEvent("Shift+F1", key)
      && (KEYMGR.get(key, true) == "")
      && KEYMGR.stringToKeyEvent("Shift+F2", key)
      && (KEYMGR.get(key, true) == "")
      && KEYMGR.stringToKeyEvent("Shift+F3", key)
      && (KEYMGR.get(key, true) == "")
      && KEYMGR.stringToKeyEvent("Shift+F4", key)
      && (KEYMGR.get(key, true) == "")) {
      KEYMGR.stringToKeyEvent("Shift+F1", key);
      KEYMGR.bind(key, true, "messagepanel all");
      KEYMGR.stringToKeyEvent("Shift+F2", key);
      KEYMGR.bind(key, true, "messagepanel chat");
      KEYMGR.stringToKeyEvent("Shift+F3", key);
      KEYMGR.bind(key, true, "messagepanel server");
      KEYMGR.stringToKeyEvent("Shift+F4", key);
      KEYMGR.bind(key, true, "messagepanel misc");
    }// TODO - any other breaking changes from 1.10 to 2.0
  }

  if (configVersion <= 1) {
    if (KEYMGR.stringToKeyEvent("Tab", key)
      && (KEYMGR.get(key, false) == ""))
      KEYMGR.bind(key, false, "jump");
  }

  if (configVersion <= 2) {
    if (KEYMGR.stringToKeyEvent("7", key)
      && (KEYMGR.get(key, true) == ""))
      KEYMGR.bind(key, true, "addhunt");
  }

  if (configVersion <= 3) {   // Upgrade from 2.0.x to 2.4.0
    if (BZDB.isSet("email")) {	// Convert from email to motto
      std::string email = BZDB.get("email");	// If the email is set, see if we should convert it
						// If the email is set and does not contain an @ sign, move it to motto
      if (!email.empty() && email.find('@') == std::string::npos) {
	BZDB.set("motto", email);
      }
      BZDB.unset("email");	// discard email string from before version 2.4
    }
    if (BZDB.isSet("emailDispLen")) {
      BZDB.set("mottoDispLen", BZDB.get("emailDispLen"));
      BZDB.unset("emailDispLen");	// discard setting from before version 2.4
    }

    if (BZDB.isSet("hideEmails")) {
      BZDB.setBool("hideMottos", BZDB.isTrue("hideEmails"));
      BZDB.unset("hideEmails");	// discard setting from before version 2.4
    }

    // Get rid of geometry and lastScreenshot settings
    BZDB.unset("geometry");
    BZDB.unset("lastScreenshot");

    // Turn off dithering (since none of our automatic performance checks turn it on anymore)
    BZDB.setBool("dither", false);
  }

  if (configVersion <= 4) { // Upgrade 2.4.0 (or 2.4.2, since the config file version was not incremented) to 2.4.4
    BZDB.unset("displayZoom");		// removed in r22109
    BZDB.unset("radarShotLineType");	// existed only in r22117
    BZDB.unset("serifFont");		// serif font was removed
					// Reset the list server URL so that people who have switched to another
					// URL gets reset back to the new HTTPS URL
    BZDB.set("list", BZDB.getDefault("list"));
  }

  if (configVersion <= 5) {
  }

  if (configVersion < 0 || configVersion > 5) {  // hm, we don't know about this one...
    printError(TextUtils::format("Config file is tagged version \"%d\", "
      "which was not expected (too new perhaps). "
      "Trying to load anyhow.", configVersion));
  }

  // set us as the updated version
  configVersion = BZ_CONFIG_FILE_VERSION;
  BZDB.setInt("config_version", configVersion);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
