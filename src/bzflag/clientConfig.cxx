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

void initConfigData ( void )
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

	std::string name = getConfigDirName();
	name += "config";

	// add in hostname on UNIX
	if (getenv("HOST")) {
		name += ".";
		name += getenv("HOST");
	}

	return name;

#elif defined(_WIN32) /* !defined(_WIN32) */

	// get location of personal files from system.  this appears to be
	// the closest thing to a home directory on windows.  use root of
	// C drive as a default in case we can't get the path or it doesn't
	// exist.
	std::string oldConfigName = "config.cfg";
	std::string name = getConfigDirName("2.0");
	name += oldConfigName;
	return name;

#endif /* !defined(_WIN32) */
}

std::string getCurrentConfigFileName(void)
{
	std::string configFile = BZ_CONFIG_FILE_NAME;

	std::string name = getConfigDirName(BZ_CONFIG_DIR_VERSION);
	name += configFile;

#if !defined(_WIN32)
	// add in hostname on UNIX
	if (getenv("HOST")) {
		name += ".";
		name += getenv("HOST");
	}
#endif
	return name;
}

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
	fp = fopen(oldConfigName.c_str(), "rb");
	if (fp) {
		// there is an old config so lets copy it to the new dir and let the update take care of it.
#if defined(_WIN32)
		fclose(fp);
		// make the dir if we need to
		std::string configDir = getConfigDirName(BZ_CONFIG_DIR_VERSION);
		mkdir(configDir.c_str());
		// copy the old config to the new dir location with the new name
		CopyFile(oldConfigName.c_str(), configName.c_str(),true);

#else	// the other OSs should do what they need to do
		mkdir(getConfigDirName(BZ_CONFIG_DIR_VERSION).c_str(), 0755);

		FILE *newFile = fopen(configName.c_str(),"wb");
		if (newFile) {
			fseek(fp, 0, SEEK_END);
			int len = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			unsigned char* temp = (unsigned char*) malloc(len);

			fread(temp, len, 1, fp);
			fwrite(temp, len, 1, newFile);

			free(temp);

			fclose(newFile);
			fclose(fp);
		}
#endif
	}
}

void updateConfigFile(void)
{
	int		configVersion = 0;
	if (BZDB.isSet("config_vers"))
		configVersion = (int)BZDB.eval("config_vers");

	switch (configVersion) {
  case 0: // 2.1.0

  case 1:// 2.1.6
	  break; // no action, current version

  default: // hm, we don't know about this one...
	  printError(TextUtils::format("Config file is tagged version \"%d\", "
		  "which was not expected (too new perhaps). "
		  "Trying to load anyhow.", configVersion));
	  break;
	}

	// set us as the updated version
	configVersion = BZ_CONFIG_FILE_VERSION;
	BZDB.setInt("config_vers", configVersion);
	BZDB.unset("config_version");// pull the old version, we are updated
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
