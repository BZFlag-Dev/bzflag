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

#include "SceneVisitorParams.h"

//
// SceneVisitorParams
//

SceneVisitorParams::SceneVisitorParams()
{
	// do nothing
}

SceneVisitorParams::~SceneVisitorParams()
{
	// do nothing
}

void					SceneVisitorParams::setInt(
								const std::string& name, int value)
{
	IntParameter& list = intParams[name];
	list[list.size() - 1] = value;
}

void					SceneVisitorParams::pushInt(
								const std::string& name, int value)
{
	intParams[name].push_back(value);
}

void					SceneVisitorParams::popInt(const std::string& name)
{
	intParams[name].pop_back();
}

int						SceneVisitorParams::getInt(const std::string& name) const
{
	IntParamMap::const_iterator index = intParams.find(name);
	if (index == intParams.end())
		return 0;
	else
		return index->second.back();
}

bool					SceneVisitorParams::isInt(const std::string& name) const
{
	return (intParams.find(name) != intParams.end());
}

void					SceneVisitorParams::setFloat(
								const std::string& name, float value)
{
	FloatParameter& list = floatParams[name];
	list[list.size() - 1] = value;
}

void					SceneVisitorParams::pushFloat(
								const std::string& name, float value)
{
	floatParams[name].push_back(value);
}

void					SceneVisitorParams::popFloat(const std::string& name)
{
	floatParams[name].pop_back();
}

float					SceneVisitorParams::getFloat(
								const std::string& name) const
{
	FloatParamMap::const_iterator index = floatParams.find(name);
	if (index == floatParams.end())
		return 0.0f;
	else
		return index->second.back();
}

bool					SceneVisitorParams::isFloat(const std::string& name) const
{
	return (floatParams.find(name) != floatParams.end());
}
