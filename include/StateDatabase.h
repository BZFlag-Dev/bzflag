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

#ifndef BZF_STATE_DATABASE_H
#define BZF_STATE_DATABASE_H

#include "common.h"
#include "BzfString.h"
#include "CallbackList.h"
#include "bzfio.h"

#define BZDB (StateDatabase::getInstance())

class StateDatabase {
public:
	typedef void (*Callback)(const BzfString& name, void* userData);
	enum Permission {
		// permission levels
		ReadWrite,
		Locked,
		ReadOnly,

		// access levels
		User   = ReadWrite,
		Server = Locked,
		Client = ReadOnly
	};

	~StateDatabase();

	// set a name/value pair.  if access is less than the permission
	// level of the name then this has no effect.
	void				set(const BzfString& name, const BzfString& value,
							Permission access = Client);

	// unset a name if access is not less than the permission level
	// of the name.
	void				unset(const BzfString& name,
							Permission access = Client);

	// simulate a change to a value (i.e. invoke the callbacks on it)
	void				touch(const BzfString& name,
							Permission access = Client);

	// mark a value as persistent (i.e. to be saved) or volatile.
	// this state is stored independently of the existance of a value
	// with the given name.  that is, adding or removing the name
	// will not affect persistence of the name.  the default is
	// volatile.
	void				setPersistent(const BzfString& name, bool = true);

	// set the default value for a name.  if the default value is set
	// then the value will not be written by write() if the current
	// value is equal to the default value.
	void				setDefault(const BzfString& name,
							const BzfString& value);

	// set the permission level of a name.  like persistence, this is
	// stored independently of a value with the name.  the default
	// permission is ReadWrite (i.e. full access).
	void				setPermission(const BzfString& name, Permission);

	// add/remove a callback to/from a name.  all callbacks on a name are
	// invoked when the value changes (either by being set or unset).
	// each name can have any number of callbacks but any given callback
	// function/userData pair on a name can only be registered once (i.e.
	// multiple adds have the same effect as a single add).
	void				addCallback(const BzfString& name,
							Callback, void* userData);
	void				removeCallback(const BzfString& name,
							Callback, void* userData);

	// test if a name is set or not
	bool				isSet(const BzfString& name) const;

	// get the value associated with a name.  returns the empty string
	// if the name isn't set.
	BzfString			get(const BzfString& name) const;

	// return true if the value associated with a name indicates
	// logical true, which is when the value is not empty and not
	// "0" and not "false" and not "no".
	bool				isTrue(const BzfString& name) const;

	// test if a name is empty or not.  a name is empty if it's
	// not set or it's set to the empty string.
	bool				isEmpty(const BzfString& name) const;

	// get the persistence, permission, and default for an entry
	bool				isPersistent(const BzfString& name) const;
	BzfString			getDefault(const BzfString& name) const;
	Permission			getPermission(const BzfString& name) const;

	// invoke the callback for each entry
	void				iterate(Callback, void* userData) const;

	// invoke the callback for each entry that should be written (i.e.
	// is set, persistent, and not the default).
	void				write(Callback, void* userData) const;

	// get the singleton instance of the state database
	static StateDatabase* getInstance();

private:
	StateDatabase();

	static bool			onCallback(Callback, void* userData, void* iterateData);

	struct Item {
	public:
		Item();

	public:
		BzfString		value;
		BzfString		defValue;
		bool			isSet;
		bool			isTrue;
		bool			save;
		Permission		permission;
		CallbackList<Callback>	callbacks;
	};
	typedef std::map<BzfString, Item> Map;

	Map::iterator		lookup(const BzfString&);
	void				notify(Map::iterator);

private:
	Map					items;
	static StateDatabase* s_instance;
};

#endif // BZF_STATE_DATABASE_H
