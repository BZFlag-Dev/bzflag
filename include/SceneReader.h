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

#ifndef BZF_SCENE_READER_H
#define BZF_SCENE_READER_H

#include "ConfigIO.h"
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
	typedef ConfigReader::Values Values;

	SceneReader();
	~SceneReader();

	// read a scene
	SceneNode*			read(istream&);

	// get the position in the stream (usually for error reporting)
	BzfString			getPosition() const;

private:
	bool				open(ConfigReader*, const BzfString&,
							const ConfigReader::Values&);
	bool				close(ConfigReader*, const BzfString&);

	bool				openField(ConfigReader*, const BzfString&,
							const ConfigReader::Values&);
	bool				closeField(ConfigReader*, const BzfString&);
	bool				dataField(ConfigReader*, const BzfString&);

	bool				pushCommon(ConfigReader* reader,
							const ConfigReader::Values& values,
							SceneNode* newNode);
	bool				push(ConfigReader* reader,
							const ConfigReader::Values& values,
							SceneNode* newNode);
	bool				push(ConfigReader* reader,
							const ConfigReader::Values& values,
							SceneNodeGroup* newNode);
	bool				pop(ConfigReader* reader);
	void				readEnum(ConfigReader* reader,
							const ConfigReader::Values& values,
							SceneNodeSFEnum& field);
	void				addReader(SceneNodeFieldReader* reader);

	static bool			openCB(ConfigReader*, const BzfString&, const ConfigReader::Values&, void*);
	static bool			closeCB(ConfigReader*, const BzfString&, void*);

	static bool			openFieldCB(ConfigReader*, const BzfString&, const ConfigReader::Values&, void*);
	static bool			closeFieldCB(ConfigReader*, const BzfString&, void*);
	static bool			dataFieldCB(ConfigReader*, const BzfString&, void*);

private:
	typedef std::map<BzfString, SceneNodeFieldReader*> FieldReaders;
	struct State {
	public:
		SceneNode*				node;
		SceneNodeGroup*			group;
		FieldReaders			fieldReaders;
		SceneNodeFieldReader*	activeField;
	};

	typedef std::vector<State> Stack;
	typedef std::map<BzfString, SceneNode*> NodeMap;

	ConfigReader*		configReader;
	Stack				stack;
	SceneNode*			node;
	NodeMap				namedNodes;
};

#endif
