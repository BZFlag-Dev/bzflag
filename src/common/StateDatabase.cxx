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

void					StateDatabase::set(const BzfString& name,
								const BzfString& value,
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

void					StateDatabase::unset(const BzfString& name,
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

void					StateDatabase::touch(const BzfString& name,
								Permission access)
{
	Map::iterator index = lookup(name);
	if (access >= index->second.permission)
		notify(index);
}

void					StateDatabase::setPersistent(
								const BzfString& name, bool save)
{
	Map::iterator index = lookup(name);
	index->second.save = save;
}

void					StateDatabase::setDefault(
								const BzfString& name, const BzfString& value)
{
	Map::iterator index = lookup(name);
	index->second.defValue = value;
}

void					StateDatabase::setPermission(
								const BzfString& name,
								Permission permission)
{
	Map::iterator index = lookup(name);
	index->second.permission = permission;
}

void					StateDatabase::addCallback(
								const BzfString& name,
								Callback callback,
								void* userData)
{
	Map::iterator index = lookup(name);
	index->second.callbacks.add(callback, userData);
}

void					StateDatabase::removeCallback(
								const BzfString& name,
								Callback callback,
								void* userData)
{
	Map::iterator index = lookup(name);
	index->second.callbacks.remove(callback, userData);
}

bool					StateDatabase::isSet(const BzfString& name) const
{
	Map::const_iterator index = items.find(name);
	return !(index == items.end() || !index->second.isSet);
}

BzfString				StateDatabase::get(const BzfString& name) const
{
	Map::const_iterator index = items.find(name);
	if (index == items.end() || !index->second.isSet)
		return BzfString();
	else
		return index->second.value;
}

bool					StateDatabase::isTrue(const BzfString& name) const
{
	Map::const_iterator index = items.find(name);
	return !(index == items.end() || !index->second.isTrue);
}

bool					StateDatabase::isEmpty(const BzfString& name) const
{
	Map::const_iterator index = items.find(name);
	return (index == items.end() || !index->second.isSet ||
								index->second.value.empty());
}

bool					StateDatabase::isPersistent(const BzfString& name) const
{
	Map::const_iterator index = items.find(name);
	return (index != items.end() && index->second.save);
}

BzfString				StateDatabase::getDefault(const BzfString& name) const
{
	Map::const_iterator index = items.find(name);
	if (index != items.end())
		return index->second.defValue;
	else
		return "";
}

StateDatabase::Permission
						StateDatabase::getPermission(const BzfString& name) const
{
	Map::const_iterator index = items.find(name);
	if (index != items.end())
		return index->second.permission;
	else
		return ReadWrite;
}

StateDatabase::Map::iterator
						StateDatabase::lookup(const BzfString& name)
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
	callback(*reinterpret_cast<BzfString*>(iterateData), userData);
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

