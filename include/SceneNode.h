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

#ifndef BZF_SCENE_NODE_H
#define BZF_SCENE_NODE_H

#include "common.h"
#include "BzfString.h"
#include <assert.h>
#include <vector>

class SceneVisitor;

class SceneNode {
public:
	SceneNode() : refCount(1) { }

	int					ref();
	int					unref();

	void				setID(const BzfString& _id) { id = _id; }
	BzfString			getID() const { return id; }

	virtual bool		visit(SceneVisitor*) = 0;

protected:
	virtual ~SceneNode() { }

private:
	int					refCount;
	BzfString			id;
};

class SceneNodeField {
public:
	SceneNodeField(const char* _name) : name(_name) { }
	~SceneNodeField() { }

	const char*			getName() const { return name; }

private:
	const char*			name;
};

template <class T>
class SceneNodeScalarField : public SceneNodeField {
public:
	SceneNodeScalarField(const char* name) : SceneNodeField(name) { }
	SceneNodeScalarField(const char* name, const T& _value) :
							SceneNodeField(name), value(_value) { }
	~SceneNodeScalarField() { }

	void				set(const T& _value)
							{ value = _value; }

	T					get() const
							{ return value; }

private:
	T					value;
};

template <class T>
class SceneNodeVectorField : public SceneNodeField {
public:
	typedef std::vector<T> Values;

	SceneNodeVectorField(const char* name, unsigned int _minNum,
							unsigned int _maxNum,
							unsigned int _multNum,
							const char* _interpolationParameter = "t") :
							SceneNodeField(name),
							dirty(false),
							dirtyPtr(&dirty),
							minNum(_minNum),
							maxNum(_maxNum == 0 ? 0xffffffff : _maxNum),
							multNum(_multNum),
							interpolationParameter(_interpolationParameter)
							{ }
	~SceneNodeVectorField() { }

	void				setInterpolationParameter(const BzfString& param)
							{ interpolationParameter = param; }
	void				setDirtyFlag(bool* flag)
							{ *flag = *dirtyPtr; dirtyPtr = flag; }
	void				set(const SceneNodeVectorField<T>& v)
							{ set(v.values); }
	void				set(const Values& v);
	void				set(const T* v, unsigned int num);
	void				set(unsigned int index, const T& value);
	void				swap(SceneNodeVectorField<T>& v);
	void				swap(Values& v);
	void				reserve(unsigned int num)
							{ values.reserve(num); }

	// these do not verify the resulting element count
	void				clear() { values.clear(); setDirty(); }
	void				resize(unsigned int num)
							{ values.resize(num); setDirty(); }
	void				push(const T& value)
							{ values.push_back(value);
								  setDirty(); }
	void				push(const T& v1, const T& v2)
							{ values.push_back(v1); values.push_back(v2);
							  setDirty(); }
	void				push(const T& v1, const T& v2, const T& v3)
							{ values.push_back(v1); values.push_back(v2);
							  values.push_back(v3);
							  setDirty(); }
	void				push(const T& v1, const T& v2, const T& v3, const T& v4)
							{ values.push_back(v1); values.push_back(v2);
							  values.push_back(v3); values.push_back(v4);
							  setDirty(); }

	void				clearDirty()
							{ *dirtyPtr = false; }

	const T*			get() const
							{ return &values[0]; }
	T					get(int index) const
							{ return values[index]; }
	unsigned int		getNum() const
							{ return values.size(); }
	unsigned int		getMinNum() const
							{ return minNum; }
	unsigned int		getMaxNum() const
							{ return maxNum; }
	unsigned int		getMultNum() const
							{ return multNum; }
	const BzfString&	getInterpolationParameter() const
							{ return interpolationParameter; }

protected:
	void				setDirty()
							{ *dirtyPtr = true; }

private:
	bool				dirty;
	bool*				dirtyPtr;
	unsigned int		minNum;
	unsigned int		maxNum;
	unsigned int		multNum;
	BzfString			interpolationParameter;
	Values				values;
};

template <class T>
void					SceneNodeVectorField<T>::set(const Values& v)
{
	assert(v.size() >= minNum && v.size() <= maxNum);
	assert(v.size() % multNum == 0);

	values = v;
	setDirty();
}

template <class T>
void					SceneNodeVectorField<T>::set(
							const T* v, unsigned int num)
{
	assert(v != NULL || num == 0);
	assert(num >= minNum && num <= maxNum);
	assert(num % multNum == 0);

	values.clear();
	values.reserve(num);
	for (; num > 0; --num)
		values.push_back(*v++);
	setDirty();
}

template <class T>
void					SceneNodeVectorField<T>::set(
							unsigned int index, const T& value)
{
	assert(index < values.size());

	values[index] = value;
	setDirty();
}

template <class T>
void					SceneNodeVectorField<T>::swap(
							SceneNodeVectorField<T>& v)
{
	assert(values.size() >= v.minNum && values.size() <= v.maxNum);
	assert(values.size() % v.multNum == 0);

	swap(v.values);
	setDirty();
}

template <class T>
void					SceneNodeVectorField<T>::swap(Values& v)
{
	assert(v.size() >= minNum && v.size() <= maxNum);
	assert(v.size() % multNum == 0);

	values.swap(v);
	setDirty();
}

typedef SceneNodeScalarField<float> SceneNodeSFFloat;
typedef SceneNodeScalarField<bool> SceneNodeSFBool;
typedef SceneNodeScalarField<unsigned int> SceneNodeSFUInt;
typedef SceneNodeScalarField<BzfString> SceneNodeSFString;
typedef SceneNodeVectorField<float> SceneNodeVFFloat;
typedef SceneNodeVectorField<unsigned int> SceneNodeVFUInt;
typedef SceneNodeVectorField<BzfString> SceneNodeVFString;

class SceneNodeSFEnum : public SceneNodeScalarField<unsigned int> {
public:
	SceneNodeSFEnum(const char* name, const char** _enums,
							unsigned int _numEnums) :
							SceneNodeScalarField<unsigned int>(name),
							enums(_enums),
							numEnums(_numEnums) { }
	SceneNodeSFEnum(const char* name, const int& value,
							const char** _enums,
							unsigned int _numEnums) :
							SceneNodeScalarField<unsigned int>(name, value),
							enums(_enums),
							numEnums(_numEnums) { }
	~SceneNodeSFEnum() { }

	unsigned int		getNumEnums() const
							{ return numEnums; }
	const char*			getEnum(unsigned int index) const
							{ return enums[index]; }

private:
	const char**		enums;
	unsigned int		numEnums;
};

#endif
