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

#include "MenuManager.h"
#include "Menu.h"
#include "CommandManager.h"
#include "ErrorHandler.h"

//
// MenuManager
//

MenuManager*			MenuManager::mgr = NULL;

MenuManager::MenuManager()
{
	// do nothing
}

MenuManager::~MenuManager()
{
	// clean up
	for (Menus::iterator index = menus.begin(); index != menus.end(); ++index)
		delete index->second;
	mgr = NULL;
}

void					MenuManager::insert(const BzfString& name, Menu* menu)
{
	Menus::iterator index = menus.find(name);
	if (index != menus.end()) {
		delete index->second;
		menus.erase(index);
	}
	menus.insert(std::make_pair(name, menu));
}

bool					MenuManager::push(const BzfString& name)
{
	Menus::const_iterator index = menus.find(name);
	if (index == menus.end()) {
		return false;
	}
	else {
		// put menu on the stack and notify it
		Menu* menu = index->second;
		stack.push_back(menu);
		menu->open();
		CMDMGR->run(menu->getOpenCommand());
		return true;
	}
}

void					MenuManager::pop()
{
	assert(!stack.empty());

	// remove menu
	Menu* menu = stack.back();
	CMDMGR->run(menu->getCloseCommand());
	menu->close();
	stack.pop_back();
}

Menu*					MenuManager::top() const
{
	if (stack.empty())
		return NULL;
	else
		return stack.back();
}

MenuManager*			MenuManager::getInstance()
{
	if (mgr == NULL)
		mgr = new MenuManager;
	return mgr;
}
