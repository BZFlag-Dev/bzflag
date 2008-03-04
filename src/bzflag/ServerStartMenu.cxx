/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include "ServerStartMenu.h"

/* system implementation headers */
#include <vector>
#include <string>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#ifndef _WIN32
#  include <signal.h>
#  include <dirent.h>
#  include <pwd.h>
#  ifndef GUSI_20
#    include <sys/wait.h>
#  endif
#else
#  define _WINSOCKAPI_
#  include <shlobj.h>
#  include <sys/stat.h>
#  include <direct.h>
#  define _POSIX_
#  include <limits.h>
#  undef _POSIX_
#  include <process.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

/* common implementation headers */
#include "DirectoryNames.h"
#include "StateDatabase.h"
#include "FontManager.h"
#include "TextUtils.h"

/* local implementation headers */
#include "FontSizer.h"
#include "MenuDefaultKey.h"
#include "MainMenu.h"
#include "HUDuiList.h"
#include "playing.h"
#include "bzflag.h"
#include "HUDui.h"

char ServerStartMenu::settings[] = "bfaaaaabaaaaa";

ServerStartMenu::ServerStartMenu()
{
  // add controls
  HUDuiList* listHUD;
  std::vector<std::string>* items;

  // 0
  addControl(createLabel("Start Server"), false);

  // 1
  listHUD = createList("Style:");
  items = &listHUD->getList();
  items->push_back("Capture the Flag");
  items->push_back("Free for All");
  items->push_back("Rabbit Hunt (Random Selection)");
  items->push_back("Rabbit Hunt (Score-based Selection)");
  items->push_back("Rabbit Hunt (Killer Selection)");
  listHUD->update();
  addControl(listHUD);

  // 2
  listHUD = createList("Max Players:");
  items = &listHUD->getList();
  items->push_back("2");
  items->push_back("3");
  items->push_back("4");
  items->push_back("8");
  items->push_back("20");
  items->push_back("40");
  listHUD->update();
  addControl(listHUD);

  // 3
  listHUD = createList("Max Shots:");
  items = &listHUD->getList();
  items->push_back("1");
  items->push_back("2");
  items->push_back("3");
  items->push_back("4");
  items->push_back("5");
  items->push_back("8");
  items->push_back("10");
  items->push_back("15");
  items->push_back("20");
  listHUD->update();
  addControl(listHUD);

  // 4
  listHUD = createList("Teleporters:");
  items = &listHUD->getList();
  items->push_back("no");
  items->push_back("yes");
  listHUD->update();
  addControl(listHUD);

  // 5
  listHUD = createList("Ricochet:");
  items = &listHUD->getList();
  items->push_back("no");
  items->push_back("yes");
  listHUD->update();
  addControl(listHUD);

  // 6
  listHUD = createList("Jumping:");
  items = &listHUD->getList();
  items->push_back("no");
  items->push_back("yes");
  listHUD->update();
  addControl(listHUD);

  // 7
  listHUD = createList("Handicap:");
  items = &listHUD->getList();
  items->push_back("no");
  items->push_back("yes");
  listHUD->update();
  addControl(listHUD);

  // 8
  listHUD = createList("Superflags:");
  items = &listHUD->getList();
  items->push_back("no");
  items->push_back("good flags only");
  items->push_back("all flags");
  listHUD->update();
  addControl(listHUD);

  // 9
  listHUD = createList("Max Superflags:");
  items = &listHUD->getList();
  items->push_back("10");
  items->push_back("20");
  items->push_back("30");
  items->push_back("40");
  listHUD->update();
  addControl(listHUD);

  // 10
  listHUD = createList("Bad Flag Antidote:");
  items = &listHUD->getList();
  items->push_back("no");
  items->push_back("yes");
  listHUD->update();
  addControl(listHUD);

  // 11
  listHUD = createList("Bad Flag Time Limit:");
  items = &listHUD->getList();
  items->push_back("no limit");
  items->push_back("15 seconds");
  items->push_back("30 seconds");
  items->push_back("60 seconds");
  items->push_back("180 seconds");
  listHUD->update();
  addControl(listHUD);

  // 12
  listHUD = createList("Bad Flag Win Limit:");
  items = &listHUD->getList();
  items->push_back("no limit");
  items->push_back("drop after 1 win");
  items->push_back("drop after 2 wins");
  items->push_back("drop after 3 wins");
  listHUD->update();
  addControl(listHUD);

  // 13
  listHUD = createList("Game Over:");
  items = &listHUD->getList();
  items->push_back("never");
  items->push_back("after 5 minutes");
  items->push_back("after 15 minutes");
  items->push_back("after 60 minutes");
  items->push_back("after 3 hours");
  items->push_back("when a player gets +3");
  items->push_back("when a player gets +10");
  items->push_back("when a player gets +25");
  items->push_back("when a team gets +3");
  items->push_back("when a team gets +10");
  items->push_back("when a team gets +25");
  items->push_back("when a team gets +100");
  listHUD->update();
  addControl(listHUD);

  // 14
  listHUD = createList("Server Reset:");
  items = &listHUD->getList();
  items->push_back("no, quit after game");
  items->push_back("yes, reset for more games");
  listHUD->update();
  addControl(listHUD);

  // 15
  listHUD = createList("World Map:");
  items = &listHUD->getList();
  items->push_back("random map");

  // add a list of .bzw files found in the world file dir
  std::string searchDir = getWorldDirName();
  scanWorldFiles (searchDir, items);

  // add a list of .bzw files found in the config file dir
  searchDir = getConfigDirName();
  scanWorldFiles (searchDir, items);

  // add a list of .bzw files found in the data or current dir
  searchDir = BZDB.get("directory"); // could be an empty string
#ifdef _WIN32
  if (searchDir.length() == 0) {
    long availDrives = GetLogicalDrives();
    for (int i = 2; i < 31; i++) {
      if (availDrives & (1 << i)) {
	searchDir = 'a' + i;
	searchDir += ":";
	break;
      }
    }
  }
  searchDir += BZ_DIRECTORY_SEPARATOR;
#else
  if (searchDir.length() > 0) {
    searchDir += BZ_DIRECTORY_SEPARATOR;
  }
  else {
    searchDir = ".";
  }
#endif // _WIN32
  scanWorldFiles (searchDir, items);

  listHUD->update();
  addControl(listHUD);

  // 16
  start = createLabel("Start");
  addControl(start);

  // 17
  status = createLabel("");
  addControl(status, false);

  // 18
  failedMessage = createLabel("");
  addControl(failedMessage, false);

  initNavigation();

  // set settings
  loadSettings();
}
ServerStartMenu::~ServerStartMenu()
{
}

