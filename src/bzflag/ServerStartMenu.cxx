/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

/* interface header */
#include "ServerStartMenu.h"

/* system implementation headers */
#include <sys/types.h>
#ifndef _WIN32
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

/* common implementation headers */
#include "TextUtils.h"

/* local implementation headers */
#include "MenuDefaultKey.h"
#include "MainMenu.h"


char ServerStartMenu::settings[] = "bfaaaaabaaaaa";

ServerStartMenu::ServerStartMenu()
{
  // add controls
  std::vector<HUDuiControl*>& controls = getControls();
  HUDuiList* list;
  std::vector<std::string>* items;

  controls.push_back(createLabel("Start Server"));

  list = createList("Style:");
  items = &list->getList();
  items->push_back("Capture the Flag");
  items->push_back("Free for All");
  items->push_back("Rabbit Hunt");
  list->update();
  controls.push_back(list);

  list = createList("Max Players:");
  items = &list->getList();
  items->push_back("2");
  items->push_back("3");
  items->push_back("4");
  items->push_back("8");
  items->push_back("20");
  items->push_back("40");
  list->update();
  controls.push_back(list);

  list = createList("Max Shots:");
  items = &list->getList();
  items->push_back("1");
  items->push_back("2");
  items->push_back("3");
  items->push_back("4");
  items->push_back("5");
  list->update();
  controls.push_back(list);

  list = createList("Teleporters:");
  items = &list->getList();
  items->push_back("no");
  items->push_back("yes");
  list->update();
  controls.push_back(list);

  list = createList("Ricochet:");
  items = &list->getList();
  items->push_back("no");
  items->push_back("yes");
  list->update();
  controls.push_back(list);

  list = createList("Jumping:");
  items = &list->getList();
  items->push_back("no");
  items->push_back("yes");
  list->update();
  controls.push_back(list);

  list = createList("Superflags:");
  items = &list->getList();
  items->push_back("no");
  items->push_back("good flags only");
  items->push_back("all flags");
  list->update();
  controls.push_back(list);

  list = createList("Max Superflags:");
  items = &list->getList();
  items->push_back("10");
  items->push_back("20");
  items->push_back("30");
  items->push_back("40");
  list->update();
  controls.push_back(list);

  list = createList("Bad Flag Antidote:");
  items = &list->getList();
  items->push_back("no");
  items->push_back("yes");
  list->update();
  controls.push_back(list);

  list = createList("Bad Flag Time Limit:");
  items = &list->getList();
  items->push_back("no limit");
  items->push_back("15 seconds");
  items->push_back("30 seconds");
  items->push_back("60 seconds");
  items->push_back("180 seconds");
  list->update();
  controls.push_back(list);

  list = createList("Bad Flag Win Limit:");
  items = &list->getList();
  items->push_back("no limit");
  items->push_back("drop after 1 win");
  items->push_back("drop after 2 wins");
  items->push_back("drop after 3 wins");
  list->update();
  controls.push_back(list);

  list = createList("Game Over:");
  items = &list->getList();
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
  list->update();
  controls.push_back(list);

  list = createList("Server Reset:");
  items = &list->getList();
  items->push_back("no, quit after game");
  items->push_back("yes, reset for more games");
  list->update();
  controls.push_back(list);

  list = createList("World Map:");
  items = &list->getList();
  items->push_back("random map");

  /* add a list of world files found.  look in the current directory
   * as well as in the config file directory.
   */
#ifdef _WIN32
  /* add a list of .bzw files found in the current dir */
  std::string searchDir = BZDB.get("directory");
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

  searchDir += "\\";
  std::string pattern = searchDir + "*.bzw";
  WIN32_FIND_DATA findData;
  HANDLE h = FindFirstFile(pattern.c_str(), &findData);
  if (h != INVALID_HANDLE_VALUE) {
    std::string file;
    std::string suffix;
    while (FindNextFile(h, &findData)) {
      file = findData.cFileName;
      worldFiles[file] = searchDir + file;
      items->push_back(file);
    }
  }

  /* add a list of .bzw files found in the config file dir */
  searchDir = "C:";
  char dir[MAX_PATH];
  ITEMIDLIST* idl;
  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &idl))) {
    if (SHGetPathFromIDList(idl, dir)) {
      struct stat statbuf;
      if (stat(dir, &statbuf) == 0 && (statbuf.st_mode & _S_IFDIR) != 0)
	searchDir = dir;
    }
    IMalloc* shalloc;
    if (SUCCEEDED(SHGetMalloc(&shalloc))) {
      shalloc->Free(idl);
      shalloc->Release();
    }
  }
  // yes it seems silly but the windows way is to have "my" in front of any folder you make in the my docs dir
  // todo: make this stuff go into the application data dir.
  searchDir += "\\My BZFlag Files\\";
  pattern = searchDir + "*.bzw";
  h = FindFirstFile(pattern.c_str(), &findData);
  if (h != INVALID_HANDLE_VALUE) {
    std::string file;
    std::string suffix;
    while (FindNextFile(h, &findData)) {
      file = findData.cFileName;
      worldFiles[file] = searchDir + file;
      items->push_back(file);
    }
  }
