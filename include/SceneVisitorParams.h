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

#ifndef BZF_SCENE_VISITOR_PARAMS_H
#define BZF_SCENE_VISITOR_PARAMS_H

#ifdef WIN32
#pragma warning(4:4786)
#endif

#include <string>
#include <map>
#include <vector>

class SceneVisitorParams {
public:
	SceneVisitorParams();
	~SceneVisitorParams();

	void				setInt(const std::string& name, int value);
	void				pushInt(const std::string& name, int value);
	void				popInt(const std::string& name);
	int					getInt(const std::string& name) const;
	bool				isInt(const std::string& name) const;

	void				setFloat(const std::string& name, float value);
	void				pushFloat(const std::string& name, float value);
	void				popFloat(const std::string& name);
	float				getFloat(const std::string& name) const;
	bool				isFloat(const std::string& name) const;

private:
	typedef std::vector<int> IntParameter;
	typedef std::vector<float> FloatParameter;
	typedef std::map<std::string, IntParameter> IntParamMap;
	typedef std::map<std::string, FloatParameter> FloatParamMap;

	IntParamMap			intParams;
	FloatParamMap		floatParams;
};

#endif
// ex: shiftwidth=4 tabstop=4
