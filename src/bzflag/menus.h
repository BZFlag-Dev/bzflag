/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * OptionDialog:
 *	A single HUD dialog of HUDuiControls.
 */

#ifndef	BZF_MENUS_H
#define	BZF_MENUS_H

#ifdef _WIN32
#pragma warning( 4 : 4786 )
#endif

#include <string>
#include "common.h"
#include "HUDDialog.h"
#include "OpenGLTexFont.h"
#include "Ping.h"

class JoinMenu;
class OptionsMenu;
class QuitMenu;

class MenuDefaultKey : public HUDuiDefaultKey {
  public:
			MenuDefaultKey();
			~MenuDefaultKey();

    bool		keyPress(const BzfKeyEvent&);
    bool		keyRelease(const BzfKeyEvent&);

    static MenuDefaultKey* getInstance() { return &instance; }

  private:
    static MenuDefaultKey instance;
};

static const int       MaxListServers = 5;
class ListServer {
  public:
    Address		address;
    int			port;
    int			socket;
    int			phase;
    int			bufferSize;
    char		buffer[1024];
};

class MainMenu : public HUDDialog {
  public:
			MainMenu();
			~MainMenu();

    HUDuiDefaultKey*	getDefaultKey();
    void		execute();
    void		resize(int width, int height);

    static const OpenGLTexFont& getFont();

  private:
    OpenGLTexFont	font;
    OpenGLTexture	title;
    JoinMenu*		joinMenu;
    OptionsMenu*	optionsMenu;
    QuitMenu*		quitMenu;
    static OpenGLTexFont* mainFont;
};

class ServerStartMenu : public HUDDialog {
  public:
			ServerStartMenu();
			~ServerStartMenu() { }

    HUDuiDefaultKey*	getDefaultKey()
				{ return MenuDefaultKey::getInstance(); }
    void		execute();
    void		show();
    void		dismiss();
    void		resize(int width, int height);

    static const char*	getSettings() { return settings; }
    static void		setSettings(const char*);

  private:
    HUDuiList*		createList(const char*);
    HUDuiLabel*		createLabel(const char*);
    void		setStatus(const char*, const std::vector<std::string> *parms = NULL);
    void		loadSettings();

  private:
    float		center;
    HUDuiLabel*		status;
    HUDuiLabel*		failedMessage;
    static char		settings[];
};

class ServerItem {
  public:
    void		writeToFile(ofstream& out) const; // serialize out
    bool		readFromFile(ifstream& in); // serialize in
    void		setUpdateTime(); // set last updated to now
    int			getPlayerCount() const;
    time_t		getAgeMinutes() const;
    time_t		getAgeSeconds() const;
    std::string		getAgeString() const; // nifty formated age string
    time_t		getNow() const; // current time
    bool		operator<(const ServerItem &right);
  public:
    std::string		name;
    std::string		description;
    PingPacket		ping;
    bool		cached; // was I cached ?
    time_t		updateTime; // last time I was updated
};

class ServerMenu;

class ServerMenuDefaultKey : public MenuDefaultKey {
  public:
			ServerMenuDefaultKey(ServerMenu* _menu) :
				menu(_menu) { }
			~ServerMenuDefaultKey() { }

    bool		keyPress(const BzfKeyEvent&);
    bool		keyRelease(const BzfKeyEvent&);

  private:
    ServerMenu*		menu;
};

typedef std::map<std::string, ServerItem> SRV_STR_MAP;

class ServerMenu : public HUDDialog {
  public:
			ServerMenu();
			~ServerMenu() { }

    HUDuiDefaultKey*	getDefaultKey() { return &defaultKey; }
    int			getSelected() const;
    void		setSelected(int);
    void		show();
    void		execute();
    void		dismiss();
    void		resize(int width, int height);
    static void		saveCache();
    static void		loadCache();
    static void		setMaxCacheAge(time_t time);
    static time_t	getMaxCacheAge();
    static void		clearCache();


  public:
    static const int	NumItems;

  private:
    void		addLabel(const char* string, const char* label);
    void		checkEchos();
    void		readServerList(int index);
    void		addToList(ServerItem&, bool doCache=false);
    void		addToListWithLookup(ServerItem&);
    void		setStatus(const char*, const std::vector<std::string> *parms = NULL);
    void		pick();
    static void		playingCB(void*);
    void		addCacheToList();
    ServerItem&		serversAt(int index);

  private:
    ServerMenuDefaultKey	defaultKey;
    std::vector<ServerItem>	servers;
    int				pingInSocket;
    struct sockaddr_in		pingInAddr;
    int				pingBcastSocket;
    struct sockaddr_in		pingBcastAddr;
    HUDuiLabel*			status;

    HUDuiLabel*			pageLabel;
    int				selectedIndex;

    int				phase;
    ListServer			listServers[MaxListServers];
    int				numListServers;

    static const int	NumReadouts;
    static SRV_STR_MAP	serverCache;
    static time_t	maxCacheAge; // age after we don't show servers in cache
    bool		addedCacheToList; // have we already added cache to list
    static int		cacheAddedNum; // how many items were added to cache
};

#endif /* BZF_MENUS_H */
// ex: shiftwidth=2 tabstop=8