#else
  /* add a list of .bzw files found in the current dir */
  std::string pattern = BZDB.get("directory") + "/";
  DIR *directory = opendir(pattern.c_str());
  if (directory) {
    struct dirent* contents;
    std::string file;
    std::string suffix;
    while ((contents = readdir(directory))) {
      file = contents->d_name;
      if (file.length() > 4) {
	suffix = file.substr(file.length()-4, 4);
	if (compare_nocase(suffix, ".bzw") == 0) {
	  worldFiles[file] = pattern + file;
	  items->push_back(file);
	}
      }
    }
    closedir(directory);
  }

  /* add a list of .bzw files found in the config file dir */
  struct passwd* pwent = getpwuid(getuid());
  pattern = "";
  if (pwent && pwent->pw_dir) {
    pattern += std::string(pwent->pw_dir);
    pattern += "/";
  }
  pattern += ".bzf/";
  directory = opendir(pattern.c_str());
  if (directory) {
    struct dirent* contents;
    std::string file;
    std::string suffix;
    while ((contents = readdir(directory))) {
      file = contents->d_name;
      if (file.length() > 4) {
	suffix = file.substr(file.length()-4, 4);
	if (compare_nocase(suffix, ".bzw") == 0) {
	  worldFiles[file] = pattern + file;
	  items->push_back(file);
	}
      }
    }
    closedir(directory);
  }

#endif

  list->update();
  controls.push_back(list);

  start = createLabel("Start");
  controls.push_back(start);

  status = createLabel("");
  controls.push_back(status);

  failedMessage = createLabel("");
  controls.push_back(failedMessage);

  initNavigation(controls, 1, controls.size()-3);

  // set settings
  loadSettings();
}
ServerStartMenu::~ServerStartMenu()
{
}

HUDuiDefaultKey* ServerStartMenu::getDefaultKey()
{
  return MenuDefaultKey::getInstance();
}


void ServerStartMenu::setSettings(const char* _settings)
{
  // FIXME -- temporary to automatically upgrade old configurations
  if (strlen(_settings) == 14) {
    strcpy(settings, _settings);
    settings[12] = settings[13];
    settings[13] = settings[14];
    return;
  }

  if (strlen(_settings) != strlen(settings)) return;
  strcpy(settings, _settings);
}

void ServerStartMenu::loadSettings()
{
  std::vector<HUDuiControl*>& controls = getControls();
  const char* scan = settings;
  for (int i = 1; *scan; i++, scan++)
    ((HUDuiList*)controls[i])->setIndex(*scan - 'a');
}

void ServerStartMenu::show()
{
  setStatus("");
}

void ServerStartMenu::dismiss()
{
  std::vector<HUDuiControl*>& controls = getControls();
  char* scan = settings;
  for (int i = 1; *scan; i++, scan++)
    *scan = (char)('a' + ((HUDuiList*)controls[i])->getIndex());
}

