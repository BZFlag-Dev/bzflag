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

#include "SceneReader.h"
#include "SceneNodes.h"
#include "OpenGLTexture.h"
#include "ErrorHandler.h"
#include <ctype.h>
#include <stdlib.h>


//
// SceneNodeFieldReader
//

class SceneNodeFieldReader {
	public:
		SceneNodeFieldReader() { }
		virtual ~SceneNodeFieldReader() { }

		virtual bool		open(ConfigReader* reader,
								const BzfString& tag,
								const ConfigReader::Values& values) = 0;
		virtual bool		close(ConfigReader* reader) = 0;
		virtual bool		read(ConfigReader* reader, const BzfString& data) = 0;
		virtual const char*	getName() const = 0;
};

//
// SceneNodeScalarReader<T>
//

template <class T>
class SceneNodeScalarReader : public SceneNodeFieldReader {
	public:
		SceneNodeScalarReader(SceneNodeScalarField<T>*);
		virtual ~SceneNodeScalarReader() { }

		virtual bool		open(ConfigReader* reader,
								const BzfString& tag,
								const ConfigReader::Values& values);
		virtual bool		close(ConfigReader* reader);
		virtual bool		read(ConfigReader* reader, const BzfString& data);
		virtual const char*	getName() const;

	private:
		T					parse(const char*, char**) const;

	private:
		bool				done;
		SceneNodeScalarField<T>*	field;
};

template <class T>
SceneNodeScalarReader<T>::SceneNodeScalarReader(
								SceneNodeScalarField<T>* _field) :
								field(_field)
{
	// do nothing
}

template <class T>
bool					SceneNodeScalarReader<T>::open(ConfigReader*,
								const BzfString&,
								const ConfigReader::Values&)
{
	done = false;
	return true;
}

template <class T>
bool					SceneNodeScalarReader<T>::close(ConfigReader* reader)
{
	// make sure we satisfy field constraints
	if (!done) {
		printError("%s: expected a value in %s",
								reader->getPosition().c_str(),
								field->getName());
		return false;
	}

	return true;
}

template <class T>
bool					SceneNodeScalarReader<T>::read(
								ConfigReader* reader,
								const BzfString& data)
{
	// skip whitespace
	const char* scan = data.c_str();
	while (*scan != '\0' && isspace(*scan))
		++scan;

	// read values until there are no more
	while (*scan != '\0') {
		// parse
		char* end;
		T value = parse(scan, &end);

		// check for parse errors
		if (end == scan) {
			printError("%s: error reading value in %s",
								reader->getPosition().c_str(),
								field->getName());
			return false;
		}
		if (done) {
			printError("%s: more than one value in %s",
								reader->getPosition().c_str(),
								field->getName());
			return false;
		}

		// append index
		field->set(value);
		done = true;

		// prep for next token
		scan  = end;
		while (*scan != '\0' && isspace(*scan))
			++scan;
	}

	return true;
}

template <class T>
const char*				SceneNodeScalarReader<T>::getName() const
{
	return field->getName();
}

inline
unsigned int			SceneNodeScalarReader<unsigned int>::parse(
								const char* src, char** end) const
{
	return static_cast<unsigned int>(strtoul(src, end, 10));
}

inline
bool					SceneNodeScalarReader<bool>::parse(
								const char* src, char** end) const
{
	if (strcmp(src, "true") == 0) {
		if (isspace(src[4]) || src[4] == '\0') {
			*end = const_cast<char*>(src) + 4;
			return true;
		}
	}
	else if (strcmp(src, "false") == 0) {
		if (isspace(src[5]) || src[5] == '\0') {
			*end = const_cast<char*>(src) + 5;
			return false;
		}
	}
	*end = const_cast<char*>(src);
	return false;
}

inline
float					SceneNodeScalarReader<float>::parse(
								const char* src, char** end) const
{
	return static_cast<float>(strtod(src, end));
}

inline
bool					SceneNodeScalarReader<BzfString>::open(
								ConfigReader*,
								const BzfString&,
								const ConfigReader::Values&)
{
	field->set("");
	done = false;
	return true;
}

inline
bool					SceneNodeScalarReader<BzfString>::read(
								ConfigReader*,
								const BzfString& data)
{
	// skip leading whitespace
	const char* scan = data.c_str();
	while (*scan != '\0' && isspace(*scan))
		++scan;

	// prepend newline if not empty
	BzfString buffer = field->get();
	if (buffer.size() > 0)
		buffer += "\n";

	// append data
	buffer += scan;
	field->set(buffer);
	done = true;

	return true;
}


//
// SceneNodeVectorReader<T>
//