void ServerStartMenu::scanWorldFiles (const std::string& searchDir,
				      std::vector<std::string>* items)
{
#ifdef _WIN32
  std::vector<std::string> pattern;
  pattern.push_back(searchDir + "*.bzw");
  for (unsigned int i=0; i<pattern.size(); i++) {
    WIN32_FIND_DATA findData;
    HANDLE h = FindFirstFile(pattern[i].c_str(), &findData);
    if (h != INVALID_HANDLE_VALUE) {
      std::string file;
      do {
	file = findData.cFileName;
	worldFiles[file] = searchDir + file;
	items->push_back(file);
      } while (FindNextFile(h, &findData));
    }
  }
#else
  /* add a list of .bzw files found in the current dir */
  DIR *directory = opendir(searchDir.c_str());
  if (directory) {
    struct dirent* contents;
    std::string file;
    std::string suffix;
    while ((contents = readdir(directory))) {
      file = contents->d_name;
      if (file.length() > 4) {
	suffix = file.substr(file.length()-4, 4);
	if (TextUtils::compare_nocase(suffix, ".bzw") == 0) {
	  worldFiles[file] = searchDir + file;
	  items->push_back(file);
	}
      }
    }
    closedir(directory);
  }
#endif // _WIN32
  return;
}


HUDuiDefaultKey* ServerStartMenu::getDefaultKey()
{
  return MenuDefaultKey::getInstance();
}


void ServerStartMenu::setSettings(const char *_settings)
{
  if (strlen(_settings) != strlen(settings)) return;
  strncpy(settings, _settings, sizeof(settings));
}

void ServerStartMenu::loadSettings()
{
  HUDNavigationQueue& controls = getNav();
  const char* scan = settings;
  for (int i = 0; *scan; i++, scan++)
    ((HUDuiList*)controls[i])->setIndex(*scan - 'a');
}

void ServerStartMenu::show()
{
  setStatus("");
}

void ServerStartMenu::dismiss()
{
  HUDNavigationQueue& controls = getNav();
  char* scan = settings;
  for (int i = 0; *scan; i++, scan++)
    *scan = (char)('a' + ((HUDuiList*)controls[i])->getIndex());
}

