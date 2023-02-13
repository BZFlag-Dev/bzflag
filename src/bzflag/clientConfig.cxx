/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
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

std::string getOldConfigFileName(void)
{
    return getConfigDirName("2.4") + BZ_CONFIG_FILE_NAME;
}

std::string getCurrentConfigFileName(void)
{
    return getConfigDirName(BZ_CONFIG_DIR_VERSION) + BZ_CONFIG_FILE_NAME;
}

#if !defined(_WIN32)
static void copyConfigFile(const char *oldConfigName, std::string configName)
{

    FILE *fp = fopen(oldConfigName, "rb");
    if (!fp)
        return;

    // there is an old config so lets copy it to the new dir and let the
    // update take care of it.
    mkdir(getConfigDirName(BZ_CONFIG_DIR_VERSION).c_str(), 0755);
    FILE *newFile = fopen(configName.c_str(), "wb");
    if (!newFile)
    {
        fclose(fp);
        return;
    }

    fseek(fp, 0, SEEK_END);
    const int len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    unsigned char *temp = (unsigned char *)malloc(len);
    if (temp == NULL)
    {
        printError("Insufficient Memory");
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
    if (fp)
    {
        // we found the current file, nothing to do, just return
        fclose(fp);
        return;
    }

    // try and find the old file
    std::string oldConfigName = getOldConfigFileName();
#if defined(_WIN32)
    fp = fopen(oldConfigName.c_str(), "rb");
    if (fp)
    {
        // there is an old config so lets copy it to the new dir and
        // let the update take care of it.
        fclose(fp);
        // make the dir if we need to
        std::string configDir = getConfigDirName(BZ_CONFIG_DIR_VERSION);
        mkdir(configDir.c_str());
        // copy the old config to the new dir location with the new name
        CopyFile(oldConfigName.c_str(), configName.c_str(), true);
    }
#else   // the other OSs should do what they need to do
    copyConfigFile(oldConfigName.c_str(), configName);
#endif
}

void updateConfigFile(void)
{
    int       configVersion = 0;
    if (BZDB.isSet("config_version"))
        configVersion = BZDB.evalInt("config_version");

    if (configVersion < 4 || configVersion > BZ_CONFIG_FILE_VERSION)    // hm, we don't know about this one...
    {
        printError(TextUtils::format("Config file is tagged version \"%d\", "
                                     "which was not expected (too new/old perhaps). "
                                     "Trying to load anyhow.", configVersion));
    }

    if (configVersion <= 4)   // Upgrade 2.4.0 (or 2.4.2, since the config file version was not incremented) to 2.4.4
    {
        BZDB.unset("displayZoom");      // removed in r22109
        BZDB.unset("radarShotLineType");    // existed only in r22117
        BZDB.unset("serifFont");        // serif font was removed
        // Reset the list server URL so that people who have switched to another
        // URL gets reset back to the new HTTPS URL
        BZDB.set("list", BZDB.getDefault("list"));
    }

    // Upgrade 2.4.26 to 2.4.28
    if (configVersion <= 5)
        BZDB.unset("forceFeedback");

    // Upgrade 2.4.x to 2.6.0
    if (configVersion <= 6)
    {
    }

    // set us as the updated version
    configVersion = BZ_CONFIG_FILE_VERSION;
    BZDB.setInt("config_version", configVersion);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
