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

#ifndef BZF_SCENE_VISITOR_PARAMS_H
#define BZF_SCENE_VISITOR_PARAMS_H

#include "BzfString.h"
#include <map>
#include <vector>

class SceneVisitorParams {
public:
	SceneVisitorParams();
	~SceneVisitorParams();

	void				setInt(const BzfString& name, int value);
	void				pushInt(const BzfString& name, int value);
	void				popInt(const BzfString& name);
	int					getInt(const BzfString& name) const;
	bool				isInt(const BzfString& name) const;

	void				setFloat(const BzfString& name, float value);
	void				pushFloat(const BzfString& name, float value);
	void				popFloat(const BzfString& name);
	float				getFloat(const BzfString& name) const;
	bool				isFloat(const BzfString& name) const;

private:
	typedef std::vector<int> IntParameter;
	typedef std::vector<float> FloatParameter;
	typedef std::map<BzfString, IntParameter> IntParamMap;
	typedef std::map<BzfString, FloatParameter> FloatParamMap;

	IntParamMap			intParams;
	FloatParamMap		floatParams;
};

#endif