template <class T>
class SceneNodeVectorReader : public SceneNodeFieldReader {
	public:
		SceneNodeVectorReader(SceneNodeVectorField<T>*);
		virtual ~SceneNodeVectorReader() { }

		virtual bool		open(ConfigReader* reader,
								const BzfString& tag,
								const ConfigReader::Values& values);
		virtual bool		close(ConfigReader* reader);
		virtual bool		read(ConfigReader* reader, const BzfString& data);
		virtual const char*	getName() const;

	private:
		T					parse(const char*, char**) const;

	private:
		typedef std::vector<T> Buffer;
		Buffer				buffer;
		SceneNodeVectorField<T>*	field;
};

template <class T>
SceneNodeVectorReader<T>::SceneNodeVectorReader(
								SceneNodeVectorField<T>* _field) :
								buffer(),
								field(_field)
{
	// do nothing
}

template <class T>
bool					SceneNodeVectorReader<T>::open(ConfigReader*,
								const BzfString&,
								const ConfigReader::Values& values)
{
	buffer.clear();

	ConfigReader::Values::const_iterator index = values.find("t");
	if (index != values.end())
		field->setInterpolationParameter(index->second);

	return true;
}

template <class T>
bool					SceneNodeVectorReader<T>::close(ConfigReader* reader)
{
	// make sure we satisfy field constraints
	if (buffer.size() < field->getMinNum()) {
		printError("%s: not enough values in %s (got %u, expected %u)",
								reader->getPosition().c_str(),
								field->getName(),
								field->getMinNum(),
								buffer.size());
		return false;
	}
	if (buffer.size() > field->getMaxNum()) {
		printError("%s: too many values in %s (got %u, expected %u)",
								reader->getPosition().c_str(),
								field->getName(),
								field->getMaxNum(),
								buffer.size());
		return false;
	}
	if (buffer.size() % field->getMultNum() != 0) {
		printError("%s: wrong multiple of values in %s (got %u of %u)",
								reader->getPosition().c_str(),
								field->getName(),
								buffer.size() % field->getMultNum(),
								field->getMultNum());
		return false;
	}

	// swap
	field->swap(buffer);

	return true;
}

template <class T>
bool					SceneNodeVectorReader<T>::read(
								ConfigReader* reader,
								const BzfString& data)
{
	// skip whitespace
	const char* scan = data.c_str();
	while (*scan != '\0' && isspace(*scan))
		++scan;

	// read values until there are no more
	while (*scan != '\0') {
		// parse
		char* end;
		T value = parse(scan, &end);

		// check for parse errors
		if (end == scan) {
			printError("%s: error reading value in %s",
								reader->getPosition().c_str(),
								field->getName());
			return false;
		}

		// append index
		buffer.push_back(value);

		// prep for next token
		scan  = end;
		while (*scan != '\0' && isspace(*scan))
			++scan;
	}

	return true;
}

template <class T>
const char*				SceneNodeVectorReader<T>::getName() const
{
	return field->getName();
}

inline
unsigned int			SceneNodeVectorReader<unsigned int>::parse(
								const char* src, char** end) const
{
	return static_cast<unsigned int>(strtoul(src, end, 10));
}

inline
float					SceneNodeVectorReader<float>::parse(
								const char* src, char** end) const
{
	return static_cast<float>(strtod(src, end));
}

inline
bool					SceneNodeVectorReader<BzfString>::read(
								ConfigReader*,
								const BzfString& data)
{
	// skip leading whitespace
	const char* scan = data.c_str();
	while (*scan != '\0' && isspace(*scan))
		++scan;

	buffer.push_back(scan);
	return true;
}


//
// SceneNodeGStateReader
//

class SceneNodeGStateReader : public SceneNodeFieldReader {
	public:
		SceneNodeGStateReader(SceneNodeGState* _node);
		virtual ~SceneNodeGStateReader();

		virtual bool		open(ConfigReader* reader,
								const BzfString& tag,
								const ConfigReader::Values& values);
		virtual bool		close(ConfigReader* reader);
		virtual bool		read(ConfigReader* reader, const BzfString& data);
		virtual const char*	getName() const;

	private:
		SceneNodeGState*	node;
		OpenGLGStateBuilder	builder;
		BzfString			active;
};

SceneNodeGStateReader::SceneNodeGStateReader(SceneNodeGState* _node) :
								node(_node)
{
	// default depth function for scene nodes is less-than
	builder.setDepthFunc(GState::kLess);
}

SceneNodeGStateReader::~SceneNodeGStateReader()
{
	node->set(builder.getState());
}

