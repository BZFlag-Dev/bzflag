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

#ifndef BZF_BODY_MANAGER_H
#define BZF_BODY_MANAGER_H

#include "common.h"
#include <vector>

#define BODYMGR (BodyManager::getInstance())

class Body;

class BodyManager {
public:
	~BodyManager();

	// maintain list of obstacles (add, remove, remove all)
	//   need flag for updatable
	//   detect and flag immovable (zero mass^-1 and inertia^-1)
	// perform time step (collision detection and response)
	//   only non-immovable objects are checked for collisions
	//   only update updatable objects
	//   should report info on collisions
	//     maybe just existence
	//     maybe velocity/acceleration, possible direction, too
	//     can use for sound feedback

	void				add(Body* adopted);
	void				remove(Body*);
	void				removeAll();

	typedef std::vector<Body*> BodyList;
	typedef BodyList::iterator iterator;
	typedef BodyList::const_iterator const_iterator;

	const_iterator		begin() const { return bodies.begin(); }
	const_iterator		end() const { return bodies.end(); }

	static BodyManager*	getInstance();

private:
	BodyManager();

private:
	BodyList			bodies;

	static BodyManager*	mgr;
};

#endif