void ServerStartMenu::execute()
{
  static const char*	serverApp = "bzfs";
  bool success = false;

  HUDNavigationQueue& listHUD = getNav();
  HUDuiControl* _focus = HUDui::getFocus();
  if (_focus == start) {
    // start it up:
    //   without: -p, -i, -q, -a, +f, -synctime
    //   -b if -c
    //   -h, -r if !-c
    //   +s if superflags != no
    //   -f for all bad flags if superflags == good only
    //
    // other options as they were set

    // get path to server from path to client
    // add 256 for flags room
    char serverCmd[PATH_MAX + 256];
    strncpy(serverCmd, argv0, PATH_MAX+255);
    char* base = strrchr(serverCmd, '/');

#if defined(_WIN32)
    char* base2 = strrchr(serverCmd, '\\');
    if (base2 && (!base || base2 - serverCmd > base - serverCmd))
      base = base2;
#endif

    if (!base) base = serverCmd;
    else base++;

    int lenrem = PATH_MAX+256 - (base - serverCmd);
    strncpy(base, serverApp, lenrem-1);

    // prepare arguments for starting server
    const char* args[30];
    int arg = 0;
    args[arg++] = serverApp;

#if defined(_WIN32)
    // always try a fallback port if default port is busy
    // (unix-like OSes will instead try to kill off the old process)
    args[arg++] = "-pf";
#endif

    // load the world map first, so that later arguments can
    // override any that are present in a map's "options" block
    if (((HUDuiList*)listHUD[14])->getIndex() != 0) { // not random
      args[arg++] = "-world";
      std::vector<std::string> fileList = ((HUDuiList*)listHUD[14])->getList();
      std::string filename
	= fileList[((HUDuiList*)listHUD[14])->getIndex()].c_str();
      args[arg++] = worldFiles[filename].c_str();
    }

    // game style
    if (((HUDuiList*)listHUD[0])->getIndex() == 0) {
      args[arg++] = "-c";
      args[arg++] = "-b";
    }
    else if (((HUDuiList*)listHUD[0])->getIndex() == 1){
      args[arg++] = "-h";
    }
    else {
	static const char* rabbitStyles[] = { "random", "score", "killer" };
	args[arg++] = "-rabbit";
	args[arg++] = rabbitStyles[(((HUDuiList*)listHUD[0])->getIndex()) - 2];
    }

    // max players
    static const char* numPlayers[] = { "2", "3", "4", "8", "20", "40" };
    args[arg++] = "-mp";
    args[arg++] = numPlayers[((HUDuiList*)listHUD[1])->getIndex()];

    // max shots
    static const char* numShots[] = { "1", "2", "3", "4", "5", "8", "10", "15", "20" };
    args[arg++] = "-ms";
    args[arg++] = numShots[((HUDuiList*)listHUD[2])->getIndex()];

    // teleporters
    if (((HUDuiList*)listHUD[3])->getIndex() == 1)
      args[arg++] = "-t";

    // ricochet
    if (((HUDuiList*)listHUD[4])->getIndex() == 1)
      args[arg++] = "+r";

    // jumping
    if (((HUDuiList*)listHUD[5])->getIndex() == 1)
      args[arg++] = "-j";

    // handicap
    if (((HUDuiList*)listHUD[6])->getIndex() == 1)
      args[arg++] = "-handicap";

    // superflags
    const int superflagOption = ((HUDuiList*)listHUD[7])->getIndex();
    if (superflagOption != 0) {
      if (superflagOption == 1) {
	args[arg++] = "-f";
	args[arg++] = "bad";
      }

      args[arg++] = "+s";

      // max superflags
      static const char* numFlagsList[] = { "10", "20", "30", "40" };
      args[arg++] = numFlagsList[((HUDuiList*)listHUD[8])->getIndex()];
    }

    // shaking
    static const char* shakingTime[] = { "", "15", "30", "60", "180" };
    static const char* shakingWins[] = { "", "1", "2", "3" };
    if (((HUDuiList*)listHUD[9])->getIndex() == 1) args[arg++] = "-sa";
    if (((HUDuiList*)listHUD[10])->getIndex() != 0) {
      args[arg++] = "-st";
      args[arg++] = shakingTime[((HUDuiList*)listHUD[11])->getIndex()];
    }
    if (((HUDuiList*)listHUD[11])->getIndex() != 0) {
      args[arg++] = "-sw";
      args[arg++] = shakingWins[((HUDuiList*)listHUD[12])->getIndex()];
    }

    // game over
    static const char* gameOverArg[] = { "",
					 "-time", "-time", "-time", "-time",
					 "-mps", "-mps", "-mps",
					 "-mts", "-mts", "-mts", "-mts" };
    static const char* gameOverValue[] = { "",
					   "300", "900", "3600", "10800",
					   "3", "10", "25",
					   "3", "10", "25", "100" };
    if (((HUDuiList*)listHUD[12])->getIndex() != 0) {
      args[arg++] = gameOverArg[((HUDuiList*)listHUD[13])->getIndex()];
      args[arg++] = gameOverValue[((HUDuiList*)listHUD[13])->getIndex()];
    }

    // server reset
    if (((HUDuiList*)listHUD[13])->getIndex() == 0)
      args[arg++] = "-g";

    // no more arguments
    args[arg++] = NULL;

    // start the server
#if defined(_WIN32)

    // Windows
    int result = (int)_spawnvp(_P_DETACH, serverCmd, (char* const*) args);
    if (result < 0) {
      if (errno == ENOENT)
	setStatus("Failed... can't find server program.");
      else if (errno == ENOMEM)
	setStatus("Failed... not enough memory.");
      else if (errno == ENOEXEC)
	setStatus("Failed... server program is not executable.");
      else
	setStatus(TextUtils::format("Failed... unknown error (%d).", errno, serverCmd).c_str());
      logDebugMessage(1,"Failed to start server (%s) - error %d.\n", serverCmd, errno);
    }
    else {
      setStatus("Server started.");
      success = true;
    }

#else /* defined(_WIN32) */

    // UNIX
    static pid_t pid = -1;

    // try to kill off the old process
    if (pid != -1) {
      int i;
      for (i = 0; i < 12; i++) {
	if (kill(pid, SIGTERM) == 0) {
	  TimeKeeper::sleep(0.25); // be gracious for max 3 seconds
	} else {
	  if (errno == ESRCH) {
	    break; // the pid doesn't exist
	  }
	}
      }
      if (i == 12) {
	kill(pid, SIGKILL); // be brutal
      }
    }

    // fork
    pid = fork();

    if (pid == -1) {
      setStatus("Failed... cannot fork.");
    }
    else if (pid == 0) {
      // child process.  close down stdio.
      close(0);
      close(1);
      close(2);

      // exec server
      execvp(serverCmd, (char* const*)args);
      // If execvp returns, bzfs wasnt at the anticipated location.
      // Let execvp try to find it in $PATH by feeding it the "bzfs" name by it self
      execvp(serverApp, (char* const*)args);
      // If that returns too, something bad has happened. Exit.
      exit(2);
    }
    else if (pid != 0) {
      // parent process.  wait a bit and check if child died.
      TimeKeeper::sleep(1.0);
      int pStatus;
      if (waitpid(pid, &pStatus, WNOHANG) != 0) {
	pid = -1;
	char failBuf[64];
	if (WIFEXITED(pStatus)) {
	  snprintf(failBuf, 64, "Failed (exit = %i).", WEXITSTATUS(pStatus));
	} else if (WIFSIGNALED(pStatus)) {
	  snprintf(failBuf, 64, "Failed (signal = %i).", WTERMSIG(pStatus));
	} else {
	  strcpy(failBuf, "Failed.");
	}
	setStatus(failBuf);
      } else {
	setStatus("Server started.");
	success = true;
      }
    }

#endif /* defined(_WIN32) */

    if (success) {
      // set server/port in join menu to localhost:5154
      StartupInfo* info = getStartupInfo();
      strcpy(info->serverName, "localhost");
      info->serverPort = 5154;  //note that if bzfs had to use a fallback port this will be wrong
    } // success
  }
}