bool					SceneNodeGStateReader::open(
								ConfigReader*,
								const BzfString& tag,
								const ConfigReader::Values& values)
{
	SceneReader::Values::const_iterator index;
	if (tag == "texture") {
		bool repeat = true;
		bool forceRGBA = false;

		// get texture parameters
		index = values.find("repeat");
		if (index != values.end())
			repeat = (index->second != "off");
		index = values.find("rgba");
		if (index != values.end())
			forceRGBA = (index->second != "off");
		index = values.find("force");
		if (index != values.end())
			builder.setForceTexture(index->second != "off");

		// get file name and verify that it's there
		index = values.find("filename");
		if (index == values.end()) {
			builder.setTexture(OpenGLTexture());
		}
		else {
			const BzfString filename(index->second);

			// get the filter
			OpenGLTexture::Filter filter = OpenGLTexture::Max;
			index = values.find("filter");
			if (index != values.end()) {
				if (index->second == "nearest")
					filter = OpenGLTexture::Nearest;
				else if (index->second == "linear")
					filter = OpenGLTexture::Linear;
				else if (index->second == "nearestmipmapnearest")
					filter = OpenGLTexture::NearestMipmapNearest;
				else if (index->second == "linearmipmapnearest")
					filter = OpenGLTexture::LinearMipmapNearest;
				else if (index->second == "nearestmipmaplinear")
					filter = OpenGLTexture::NearestMipmapLinear;
				else if (index->second == "linearmipmaplinear")
					filter = OpenGLTexture::LinearMipmapLinear;
			}

			// load the texture
			int w, h;
			builder.setTexture(OpenGLTexture(filename, &w, &h,
							filter,				// max filter
							repeat,				// repeat
							forceRGBA ? OpenGLTexture::getRGBAFormat() : 0));
		}
	}

	else if (tag == "texenv") {
		index = values.find("mode");
		if (index != values.end()) {
			if (index->second == "modulate")
				builder.setTexEnv(GState::kModulate);
			else if (index->second == "decal")
				builder.setTexEnv(GState::kDecal);
			else if (index->second == "replace")
				builder.setTexEnv(GState::kReplace);
		}
	}

	else if (tag == "shading") {
		index = values.find("model");
		if (index != values.end()) {
			if (index->second == "flat")
				builder.setShading(GState::kConstant);
			else if (index->second == "smooth")
				builder.setShading(GState::kLinear);
		}
	}

	else if (tag == "blending") {
		GState::BlendFactor src = GState::kOne;
		GState::BlendFactor dst = GState::kZero;
		index = values.find("src");
		if (index != values.end()) {
			if (index->second == "0")
				src = GState::kZero;
			else if (index->second == "1")
				src = GState::kOne;
			else if (index->second == "sa")
				src = GState::kSrcAlpha;
			else if (index->second == "1-sa")
				src = GState::kOneMinusSrcAlpha;
			else if (index->second == "da")
				src = GState::kDstAlpha;
			else if (index->second == "1-da")
				src = GState::kOneMinusDstAlpha;
			else if (index->second == "dc")
				src = GState::kDstColor;
			else if (index->second == "1-dc")
				src = GState::kOneMinusDstColor;
		}
		index = values.find("dst");
		if (index != values.end()) {
			if (index->second == "0")
				dst = GState::kZero;
			else if (index->second == "1")
				dst = GState::kOne;
			else if (index->second == "sa")
				dst = GState::kSrcAlpha;
			else if (index->second == "1-sa")
				dst = GState::kOneMinusSrcAlpha;
			else if (index->second == "da")
				dst = GState::kDstAlpha;
			else if (index->second == "1-da")
				dst = GState::kOneMinusDstAlpha;
			else if (index->second == "sc")
				dst = GState::kSrcColor;
			else if (index->second == "1-sc")
				dst = GState::kOneMinusSrcColor;
		}
		builder.setBlending(src, dst);
		index = values.find("force");
		if (index != values.end())
			builder.setForceBlending(index->second != "off");
	}

	else if (tag == "smoothing") {
		index = values.find("smooth");
		if (index != values.end()) {
			if (index->second == "on")
				builder.setSmoothing(true);
			else if (index->second == "off")
				builder.setSmoothing(false);
		}
		index = values.find("force");
		if (index != values.end())
			builder.setForceSmoothing(index->second != "off");
	}

	else if (tag == "culling") {
		index = values.find("cull");
		if (index != values.end()) {
			if (index->second == "on")
				builder.setCulling(true);
			else if (index->second == "off")
				builder.setCulling(false);
		}
	}

	else if (tag == "dithering") {
		index = values.find("dither");
		if (index != values.end()) {
			if (index->second == "on")
				builder.setDithering(true);
			else if (index->second == "off")
				builder.setDithering(false);
		}
	}

	else if (tag == "alpha") {
		GState::Func func = GState::kAlways;
		float ref = 0.0f;
		index = values.find("func");
		if (index != values.end()) {
			if (index->second == "0")
				func = GState::kNever;
			else if (index->second == "1")
				func = GState::kAlways;
			else if (index->second == "==")
				func = GState::kEqual;
			else if (index->second == "!=")
				func = GState::kNotEqual;
			else if (index->second == "<")
				func = GState::kLess;
			else if (index->second == "<=")
				func = GState::kLEqual;
			else if (index->second == ">")
				func = GState::kGreater;
			else if (index->second == ">=")
				func = GState::kGEqual;
		}
		index = values.find("ref");
		if (index != values.end()) {
			ref = static_cast<float>(atof(index->second.c_str()));
			if (ref < 0.0f)
				ref = 0.0f;
			else if (ref > 1.0f)
				ref = 1.0f;
		}
		builder.setAlphaFunc(func, ref);
	}

	else if (tag == "depth") {
		GState::Func func = GState::kLess;
		bool write = true;
		index = values.find("func");
		if (index != values.end()) {
			if (index->second == "0")
				func = GState::kNever;
			else if (index->second == "1")
				func = GState::kAlways;
			else if (index->second == "==")
				func = GState::kEqual;
			else if (index->second == "!=")
				func = GState::kNotEqual;
			else if (index->second == "<")
				func = GState::kLess;
			else if (index->second == "<=")
				func = GState::kLEqual;
			else if (index->second == ">")
				func = GState::kGreater;
			else if (index->second == ">=")
				func = GState::kGEqual;
		}
		index = values.find("write");
		if (index != values.end()) {
			if (index->second == "on")
				write = true;
			else if (index->second == "off")
				write = false;
		}
		builder.setDepthFunc(func);
		builder.setDepthMask(write);
	}

	else if (tag == "point") {
		float size = 1.0f;
		index = values.find("size");
		if (index != values.end()) {
			size = (float)atof(index->second.c_str());
			if (size <= 0.0f)
				size = 1.0f;
		}
		builder.setPointSize(size);
	}

	else if (tag == "stipple") {
		bool mask = false;
		index = values.find("mask");
		if (index != values.end()) {
			if (index->second == "on")
				mask = true;
			else if (index->second == "off")
				mask = false;
		}
		builder.setStipple(mask);
	}

	else if (tag == "pass") {
		int n = 0;
		index = values.find("number");
		if (index != values.end()) {
			n = atoi(index->second.c_str());
		}
		builder.setPass(n);
	}

	else {
		return false;
	}

	active = tag;
	return true;
}

