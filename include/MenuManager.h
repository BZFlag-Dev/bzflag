/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_MENU_MANAGER_H
#define BZF_MENU_MANAGER_H

#include "common.h"
#include "BzfString.h"
#include <map>
#include <vector>

#define MENUMGR (MenuManager::getInstance())

class Menu;

class MenuManager {
public:
	~MenuManager();

	void				insert(const BzfString& name, Menu* menu);

	bool				push(const BzfString& name);
	void				pop();

	Menu*				top() const;

	static MenuManager*	getInstance();

private:
	MenuManager();

private:
	typedef std::vector<Menu*> MenuStack;
	typedef std::map<BzfString, Menu*> Menus;

	MenuStack			stack;
	Menus				menus;
	static MenuManager*	mgr;
};

#endif
