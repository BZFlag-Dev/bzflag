/* bzflag
 * Copyright (c) 1993 - 2000 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
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

#include "HUDDialog.h"
#include "OpenGLTexFont.h"

class JoinMenu;
class OptionsMenu;
class QuitMenu;

class MenuDefaultKey : public HUDuiDefaultKey {
  public:
			MenuDefaultKey();
			~MenuDefaultKey();

    boolean		keyPress(const BzfKeyEvent&);
    boolean		keyRelease(const BzfKeyEvent&);

    static MenuDefaultKey* getInstance() { return &instance; }

  private:
    static MenuDefaultKey instance;
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
    void		setStatus(const char*);
    void		loadSettings();

  private:
    float		center;
    HUDuiLabel*		status;
    HUDuiLabel*		failedMessage;
    static char		settings[];
};

#endif /* BZF_MENUS_H */