bool					SceneNodeGStateReader::close(ConfigReader*)
{
	return true;
}

bool					SceneNodeGStateReader::read(
								ConfigReader*,
								const BzfString&)
{
	// ignore data
	return true;
}

const char*				SceneNodeGStateReader::getName() const
{
	return active.c_str();
}


//
// SceneNodeGeometryReader
//

class SceneNodeGeometryReader : public SceneNodeFieldReader {
	public:
		SceneNodeGeometryReader(SceneNodeGeometry* _node,
								SceneNodeGeometry::Property _property);
		virtual ~SceneNodeGeometryReader();

		virtual bool		open(ConfigReader* reader,
								const BzfString& tag,
								const ConfigReader::Values& values);
		virtual bool		close(ConfigReader* reader);
		virtual bool		read(ConfigReader* reader, const BzfString& data);
		virtual const char*	getName() const;

	private:
		SceneNodeVFFloat*	getField() const;

	private:
		SceneNodeGeometry*	node;
		SceneNodeGeometry::Property	property;
		SceneNodeFieldReader*	fieldReader;
		unsigned int		count;
};

SceneNodeGeometryReader::SceneNodeGeometryReader(
								SceneNodeGeometry* _node,
								SceneNodeGeometry::Property _property) :
								node(_node),
								property(_property),
								fieldReader(NULL),
								count(0)
{
	assert(node != NULL);
}

SceneNodeGeometryReader::~SceneNodeGeometryReader()
{
	assert(fieldReader == NULL);
}

