/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(WIN32)
#pragma warning(4:4503)
#endif

#include "StateDatabase.h"
#include "ErrorHandler.h"
#include <assert.h>
#include <ctype.h>

//
// StateDatabase::Item
//

StateDatabase::Item::Item() : value(),
								defValue(),
								isSet(false),
								isTrue(false),
								save(true), // FIXME -- false by default?
								permission(ReadWrite)
{
	// do nothing
}


//
// StateDatabase
//

StateDatabase*			StateDatabase::s_instance = NULL;

StateDatabase::StateDatabase()
{
	// do nothing
}

StateDatabase::~StateDatabase()
{
	s_instance = NULL;
}

void					StateDatabase::set(const std::string& name,
								const std::string& value,
								Permission access)
{
	Map::iterator index = lookup(name);
	if (access >= index->second.permission) {
		index->second.value  = value;
		index->second.isSet  = true;
		index->second.isTrue = (index->second.value != "0" &&
						    index->second.value != "false" &&
						    index->second.value != "false" &&
						    index->second.value != "FALSE" &&
						    index->second.value != "no" &&
						    index->second.value != "No" &&
						    index->second.value != "NO");
		notify(index);
	}
}

void					StateDatabase::unset(const std::string& name,
								Permission access)
{
	Map::iterator index = lookup(name);
	if (access >= index->second.permission) {
		index->second.value  = "";
		index->second.isSet  = false;
		index->second.isTrue = false;
		notify(index);
	}
}

void					StateDatabase::touch(const std::string& name,
								Permission access)
{
	Map::iterator index = lookup(name);
	if (access >= index->second.permission)
		notify(index);
}

void					StateDatabase::setPersistent(
								const std::string& name, bool save)
{
	Map::iterator index = lookup(name);
	index->second.save = save;
}

void					StateDatabase::setDefault(
								const std::string& name, const std::string& value)
{
	Map::iterator index = lookup(name);
	index->second.defValue = value;
}

void					StateDatabase::setPermission(
								const std::string& name,
								Permission permission)
{
	Map::iterator index = lookup(name);
	index->second.permission = permission;
}

void					StateDatabase::addCallback(
								const std::string& name,
								Callback callback,
								void* userData)
{
	Map::iterator index = lookup(name);
	index->second.callbacks.add(callback, userData);
}

void					StateDatabase::removeCallback(
								const std::string& name,
								Callback callback,
								void* userData)
{
	Map::iterator index = lookup(name);
	index->second.callbacks.remove(callback, userData);
}

bool					StateDatabase::isSet(const std::string& name) const
{
	Map::const_iterator index = items.find(name);
	return !(index == items.end() || !index->second.isSet);
}

std::string				StateDatabase::get(const std::string& name) const
{
	Map::const_iterator index = items.find(name);
	if (index == items.end() || !index->second.isSet)
		return std::string();
	else
		return index->second.value;
}

bool					StateDatabase::isTrue(const std::string& name) const
{
	Map::const_iterator index = items.find(name);
	return !(index == items.end() || !index->second.isTrue);
}

bool					StateDatabase::isEmpty(const std::string& name) const
{
	Map::const_iterator index = items.find(name);
	return (index == items.end() || !index->second.isSet ||
								index->second.value.empty());
}

bool					StateDatabase::isPersistent(const std::string& name) const
{
	Map::const_iterator index = items.find(name);
	return (index != items.end() && index->second.save);
}

std::string				StateDatabase::getDefault(const std::string& name) const
{
	Map::const_iterator index = items.find(name);
	if (index != items.end())
		return index->second.defValue;
	else
		return "";
}

StateDatabase::Permission
						StateDatabase::getPermission(const std::string& name) const
{
	Map::const_iterator index = items.find(name);
	if (index != items.end())
		return index->second.permission;
	else
		return ReadWrite;
}

StateDatabase::Map::iterator
						StateDatabase::lookup(const std::string& name)
{
	Map::iterator index = items.find(name);
	if (index == items.end()) {
		Item tmp;
		return items.insert(std::make_pair(name, tmp)).first;
	}
	else {
		return index;
	}
}

void					StateDatabase::notify(Map::iterator index)
{
	index->second.callbacks.iterate(&onCallback,
								const_cast<void*>(
								reinterpret_cast<const void*>(&index->first)));
}

bool					StateDatabase::onCallback(
								Callback callback,
								void* userData,
								void* iterateData)
{
	callback(*reinterpret_cast<std::string*>(iterateData), userData);
	return true;
}

void					StateDatabase::iterate(
								Callback callback, void* userData) const
{
	assert(callback != NULL);

	for (Map::const_iterator index = items.begin();
								index != items.end(); ++index) {
		if (index->second.isSet) {
			(*callback)(index->first, userData);
		}
	}
}

void					StateDatabase::write(
								Callback callback, void* userData) const
{
	assert(callback != NULL);

	for (Map::const_iterator index = items.begin();
								index != items.end(); ++index) {
		if (index->second.isSet && index->second.save &&
		index->second.value != index->second.defValue) {
			(*callback)(index->first, userData);
		}
	}
}

StateDatabase*			StateDatabase::getInstance()
{
	if (s_instance == NULL)
		s_instance = new StateDatabase;
	return s_instance;
}

// ex: shiftwidth=4 tabstop=4