void ServerStartMenu::setStatus(const char* msg, const std::vector<std::string> *parms)
{
  status->setString(msg, parms);
  FontManager &fm = FontManager::instance();
  const float widt = fm.getStringWidth(status->getFontFace(),
				     status->getFontSize(),
				     status->getString().c_str());
  status->setPosition(center - 0.5f * widt, status->getY());
}

void ServerStartMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);
  FontSizer fs = FontSizer(_width, _height);

  center = 0.5f * (float)_width;

  FontManager &fm = FontManager::instance();
  std::vector<HUDuiElement*>& listHUD = getElements();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  const int fontFace = title->getFontFace();

  // use a big font for title, smaller font for the rest
  fs.setMin(0, (int)(1.0 / BZDB.eval("headerFontSize") / 2.0));
  const float titleFontSize = fs.getFontSize(fontFace, "headerFontSize");

  fs.setMin(0, 20);
  const float fontSize = fs.getFontSize(listHUD[1]->getFontFace(), "menuFontSize");

  // reposition title
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStringWidth(fontFace, titleFontSize, title->getString().c_str());
  const float titleHeight = fm.getStringHeight(fontFace, titleFontSize);
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * (float)_width;
  y -= 1.0f * titleHeight;
  const float h = fm.getStringHeight(listHUD[1]->getFontFace(), fontSize);
  const int count = (const int)listHUD.size();
  for (int i = 1; i < count; i++) {
    if (listHUD[i] == start) {
      y -= 1.5f * h;
      listHUD[i]->setFontSize(1.5f * fontSize);
      listHUD[i]->setPosition(x, y);
    } else {
      listHUD[i]->setFontSize(fontSize);
      listHUD[i]->setPosition(x, y);
    }
    y -= 1.0f * h;
  }
}

HUDuiList* ServerStartMenu::createList(const char* str)
{
  HUDuiList* listHUD = new HUDuiList;
  listHUD->setFontFace(MainMenu::getFontFace());
  if (str) listHUD->setLabel(str);
  return listHUD;
}

HUDuiLabel* ServerStartMenu::createLabel(const char* str)
{
  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  if (str) label->setString(str);
  return label;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