bool					SceneNodeGeometryReader::open(
								ConfigReader* reader,
								const BzfString& tag,
								const ConfigReader::Values& values)
{
	// increment bundle count
	++count;

	// create a new bundle if necessary, otherwise just set the current
	// bundle.
	if (node->getNumBundles() < count) {
		node->pushBundle();

		// we should never be more than one behind
		assert(node->getNumBundles() == count);

		// refer to the previous bundle's field (if any) until we
		// actually read the field
		if (count > 1) {
			node->refBundle(SceneNodeGeometry::Stipple,  count - 2);
			node->refBundle(SceneNodeGeometry::Color,    count - 2);
			node->refBundle(SceneNodeGeometry::TexCoord, count - 2);
			node->refBundle(SceneNodeGeometry::Normal,   count - 2);
			node->refBundle(SceneNodeGeometry::Vertex,   count - 2);
		}
	}
	else {
		node->setBundle(count - 1);
	}

	// see if there's a reference
	ConfigReader::Values::const_iterator index = values.find("ref");
	if (index != values.end()) {
		// get ref index
		const char* start = index->second.c_str();
		char* end;
		unsigned int refIndex = static_cast<unsigned int>(
								strtoul(start, &end, 10));
		if (end != start + index->second.size()) {
			printError("%s: error reading value in ref",
								reader->getPosition().c_str());
			return false;
		}
		if (refIndex >= count) {
			printError("%s: ignoring forward reference",
								reader->getPosition().c_str());
			return false;
		}
		node->refBundle(property, refIndex);
	}

	else {
		// create vector field reader
		node->refBundle(property, count - 1);
		fieldReader = new SceneNodeVectorReader<float>(getField());
		if (!fieldReader->open(reader, tag, values)) {
			delete fieldReader;
			fieldReader = NULL;
			return false;
		}
	}

	return true;
}

bool					SceneNodeGeometryReader::close(ConfigReader* reader)
{
	// if not reading a field (which should be because of a reference)
	// then do nothing
	if (fieldReader == NULL)
		return true;

	// close field reader
	const bool result = fieldReader->close(reader);

	// clean up
	delete fieldReader;
	fieldReader = NULL;

	if (!result)
		return false;

	// if values weren't read then reference the previous bundle
	const unsigned int numItems = getField()->getNum();
	if (numItems == 0) {
		// check for errors
		if (count == 1) {
			printError("%s: missing data", reader->getPosition().c_str());
			return false;
		}

		// refer to previous bundle's field
		node->refBundle(property, count - 2);
	}

	// make sure there are exactly the right number of items for
	// any bundle but the first
	if (count > 1) {
		node->setBundle(0);
		const unsigned int numItemsInFirst = getField()->getNum();
		if (numItems != numItemsInFirst) {
			printError("%s: item count mismatch between bundles "
								"(got %u, expected %u)",
								reader->getPosition().c_str(),
								numItems, numItemsInFirst);
			return false;
		}
	}

	return true;
}

bool					SceneNodeGeometryReader::read(
								ConfigReader* reader,
								const BzfString& data)
{
	if (fieldReader == NULL)
		return true;
	else
		return fieldReader->read(reader, data);
}

const char*				SceneNodeGeometryReader::getName() const
{
	if (fieldReader != NULL)
		return fieldReader->getName();
	else
		return getField()->getName();
}

SceneNodeVFFloat*		SceneNodeGeometryReader::getField() const
{
	switch (property) {
		case SceneNodeGeometry::Stipple:
			return node->stipple;

		case SceneNodeGeometry::Color:
			return node->color;

		case SceneNodeGeometry::TexCoord:
			return node->texcoord;

		case SceneNodeGeometry::Normal:
			return node->normal;

		case SceneNodeGeometry::Vertex:
			return node->vertex;

		default:
			assert(0 && "bogus geometry property");
			return NULL;
	}
}


//
// SceneReader
//

SceneReader::SceneReader() : node(NULL)
{
	// do nothing
}

SceneReader::~SceneReader()
{
	// do nothing
}

SceneNode*				SceneReader::read(istream& s)
{
	// set initial state
	if (node != NULL) {
		node->unref();
		node = NULL;
	}

	// prep config reader
	ConfigReader reader;
	reader.push(&openCB, &closeCB, NULL);

	// read stream
	assert(stack.size() == 0);
	configReader = &reader;
	const bool result = reader.read(s, this);
	configReader = NULL;

	// clean up
	stack.clear();
	if (!result && node != NULL) {
		node->unref();
		node = NULL;
	}
	namedNodes.clear();

	return node;
}

BzfString				SceneReader::getPosition() const
{
	return configReader->getPosition();
}