void ServerStartMenu::execute()
{
  static const char*	serverApp = "bzfs";

  std::vector<HUDuiControl*>& list = getControls();
  HUDuiControl* focus = HUDui::getFocus();
  if (focus == start) {
    // start it up:
    //   without: -p, -i, -q, -a, +f, -synctime
    //   -b if -c
    //   -h, -r if !-c
    //   +s if superflags != no
    //   -f for all bad flags if superflags == good only
    //
    // other options as they were set

    // get path to server from path to client
    extern const char* argv0;			// from bzflag.cxx
    // add 256 for flags room
    char serverCmd[PATH_MAX + 256];
    strcpy(serverCmd, argv0);
    char* base = strrchr(serverCmd, '/');
#if defined(_WIN32)
    char* base2 = strrchr(serverCmd, '\\');
    if (base2 && (!base || base2 - serverCmd > base - serverCmd))
      base = base2;
#endif
    if (!base) base = serverCmd;
    else base++;
    strcpy(base, serverApp);

    // prepare arguments for starting server
    const char* args[30];
    int arg = 0;
    args[arg++] = serverApp;

    // always try a fallback port if default port is busy
    args[arg++] = "-pf";

    // game style
    if (((HUDuiList*)list[1])->getIndex() == 0) {
      args[arg++] = "-c";
      args[arg++] = "-b";
    }
    else if (((HUDuiList*)list[1])->getIndex() == 1){
      args[arg++] = "-h";
    }
    else {
      args[arg++] = "-rabbit";
    }

    // max players
    static const char* numPlayers[] = { "2", "3", "4", "8", "20", "40" };
    args[arg++] = "-mp";
    args[arg++] = numPlayers[((HUDuiList*)list[2])->getIndex()];

    // max shots
    static const char* numShots[] = { "1", "2", "3", "4", "5" };
    args[arg++] = "-ms";
    args[arg++] = numShots[((HUDuiList*)list[3])->getIndex()];

    // teleporters
    if (((HUDuiList*)list[4])->getIndex() == 1)
      args[arg++] = "-t";

    // ricochet
    if (((HUDuiList*)list[5])->getIndex() == 1)
      args[arg++] = "+r";

    // jumping
    if (((HUDuiList*)list[6])->getIndex() == 1)
      args[arg++] = "-j";

    // superflags
    const int superflagOption = ((HUDuiList*)list[7])->getIndex();
    if (superflagOption != 0) {
      if (superflagOption == 1) {
	args[arg++] = "-f";
	args[arg++] = "bad";
      }

      args[arg++] = "+s";

      // max superflags
      static const char* numFlags[] = { "10", "20", "30", "40" };
      args[arg++] = numFlags[((HUDuiList*)list[8])->getIndex()];
    }

    // shaking
    static const char* shakingTime[] = { "", "15", "30", "60", "180" };
    static const char* shakingWins[] = { "", "1", "2", "3" };
    if (((HUDuiList*)list[9])->getIndex() == 1) args[arg++] = "-sa";
    if (((HUDuiList*)list[10])->getIndex() != 0) {
      args[arg++] = "-st";
      args[arg++] = shakingTime[((HUDuiList*)list[10])->getIndex()];
    }
    if (((HUDuiList*)list[11])->getIndex() != 0) {
      args[arg++] = "-sw";
      args[arg++] = shakingWins[((HUDuiList*)list[11])->getIndex()];
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
    if (((HUDuiList*)list[12])->getIndex() != 0) {
      args[arg++] = gameOverArg[((HUDuiList*)list[12])->getIndex()];
      args[arg++] = gameOverValue[((HUDuiList*)list[12])->getIndex()];
    }

    // server reset
    if (((HUDuiList*)list[13])->getIndex() == 0)
      args[arg++] = "-g";

    // world map file
    if (((HUDuiList*)list[14])->getIndex() != 0) { // not random
      args[arg++] = "-world";
      std::vector<std::string> fileList = ((HUDuiList*)list[14])->getList();
      std::string filename = fileList[((HUDuiList*)list[14])->getIndex()].c_str();
      args[arg++] = worldFiles[filename].c_str();
    }

    // no more arguments
    args[arg++] = NULL;

    // start the server
#if defined(_WIN32)

    // Windows
#ifdef __MINGW32__
    int result = _spawnvp(_P_DETACH, serverCmd, (char* const*) args);
#else
    int result = _spawnvp(_P_DETACH, serverCmd, args);
#endif
    if (result < 0) {
      if (errno == ENOENT)
	setStatus("Failed... can't find server program.");
      else if (errno == ENOMEM)
	setStatus("Failed... not enough memory.");
      else if (errno == ENOEXEC)
	setStatus("Failed... server program is not executable.");
      else
	setStatus("Failed... unknown error.");
    }
    else {
      setStatus("Server started.");
    }
#elif defined (macintosh)

    MacLaunchServer (arg, args);
#else /* defined(_WIN32) */

    // UNIX
    pid_t pid = fork();
    if (pid == -1) setStatus("Failed... cannot fork.");
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
      sleep(1);
      if (waitpid(pid, NULL, WNOHANG) != 0)
	setStatus("Failed.");
      else
	setStatus("Server started.");
    }

#endif /* defined(_WIN32) */
  }
}

void ServerStartMenu::setStatus(const char* msg, const std::vector<std::string> *parms)
{
  status->setString(msg, parms);
  const OpenGLTexFont& font = status->getFont();
  const float width = font.getWidth(status->getString());
  status->setPosition(center - 0.5f * width, status->getY());
}

void ServerStartMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;
  const float bigFontWidth = (float)height / 24.0f;
  const float bigFontHeight = (float)height / 24.0f;
  const float fontWidth = (float)height / 36.0f;
  const float fontHeight = (float)height / 36.0f;
  center = 0.5f * (float)width;

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontWidth, titleFontHeight);
  const OpenGLTexFont& titleFont = title->getFont();
  const float titleWidth = titleFont.getWidth(title->getString());
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleFont.getHeight();
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * (float)width;
  y -= 0.6f * titleFont.getHeight();
  list[1]->setFontSize(fontWidth, fontHeight);
  const float h = list[1]->getFont().getHeight();
  const int count = list.size();
  for (int i = 1; i < count; i++) {
    if (list[i] == start) {
      y -= bigFontHeight;
      list[i]->setFontSize(bigFontWidth, bigFontHeight);
      list[i]->setPosition(x, y);
    }
    else {
      list[i]->setFontSize(fontWidth, fontHeight);
      list[i]->setPosition(x, y);
    }
    y -= 1.0f * h;
  }
}

HUDuiList* ServerStartMenu::createList(const char* str)
{
  HUDuiList* list = new HUDuiList;
  list->setFont(MainMenu::getFont());
  if (str) list->setLabel(str);
  return list;
}

HUDuiLabel* ServerStartMenu::createLabel(const char* str)
{
  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
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
