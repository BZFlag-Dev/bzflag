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

/* interface header */
#include "NewVersionMenu.h"

/* system headers */
#include <stdio.h>
#ifdef WIN32
#include <direct.h>
#include <process.h>
#include <errno.h>
#endif

/* common implementation headers */
#include "FontManager.h"
#include "StateDatabase.h"
#include "DirectoryNames.h"
#include "TextUtils.h"

/* local implementation headers */
#include "MainMenu.h"
#include "HUDDialogStack.h"
#include "HUDui.h"
#include "HUDuiLabel.h"
#include "CommandsStandard.h"

/* as platforms get added to the automatic upgrade system, update this */
#if defined(WIN32)
  #define AUTOUPGRADE 1
#else
  #define AUTOUPGRADE 0
#endif

NewVersionMenu::NewVersionMenu(std::string announce, std::string url, std::string date) :
cURLManager(), byteTransferred(0)
{
#if AUTOUPGRADE
  // prep for possible download
  setURL(url);
  long timeout = 15;
  if (BZDB.isSet("httpTimeout")) {
    timeout = (long)BZDB.eval("httpTimeout");
  }
  setTimeout(timeout);
#endif

  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiLabel* label;
  int fontFace = MainMenu::getFontFace();

  status = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("");
  listHUD.push_back(status);

  label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString(announce);
  listHUD.push_back(label);

  label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("A new version of BZFlag has been released!");
  listHUD.push_back(label);

  label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString(date);
  listHUD.push_back(label);

#ifdef AUTOUPGRADE
  label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Would you like to upgrade now?");
  listHUD.push_back(label);

  label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("(Download and install: " + url + ")");
  listHUD.push_back(label);

  yes = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Yes!");
  listHUD.push_back(label);

  no = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Not yet");
  listHUD.push_back(label);

  initNavigation(listHUD, 6, 7);
#else
  label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Please upgrade as soon as possible.");
  listHUD.push_back(label);

  yes = NULL;

  no = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("OK");
  listHUD.push_back(label);

  initNavigation(listHUD, 5, 5);
#endif

}

NewVersionMenu::~NewVersionMenu()
{
}

void NewVersionMenu::execute()
{
  HUDuiControl* _focus = HUDui::getFocus();
  if (!_focus) {
    return;
#ifdef AUTOUPGRADE
  } else if (_focus == yes) {
    addHandle();
#endif
  } else if (_focus == no) {
    HUDDialogStack::get()->pop();
  }
}

void NewVersionMenu::collectData(char* ptr, int len)
{
  char buffer[128];
  double size = 0;
  getFileSize(size);
  cURLManager::collectData(ptr, len);
  byteTransferred += len;
  snprintf(buffer, 128, "Downloading update: %d/%d KB", byteTransferred/1024, (int)size/1024);
  ((HUDuiLabel*)status)->setString(buffer);
}

void NewVersionMenu::finalization(char *data, unsigned int length, bool good)
{
  if (good && data) {
    if (length) {
      // received update.  Now what to do with it?
#ifdef WIN32
      // make sure the directory exists
      mkdir(getTempDirName().c_str());
      // write data to temporary file
      const std::string tempfile = getTempDirName() + "\\temp-upgrade.exe";
      FILE* temp = fopen(tempfile.c_str(), "wb");
      if (temp)
	fwrite(data, 1, length, temp);
      fclose(temp);
      // start the program
      char* args [2];
      args[0] = "temp-upgrade.exe";
      args[1] = NULL;
      const int result = (const int)_spawnvp(_P_DETACH, tempfile.c_str(), args);
      if (result < 0) {
	if (errno == ENOENT)
	  ((HUDuiLabel*)status)->setString("Failed... can't find upgrade installer.");
	else if (errno == ENOMEM)
	  ((HUDuiLabel*)status)->setString("Failed... not enough memory.");
	else if (errno == ENOEXEC)
	  ((HUDuiLabel*)status)->setString("Failed... installer is not executable.");
	else
	  ((HUDuiLabel*)status)->setString(TextUtils::format("Failed... unknown error (%d).", errno).c_str());
	logDebugMessage(1,"Failed to start upgrade installer (%s) - error %d.\n", tempfile, errno);
      } else {
	((HUDuiLabel*)status)->setString("Installer started.");
	CommandsStandard::quit();
      }
#endif
    }
  } else {
    status->setLabel("Download Failed!");
  }
}

void NewVersionMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);

  // use a big font
  float fontSize = (float)_height / 15.0f;
  float smallFontSize = (float)_height / 45.0f;
  float x, y;
  FontManager &fm = FontManager::instance();
  const int fontFace = MainMenu::getFontFace();

  // heights
  const float fontHeight = fm.getStrHeight(fontFace, fontSize, " ");
  const float smallFontHeight = fm.getStrHeight(fontFace, smallFontSize, " ");

  // get stuff
  std::vector<HUDuiControl*>& listHUD = getControls();
  int i = 0;

  // status
  HUDuiLabel* label = (HUDuiLabel*)listHUD[i];
  label->setFontSize(smallFontSize);
  float labelWidth = fm.getStrLength(fontFace, smallFontSize, "Downloading update: 8888/8888 KB");
  x = 0.5f * ((float)_width - labelWidth);
  y = 2.0f * fontHeight;
  label->setPosition(x, y);

  // announcement
  label = (HUDuiLabel*)listHUD[++i];
  label->setFontSize(fontSize);
  labelWidth = fm.getStrLength(fontFace, fontSize, label->getString());
  x = 0.5f * ((float)_width - labelWidth);
  y = (float)_height - fontHeight - 1.5f * smallFontHeight;
  label->setPosition(x, y);

  // release-notice
  label = (HUDuiLabel*)listHUD[++i];
  label->setFontSize(smallFontSize);
  labelWidth = fm.getStrLength(fontFace, smallFontSize, label->getString());
  x = 0.5f * ((float)_width - labelWidth);
  y -= 3.0f * smallFontHeight;
  label->setPosition(x, y);

  // release-date
  label = (HUDuiLabel*)listHUD[++i];
  label->setFontSize(smallFontSize);
  labelWidth = fm.getStrLength(fontFace, smallFontSize, label->getString());
  x = 0.5f * ((float)_width - labelWidth);
  y -= 1.5f * smallFontHeight;
  label->setPosition(x, y);

  // user request
  label = (HUDuiLabel*)listHUD[++i];
  label->setFontSize(smallFontSize);
  labelWidth = fm.getStrLength(fontFace, smallFontSize, label->getString());
  x = 0.5f * ((float)_width - labelWidth);
  y -= 4.5f * smallFontHeight;
  label->setPosition(x, y);

#ifdef AUTOUPGRADE
  // download details
  label = (HUDuiLabel*)listHUD[++i];
  label->setFontSize(smallFontSize);
  labelWidth = fm.getStrLength(fontFace, smallFontSize, label->getString());
  x = 0.5f * ((float)_width - labelWidth);
  y -= 1.5f * smallFontHeight;
  label->setPosition(x, y);
#endif

  // first user option
  label = (HUDuiLabel*)listHUD[++i];
  label->setFontSize(smallFontSize);
  labelWidth = fm.getStrLength(fontFace, smallFontSize, label->getString());
  x = 0.5f * ((float)_width - labelWidth);
  y -= 1.0f * fontHeight;
  label->setPosition(x, y);

#ifdef AUTOUPGRADE
  // second user option
  label = (HUDuiLabel*)listHUD[++i];
  label->setFontSize(smallFontSize);
  labelWidth = fm.getStrLength(fontFace, smallFontSize, label->getString());
  y -= 1.5f * smallFontHeight;
  label->setPosition(x, y);
#endif

}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