bool					SceneReader::open(ConfigReader* reader,
								const BzfString& tag,
								const ConfigReader::Values& values)
{
	if (tag == "group") {
		return push(reader, values, new SceneNodeGroup);
	}

	else if (tag == "transform") {
		SceneNodeTransform* node = new SceneNodeTransform;
		if (!push(reader, values, node))
			return false;

		readEnum(reader, values, node->type);
		addReader(new SceneNodeScalarReader<unsigned int>(&node->up));
		addReader(new SceneNodeVectorReader<float>(&node->center));
		addReader(new SceneNodeVectorReader<float>(&node->rotate));
		addReader(new SceneNodeVectorReader<float>(&node->scale));
		addReader(new SceneNodeVectorReader<float>(&node->scaleOrientation));
		addReader(new SceneNodeVectorReader<float>(&node->translate));
	}

	else if (tag == "matrix") {
		SceneNodeMatrixTransform* node = new SceneNodeMatrixTransform;
		if (!push(reader, values, node))
			return false;

		readEnum(reader, values, node->type);
		addReader(new SceneNodeScalarReader<unsigned int>(&node->up));
		addReader(new SceneNodeVectorReader<float>(&node->matrix));
	}

	else if (tag == "spherical") {
		SceneNodeSphereTransform* node = new SceneNodeSphereTransform;
		if (!push(reader, values, node))
			return false;

		readEnum(reader, values, node->type);
		addReader(new SceneNodeScalarReader<unsigned int>(&node->up));
		addReader(new SceneNodeVectorReader<float>(&node->azimuth));
		addReader(new SceneNodeVectorReader<float>(&node->altitude));
		addReader(new SceneNodeVectorReader<float>(&node->twist));
		addReader(new SceneNodeVectorReader<float>(&node->radius));
		addReader(new SceneNodeVectorReader<float>(&node->translate));
	}

	else if (tag == "billboard") {
		SceneNodeBillboard* node = new SceneNodeBillboard;
		if (!push(reader, values, node))
			return false;

		readEnum(reader, values, node->type);
		addReader(new SceneNodeScalarReader<unsigned int>(&node->up));
		addReader(new SceneNodeVectorReader<float>(&node->axis));
		addReader(new SceneNodeScalarReader<bool>(&node->turn));
		addReader(new SceneNodeScalarReader<bool>(&node->center));
	}

	else if (tag == "primitive") {
		SceneNodePrimitive* node = new SceneNodePrimitive;
		if (!push(reader, values, node))
			return false;

		readEnum(reader, values, node->type);
		addReader(new SceneNodeVectorReader<unsigned int>(&node->index));
	}

	else if (tag == "geometry") {
		SceneNodeGeometry* node = new SceneNodeGeometry;
		if (!push(reader, values, node))
			return false;

		addReader(new SceneNodeGeometryReader(node, SceneNodeGeometry::Stipple));
		addReader(new SceneNodeGeometryReader(node, SceneNodeGeometry::Color));
		addReader(new SceneNodeGeometryReader(node, SceneNodeGeometry::TexCoord));
		addReader(new SceneNodeGeometryReader(node, SceneNodeGeometry::Normal));
		addReader(new SceneNodeGeometryReader(node, SceneNodeGeometry::Vertex));
	}

	else if (tag == "gstate") {
		SceneNodeGState* node = new SceneNodeGState;
		if (!push(reader, values, node))
			return false;

		addReader(new SceneNodeGStateReader(node));
	}

	else if (tag == "animate") {
		SceneNodeAnimate* node = new SceneNodeAnimate;
		if (!push(reader, values, node))
			return false;

		readEnum(reader, values, node->type);
		addReader(new SceneNodeScalarReader<BzfString>(&node->src));
		addReader(new SceneNodeScalarReader<BzfString>(&node->dst));
		addReader(new SceneNodeScalarReader<float>(&node->start));
		addReader(new SceneNodeScalarReader<float>(&node->end));
		addReader(new SceneNodeScalarReader<float>(&node->bias));
		addReader(new SceneNodeScalarReader<float>(&node->scale));
		addReader(new SceneNodeScalarReader<float>(&node->step));
		addReader(new SceneNodeScalarReader<float>(&node->cycles));
	}

	else if (tag == "parameters") {
		SceneNodeParameters* node = new SceneNodeParameters;
		if (!push(reader, values, node))
			return false;

		addReader(new SceneNodeVectorReader<BzfString>(&node->src));
		addReader(new SceneNodeVectorReader<BzfString>(&node->dst));
		addReader(new SceneNodeVectorReader<float>(&node->scale));
		addReader(new SceneNodeVectorReader<float>(&node->bias));
	}

	else if (tag == "choice") {
		SceneNodeChoice* node = new SceneNodeChoice;
		if (!push(reader, values, node))
			return false;

		addReader(new SceneNodeVectorReader<unsigned int>(&node->mask));
	}

	else if (tag == "lod") {
		SceneNodeLOD* node = new SceneNodeLOD;
		if (!push(reader, values, node))
			return false;

		readEnum(reader, values, node->type);
		addReader(new SceneNodeVectorReader<float>(&node->sphere));
		addReader(new SceneNodeVectorReader<float>(&node->range));
	}

	else if (tag == "light") {
		SceneNodeLight* node = new SceneNodeLight;
		if (!push(reader, values, node))
			return false;

		addReader(new SceneNodeVectorReader<float>(&node->ambient));
		addReader(new SceneNodeVectorReader<float>(&node->diffuse));
		addReader(new SceneNodeVectorReader<float>(&node->specular));
		addReader(new SceneNodeVectorReader<float>(&node->position));
		addReader(new SceneNodeVectorReader<float>(&node->spotDirection));
		addReader(new SceneNodeVectorReader<float>(&node->spotExponent));
		addReader(new SceneNodeVectorReader<float>(&node->spotCutoff));
		addReader(new SceneNodeVectorReader<float>(&node->attenuation));
	}

	else if (tag == "metadata") {
		SceneNodeMetadata* node = new SceneNodeMetadata;
		if (!push(reader, values, node))
			return false;

		addReader(new SceneNodeScalarReader<BzfString>(&node->data));
	}

	// see if it's a reference
	else if (tag == "ref") {
		// get id
		ConfigReader::Values::const_iterator index = values.find("id");
		if (index == values.end()) {
			printError("%s: missing id in ref",
								reader->getPosition().c_str());
			return false;
		}

		// find the named node
		SceneNode* refNode = NULL;
		NodeMap::iterator nameIndex = namedNodes.find(index->second);
		if (nameIndex != namedNodes.end()) {
			refNode = nameIndex->second;
		}

		// error if node not found
		if (refNode == NULL) {
			printError("%s: cannot find ref'd node %s",
								reader->getPosition().c_str(),
								index->second.c_str());
			return false;
		}

		// couldn't have found node if we haven't read any nodes yet
		assert(node != NULL);

		// check for multiple top level nodes
		if (stack.empty()) {
			printError("%s: multiple top level nodes found",
								reader->getPosition().c_str());
			return false;
		}

		// check for child of a leaf
		if (stack.back().group == NULL) {
			printError("%s: child of a leaf node found",
								reader->getPosition().c_str());
			return false;
		}

		// add node as child of current group
		stack.back().group->pushChild(refNode);

		// ignore everything inside the ref
		reader->push(NULL, NULL, NULL);

		return true;
	}

	else {
		// unrecognized
		printError("%s: ignoring invalid tag: %s",
								reader->getPosition().c_str(),
								tag.c_str());
		return false;
	}

	return true;
}

