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

#ifndef BZF_SCENE_READER_H
#define BZF_SCENE_READER_H

#include "XMLTree.h"
#include <map>
#include <vector>

class SceneNode;
class SceneNodeFieldReader;
class SceneNodeGroup;
class SceneNodeSFEnum;

//
// object to parse a view configuration
//

class SceneReader {
public:
	SceneReader();
	~SceneReader();

	// read a scene.  returned node is ref()'d.
	SceneNode*			parse(XMLTree::iterator node);

private:
	void				parseNode(XMLTree::iterator xml);
	void				pushCommon(XMLTree::iterator, SceneNode* newNode);
	void				push(XMLTree::iterator, SceneNode* newNode);
	void				push(XMLTree::iterator, SceneNodeGroup* newNode);
	void				pop();
	void				readEnum(XMLTree::iterator, SceneNodeSFEnum& field);
	void				addReader(SceneNodeFieldReader* reader);
	void				saveNamedNode(const std::string& id);

private:
	typedef std::map<std::string, SceneNodeFieldReader*> FieldReaders;
	struct State {
	public:
		SceneNode*				node;
		SceneNodeGroup*			group;
		FieldReaders			fieldReaders;
		SceneNodeFieldReader*	activeField;
	};

	typedef std::vector<State> Stack;
	typedef std::map<std::string, SceneNode*> NodeMap;

	Stack				stack;
	SceneNode*			node;
	NodeMap				namedNodes;
};

#endif
