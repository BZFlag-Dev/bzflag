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

#include "SceneNode.h"
#include <assert.h>

//
// SceneNode
//

int						SceneNode::ref()
{
	assert(refCount >= 1);
	return ++refCount;
}

int						SceneNode::unref()
{
	assert(refCount >= 1);
	int n = --refCount;
	if (n == 0) {
		refCount = 0xdeadbeef;
		delete this;
	}
	return n;
}
// ex: shiftwidth=4 tabstop=4