bool					SceneReader::close(
								ConfigReader* reader,
								const BzfString& tag)
{
	if (tag == "ref") {
		// closing a ref
		return true;
	}

	else {
		// closing a node
		return pop(reader);
	}
}

bool					SceneReader::openField(
								ConfigReader* reader,
								const BzfString& tag,
								const ConfigReader::Values& values)
{
	assert(!stack.empty());

	State& state = stack.back();

	// we should not get nested parameters
	if (state.activeField != NULL) {
		printError("%s: nested parameter %s not allowed",
								reader->getPosition().c_str(),
								tag.c_str());
		return false;
	}

	// see if we've got a parameter reader for the tag.  if not then
	// try a catch-all reader.  if not that then try it as a regular node.
	bool isGeneric = false;
	FieldReaders::iterator index = state.fieldReaders.find(tag);
	if (index == state.fieldReaders.end()) {
		index = state.fieldReaders.find("");
		if (index == state.fieldReaders.end())
			return open(reader, tag, values);
		isGeneric = true;
	}

	// found it.  let it do its thing.  if we're using the catch-all
	// reader (i.e. isGeneric) and it fails to open then assume the
	// tag wasn't recognized and try opening as a regular node.
	SceneNodeFieldReader* activeField = index->second;
	if (!activeField->open(reader, tag, values))
		if (isGeneric)
			return open(reader, tag, values);
		else
			return false;

	// make it active
	state.activeField = activeField;
	return true;
}

bool					SceneReader::closeField(
								ConfigReader* reader,
								const BzfString& tag)
{
	assert(!stack.empty());

	bool result;
	State& state = stack.back();
	if (state.activeField != NULL) {
		result = state.activeField->close(reader);
		state.activeField = NULL;
	}
	else {
		result = close(reader, tag);
	}

	return result;
}

bool					SceneReader::dataField(
								ConfigReader* reader,
								const BzfString& data)
{
	assert(!stack.empty());

	// ignore data outside of a field
	if (stack.back().activeField == NULL)
		return true;

	// pass on to the field reader
	return stack.back().activeField->read(reader, data);
}

bool					SceneReader::pushCommon(ConfigReader* reader,
								const ConfigReader::Values& values,
								SceneNode* newNode)
{
	// push item onto stack
	State state;
	state.node        = newNode;
	state.group       = NULL;
	state.activeField = NULL;
	stack.push_back(state);

	// saved named nodes
	ConfigReader::Values::const_iterator index = values.find("id");
	if (index != values.end()) {
		const BzfString& id = index->second;
		NodeMap::iterator oldIndex = namedNodes.find(id);
		if (oldIndex != namedNodes.end()) {
			// replace previous with same name
			oldIndex->second = newNode;
		}
		else {
			// new name
			namedNodes.insert(std::make_pair(id, newNode));
		}

		// save id in node
		newNode->setID(id);
	}

	reader->push(openFieldCB, closeFieldCB, dataFieldCB);

	return true;
}

bool					SceneReader::push(ConfigReader* reader,
								const ConfigReader::Values& values,
								SceneNode* newNode)
{
	// if top level node then save it
	if (node == NULL) {
		assert(stack.empty());

		// save top node
		node = newNode;
	}

	// check for multiple top level nodes
	else if (stack.empty()) {
		printError("%s: multiple top level nodes found",
								reader->getPosition().c_str());
		newNode->unref();
		return false;
	}

	// check for child of a leaf
	else if (stack.back().group == NULL) {
		printError("%s: child of a leaf node found",
								reader->getPosition().c_str());
		newNode->unref();
		return false;
	}

	// add node as child of current group
	else {
		// add to end of parent group
		stack.back().group->pushChild(newNode);

		// parent now owns node
		newNode->unref();
	}

	return pushCommon(reader, values, newNode);
}

bool					SceneReader::push(ConfigReader* reader,
								const ConfigReader::Values& values,
								SceneNodeGroup* newGroupNode)
{
	// do regular node stuff
	if (!push(reader, values, static_cast<SceneNode*>(newGroupNode)))
		return false;

	// save group node
	stack.back().group = newGroupNode;

	return true;
}

bool					SceneReader::pop(ConfigReader* reader)
{
	State& state = stack.back();

	// make sure we're in a state to close
	if (state.activeField != NULL) {
		printError("%s: parameter %s not closed",
								reader->getPosition().c_str(),
								state.activeField->getName());
		return false;
	}

	// clean up
	for (FieldReaders::iterator index = state.fieldReaders.begin();
								index != state.fieldReaders.end(); ++index)
		delete index->second;

	// close
	stack.pop_back();

	return true;
}

void					SceneReader::readEnum(
								ConfigReader* reader,
								const ConfigReader::Values& values,
								SceneNodeSFEnum& field)
{
	// see if values contains a value for the field
	ConfigReader::Values::const_iterator index = values.find(field.getName());
	if (index == values.end())
		return;

	// match the value to one of the enumerations
	const BzfString& value = index->second;
	unsigned int num = field.getNumEnums();
	for (unsigned int i = 0; i < num; ++i)
		if (value == field.getEnum(i)) {
			field.set(i);
			return;
		}

	// not found
	printError("%s: ignoring unknown enumerant %s",
								reader->getPosition().c_str(),
								value.c_str());
}

void					SceneReader::addReader(
								SceneNodeFieldReader* reader)
{
	stack.back().fieldReaders.insert(std::make_pair(reader->getName(), reader));
}

bool					SceneReader::openCB(ConfigReader* reader,
								const BzfString& tag,
								const ConfigReader::Values& values,
								void* self)
{
	return reinterpret_cast<SceneReader*>(self)->open(reader, tag, values);
}

bool					SceneReader::closeCB(ConfigReader* reader,
								const BzfString& tag,
								void* self)
{
	return reinterpret_cast<SceneReader*>(self)->close(reader, tag);
}

bool					SceneReader::openFieldCB(ConfigReader* reader,
								const BzfString& tag,
								const ConfigReader::Values& values,
								void* self)
{
	return reinterpret_cast<SceneReader*>(self)->openField(reader, tag, values);
}

bool					SceneReader::closeFieldCB(ConfigReader* reader,
								const BzfString& tag,
								void* self)
{
	return reinterpret_cast<SceneReader*>(self)->closeField(reader, tag);
}

bool					SceneReader::dataFieldCB(ConfigReader* reader,
								const BzfString& data,
								void* self)
{
	return reinterpret_cast<SceneReader*>(self)->dataField(reader, data);
}
