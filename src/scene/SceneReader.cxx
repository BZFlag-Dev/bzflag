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
#include <string.h>
#include <ctype.h>

//
// SceneNodeFieldReader
//

class SceneNodeFieldReader {
public:
	SceneNodeFieldReader() { }
	virtual ~SceneNodeFieldReader() { }

	virtual bool		parse(XMLTree::iterator);
	virtual const char*	getName() const = 0;

protected:
	virtual void		parseBegin(XMLTree::iterator) { }
	virtual void		parseData(XMLTree::iterator, const std::string&) { }
	virtual void		parseEnd(XMLTree::iterator) { }
};

bool					SceneNodeFieldReader::parse(XMLTree::iterator xml)
{
	parseBegin(xml);
	XMLTree::sibling_iterator scan = xml.begin();
	XMLTree::sibling_iterator end  = xml.end();
	for (; scan != end; ++scan) {
		if (scan->type != XMLNode::Data)
			throw XMLIOException(scan->position,
								"fields may not have children");
		parseData(scan, scan->value);
	}
	parseEnd(xml);
	return true;
}


//
// SceneNodeScalarReader<T>
//

template <class T>
class SceneNodeScalarReader : public SceneNodeFieldReader {
public:
	SceneNodeScalarReader(SceneNodeScalarField<T>*);
	virtual ~SceneNodeScalarReader() { }

	virtual const char*	getName() const;

protected:
	virtual void		parseBegin(XMLTree::iterator);
	virtual void		parseData(XMLTree::iterator, const std::string&);
	virtual void		parseEnd(XMLTree::iterator);

private:
	T					parseValue(const char*, char**) const;

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
void					SceneNodeScalarReader<T>::parseBegin(XMLTree::iterator)
{
	done = false;
}

template <class T>
void					SceneNodeScalarReader<T>::parseData(
								XMLTree::iterator xml, const std::string& data)
{
	// skip whitespace
	const char* scan = data.c_str();
	while (*scan != '\0' && isspace(*scan))
		++scan;

	// read values until there are no more
	while (*scan != '\0') {
		// parse
		char* end;
		T value = parseValue(scan, &end);

		// check for parse errors
		if (end == scan)
			throw XMLIOException(xml->position,
								string_util::format(
									"error reading value in `%s' field",
									field->getName()));
		if (done)
			throw XMLIOException(xml->position,
								string_util::format(
									"more than one value in `%s' field",
									field->getName()));

		// append index
		field->set(value);
		done = true;

		// prep for next token
		scan = end;
		while (*scan != '\0' && isspace(*scan))
			++scan;
	}
}

template <class T>
void					SceneNodeScalarReader<T>::parseEnd(
								XMLTree::iterator xml)
{
	if (!done)
		throw XMLIOException(xml->position,
								string_util::format(
									"expected a value in `%s' field",
									field->getName()));
}

template <class T>
const char*				SceneNodeScalarReader<T>::getName() const
{
	return field->getName();
}

inline
unsigned int			SceneNodeScalarReader<unsigned int>::parseValue(
								const char* src, char** end) const
{
	return static_cast<unsigned int>(strtoul(src, end, 10));
}

inline
bool					SceneNodeScalarReader<bool>::parseValue(
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
float					SceneNodeScalarReader<float>::parseValue(
								const char* src, char** end) const
{
	return static_cast<float>(strtod(src, end));
}

inline
void					SceneNodeScalarReader<std::string>::parseBegin(
								XMLTree::iterator)
{
	field->set("");
	done = false;
}

inline
void					SceneNodeScalarReader<std::string>::parseData(
								XMLTree::iterator, const std::string& data)
{
	// skip leading whitespace
	const char* scan = data.c_str();
	while (*scan != '\0' && isspace(*scan))
		++scan;

	// prepend space if not empty
	std::string buffer = field->get();
	if (buffer.size() > 0)
		buffer += " ";

	// append data
	buffer += scan;
	field->set(buffer);
	done = true;
}


//
// SceneNodeVectorReader<T>
//

template <class T>
class SceneNodeVectorReader : public SceneNodeFieldReader {
public:
	SceneNodeVectorReader(SceneNodeVectorField<T>*);
	virtual ~SceneNodeVectorReader() { }

	virtual const char*	getName() const;

protected:
	virtual void		parseBegin(XMLTree::iterator);
	virtual void		parseData(XMLTree::iterator, const std::string&);
	virtual void		parseEnd(XMLTree::iterator);

private:
	T					parseValue(const char*, char**) const;

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
void					SceneNodeVectorReader<T>::parseBegin(
								XMLTree::iterator xml)
{
	buffer.clear();
	xml->getAttribute("t", xmlSetMethod(field, &SceneNodeVectorField<T>::
											setInterpolationParameter));
}

template <class T>
void					SceneNodeVectorReader<T>::parseData(
								XMLTree::iterator xml,
								const std::string& data)
{
	// skip whitespace
	const char* scan = data.c_str();
	while (*scan != '\0' && isspace(*scan))
		++scan;

	// read values until there are no more
	while (*scan != '\0') {
		// parse
		char* end;
		T value = parseValue(scan, &end);

		// check for parse errors
		if (end == scan)
			throw XMLIOException(xml->position,
								string_util::format(
									"error reading value in `%s'",
									field->getName()));

		// append index
		buffer.push_back(value);

		// prep for next token
		scan  = end;
		while (*scan != '\0' && isspace(*scan))
			++scan;
	}
}

template <class T>
void					SceneNodeVectorReader<T>::parseEnd(
								XMLTree::iterator xml)
{
	// make sure we satisfy field constraints
	if (buffer.size() < field->getMinNum())
		throw XMLIOException(xml->position, string_util::format(
							"not enough values in `%s' (got %u, expected %u)",
								field->getName(),
								field->getMinNum(),
								buffer.size()));
	if (buffer.size() > field->getMaxNum())
		throw XMLIOException(xml->position, string_util::format(
							"too many values in `%s' (got %u, expected %u)",
								field->getName(),
								field->getMaxNum(),
								buffer.size()));
	if (buffer.size() % field->getMultNum() != 0)
		throw XMLIOException(xml->position, string_util::format(
							"wrong multiple of values in `%s' (got %u of %u)",
								field->getName(),
								buffer.size() % field->getMultNum(),
								field->getMultNum()));

	// swap
	field->swap(buffer);
}

template <class T>
const char*				SceneNodeVectorReader<T>::getName() const
{
	return field->getName();
}

inline
unsigned int			SceneNodeVectorReader<unsigned int>::parseValue(
								const char* src, char** end) const
{
	return static_cast<unsigned int>(strtoul(src, end, 10));
}

inline
float					SceneNodeVectorReader<float>::parseValue(
								const char* src, char** end) const
{
	return static_cast<float>(strtod(src, end));
}

inline
void					SceneNodeVectorReader<std::string>::parseData(
								XMLTree::iterator,
								const std::string& data)
{
	// skip leading whitespace
	const char* scan = data.c_str();
	while (*scan != '\0' && isspace(*scan))
		++scan;

	// values are delimited by (unescaped) commas
	while (*scan != '\0') {
		// find the first unescaped comma
		std::string value;
		while (*scan != '\0') {
			if (*scan == '\\') {
				switch (*++scan) {
					case '\0':
						// trailing backslash.  discard.
						break;

					case ',':
					case '\\':
					default:
						// add literal
						value += *scan;
						break;
				}
				++scan;
			}
			else if (*scan == ',') {
				// end of value.  discard comma.
				++scan;
				break;
			}
			else {
				// literal
				value += *scan;
				++scan;
			}
		}

		// add value
		buffer.push_back(value);

		// skip leading whitespace of next value
		while (*scan != '\0' && isspace(*scan))
			++scan;
	}
}

//
// SceneNodeParticleSystemReader
//

class SceneNodeParticleSystemReader : public SceneNodeFieldReader {
public:
	SceneNodeParticleSystemReader(SceneNodeParticleSystem* _node);
	virtual ~SceneNodeParticleSystemReader();

	virtual bool			parse(XMLTree::iterator);
	virtual const char*		getName() const;

private:
	const char*			getField(XMLTree::iterator xml = NULL) const;

private:
	SceneNodeParticleSystem*	node;
};

SceneNodeParticleSystemReader::SceneNodeParticleSystemReader(SceneNodeParticleSystem* _node) :
								node(_node)
{
	assert(node != NULL);
}

SceneNodeParticleSystemReader::~SceneNodeParticleSystemReader()
{
	node->stopped = false;
	// kludge: copy fields from SceneNode scalars/vectors into real variables
	// this looks -really- gross
	// TODO: switch everything to SceneNodeVFFloats and similar types instead
	// of basic arrays and scalars.
	node->location[0] = node->locationV.get()[0];
	node->location[1] = node->locationV.get()[1];
	node->location[2] = node->locationV.get()[2];
	node->velocity[0] = node->velocityV.get()[0];
	node->velocity[1] = node->velocityV.get()[1];
	node->velocity[2] = node->velocityV.get()[2];
	node->startColor[0] = node->startColorV.get()[0];
	node->startColor[1] = node->startColorV.get()[1];
	node->startColor[2] = node->startColorV.get()[2];
	node->startColor[3] = node->startColorV.get()[3];
	node->endColor[0] = node->endColorV.get()[0];
	node->endColor[1] = node->endColorV.get()[1];
	node->endColor[2] = node->endColorV.get()[2];
	node->endColor[3] = node->endColorV.get()[3];
	node->startSize = node->startSizeV.get();
	node->endSize = node->endSizeV.get();
	node->gravity[0] = node->gravityV.get()[0];
	node->gravity[1] = node->gravityV.get()[1];
	node->gravity[2] = node->gravityV.get()[2];
	node->speed = node->speedV.get();
	node->life = node->lifeV.get();
	node->fieldAngle = node->fieldAngleV.get();
	node->attractionPercent = node->attractionPercentV.get();
	if(node->action == CreateBurst) {
		node->particlesPerSecond = node->burstSizeV.get();
	}
	else {
		node->particlesPerSecond = node->creationSpeedV.get();
	}
}

bool SceneNodeParticleSystemReader::parse(XMLTree::iterator xml = NULL)
{
	static const XMLParseEnumList<ParticleSystemType> s_enumMethod[] = {
		{ "burst",	CreateBurst },
		{ "continuous",	CreateConstant },
		{ NULL, 	CreateConstant }
	};

	if(xml->value == "attracting") {
		xml->getAttribute("state", xmlParseEnum(s_xmlEnumBool, xmlSetVar(node->attracting)));
	}
	else if(xml->value == "method") {
		xml->getAttribute("action", xmlParseEnum(s_enumMethod, xmlSetVar(node->action)));
	}
	else {
		return false;
	}
	return true;
}

const char*				SceneNodeParticleSystemReader::getName() const
{
	return "";
}

//
// SceneNodeGStateReader
//

class SceneNodeGStateReader : public SceneNodeFieldReader {
public:
	SceneNodeGStateReader(SceneNodeGState* _node);
	virtual ~SceneNodeGStateReader();

	virtual bool		parse(XMLTree::iterator);
	virtual const char*	getName() const;

private:
	SceneNodeGState*	node;
	OpenGLGStateBuilder	builder;
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

bool					SceneNodeGStateReader::parse(XMLTree::iterator xml)
{
	static const XMLParseEnumList<GState::BlendFactor> s_enumBlendSrc[] = {
		{ "0",    GState::kZero },
		{ "1",    GState::kOne },
		{ "sa",   GState::kSrcAlpha },
		{ "1-sa", GState::kOneMinusSrcAlpha },
		{ "da",   GState::kDstAlpha },
		{ "1-da", GState::kOneMinusDstAlpha },
		{ "dc",   GState::kDstColor },
		{ "1-dc", GState::kOneMinusDstColor },
		{ NULL, GState::kZero }
	};
	static const XMLParseEnumList<GState::BlendFactor> s_enumBlendDst[] = {
		{ "0",    GState::kZero },
		{ "1",    GState::kOne },
		{ "sa",   GState::kSrcAlpha },
		{ "1-sa", GState::kOneMinusSrcAlpha },
		{ "da",   GState::kDstAlpha },
		{ "1-da", GState::kOneMinusDstAlpha },
		{ "sc",   GState::kSrcColor },
		{ "1-sc", GState::kOneMinusSrcColor },
		{ NULL, GState::kZero }
	};
	static const XMLParseEnumList<OpenGLTexture::Filter> s_enumFilter[] = {
		{ "nearest",              OpenGLTexture::Nearest },
		{ "linear",               OpenGLTexture::Linear },
		{ "nearestmipmapnearest", OpenGLTexture::NearestMipmapNearest },
		{ "linearmipmapnearest",  OpenGLTexture::LinearMipmapNearest },
		{ "nearestmipmaplinear",  OpenGLTexture::NearestMipmapLinear },
		{ "linearmipmaplinear",   OpenGLTexture::LinearMipmapLinear },
		{ NULL, OpenGLTexture::Nearest }
	};
	static const XMLParseEnumList<GState::TexEnv> s_enumTexEnvMode[] = {
		{ "modulate", GState::kModulate },
		{ "decal",    GState::kDecal },
		{ "replace",  GState::kReplace },
		{ NULL, GState::kModulate }
	};
	static const XMLParseEnumList<GState::Shading> s_enumShadingModel[] = {
		{ "flat",   GState::kConstant },
		{ "smooth", GState::kLinear },
		{ NULL, GState::kConstant }
	};
	static const XMLParseEnumList<GState::Func> s_enumFunction[] = {
		{ "0",  GState::kNever },
		{ "1",  GState::kAlways },
		{ "==", GState::kEqual },
		{ "!=", GState::kNotEqual },
		{ "<",  GState::kLess },
		{ "<=", GState::kLEqual },
		{ ">",  GState::kGreater },
		{ ">=", GState::kGEqual },
		{ NULL, GState::kNever }
	};

	if (xml->value == "texture") {
		bool repeat = true;
		bool forceRGBA = false;
		OpenGLTexture::Filter filter = OpenGLTexture::Max;

		// get texture parameters
		xml->getAttribute("repeat", xmlParseEnum(s_xmlEnumBool, xmlSetVar(repeat)));
		xml->getAttribute("rgba", xmlParseEnum(s_xmlEnumBool, xmlSetVar(forceRGBA)));
		xml->getAttribute("force", xmlParseEnum(s_xmlEnumBool, xmlSetMethod(&builder,
								&OpenGLGStateBuilder::setForceTexture)));
		xml->getAttribute("filter", xmlParseEnum(s_enumFilter, xmlSetVar(filter)));

		// get file name and verify that it's there
		std::string filename;
		if (!xml->getAttribute("filename", filename)) {
			builder.setTexture(OpenGLTexture());
		}
		else {
			// load the texture
			int w, h;
			builder.setTexture(OpenGLTexture(filename, &w, &h,
							filter,				// max filter
							repeat,				// repeat
							forceRGBA ? OpenGLTexture::getRGBAFormat() : 0));
		}
	}

	else if (xml->value == "texenv") {
		xml->getAttribute("mode", xmlParseEnum(s_enumTexEnvMode,
							xmlSetMethod(&builder,
								&OpenGLGStateBuilder::setTexEnv)));
	}

	else if (xml->value == "shading") {
		xml->getAttribute("model", xmlParseEnum(s_enumShadingModel,
							xmlSetMethod(&builder,
								&OpenGLGStateBuilder::setShading)));
	}

	else if (xml->value == "blending") {
		GState::BlendFactor src = GState::kOne;
		GState::BlendFactor dst = GState::kZero;
		xml->getAttribute("src", xmlParseEnum(s_enumBlendSrc, xmlSetVar(src)));
		xml->getAttribute("dst", xmlParseEnum(s_enumBlendDst, xmlSetVar(dst)));
		builder.setBlending(src, dst);
		xml->getAttribute("force", xmlParseEnum(s_xmlEnumBool,
							xmlSetMethod(&builder,
								&OpenGLGStateBuilder::setForceBlending)));
	}

	else if (xml->value == "smoothing") {
		xml->getAttribute("smooth", xmlParseEnum(s_xmlEnumBool,
							xmlSetMethod(&builder,
								&OpenGLGStateBuilder::setSmoothing)));
		xml->getAttribute("force", xmlParseEnum(s_xmlEnumBool,
							xmlSetMethod(&builder,
								&OpenGLGStateBuilder::setForceSmoothing)));
	}

	else if (xml->value == "culling") {
		xml->getAttribute("cull", xmlParseEnum(s_xmlEnumBool,
							xmlSetMethod(&builder,
								&OpenGLGStateBuilder::setCulling)));
	}

	else if (xml->value == "dithering") {
		xml->getAttribute("dither", xmlParseEnum(s_xmlEnumBool,
							xmlSetMethod(&builder,
								&OpenGLGStateBuilder::setDithering)));
	}

	else if (xml->value == "alpha") {
		GState::Func func = GState::kAlways;
		float ref = 0.0f;
		xml->getAttribute("func", xmlParseEnum(s_enumFunction, xmlSetVar(func)));
		xml->getAttribute("ref", xmlStrToFloat(
							xmlCompose(
								xmlCompose(xmlSetVar(ref), xmlMax(0.0f)),
								xmlMin(1.0f))));
		builder.setAlphaFunc(func, ref);
	}

	else if (xml->value == "depth") {
		xml->getAttribute("func", xmlParseEnum(s_enumFunction,
							xmlSetMethod(&builder,
								&OpenGLGStateBuilder::setDepthFunc)));
		xml->getAttribute("write", xmlParseEnum(s_xmlEnumBool,
							xmlSetMethod(&builder,
								&OpenGLGStateBuilder::setDepthMask)));
	}

	else if (xml->value == "point") {
		float size = 1.0f;
		xml->getAttribute("size", xmlStrToFloat(xmlSetVar(size)));
		if (size <= 0.0f)
			size = 1.0f;
		builder.setPointSize(size);
	}

	else if (xml->value == "stipple") {
		xml->getAttribute("mask", xmlParseEnum(s_xmlEnumBool,
							xmlSetMethod(&builder,
								&OpenGLGStateBuilder::setStipple)));
	}

	else if (xml->value == "pass") {
		xml->getAttribute("number", xmlStrToInt(xmlSetMethod(&builder,
								&OpenGLGStateBuilder::setPass)));
	}

	else {
		return false;
	}

	// gstate fields must be empty
	if (xml.begin() != xml.end())
		throw XMLIOException(xml->position, 
								string_util::format(
								"field `%s' must be empty",
								xml->value.c_str()));
	return true;
}

const char*				SceneNodeGStateReader::getName() const
{
	return "";
}


//
// SceneNodeGeometryReader
//

class SceneNodeGeometryReader : public SceneNodeFieldReader {
public:
	SceneNodeGeometryReader(SceneNodeGeometry* _node,
							SceneNodeGeometry::Property _property);
	virtual ~SceneNodeGeometryReader();

	virtual bool		parse(XMLTree::iterator);
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

bool					SceneNodeGeometryReader::parse(
								XMLTree::iterator xml)
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
	unsigned int refIndex;
	if (xml->getAttribute("ref", xmlStrToInt(xmlSetVar(refIndex)))) {
		// field must be empty
		if (xml.begin() != xml.end())
			throw XMLIOException(xml->position, "`ref' must be empty");
		if (refIndex >= count)
			throw XMLIOException(xml->position,
								"forward reference not allowed in `ref' field");
		node->refBundle(property, refIndex);
	}

	else {
		// create vector field reader
		node->refBundle(property, count - 1);
		SceneNodeVFFloat* field = getField();
		SceneNodeVectorReader<float> fieldReader(field);
		fieldReader.parse(xml);

		// if values weren't read then reference the previous bundle
		const unsigned int numItems = field->getNum();
		if (numItems == 0) {
			// first bundle must have data
			if (count == 1)
				throw XMLIOException(xml->position, 
								string_util::format(
								"missing data in `%s' field",
								field->getName()));

			// refer to previous bundle's field
			node->refBundle(property, count - 2);
		}

		// make sure there are exactly the right number of items for
		// any bundle but the first
		if (count > 1) {
			node->setBundle(0);
			const unsigned int numItemsInFirst = getField()->getNum();
			if (numItems != numItemsInFirst) {
				throw XMLIOException(xml->position, string_util::format(
								"item count mismatch between bundles "
									"(got %u, expected %u)",
									numItems, numItemsInFirst));
		
			}
		}
	}

	return true;
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

SceneNode*				SceneReader::parse(XMLTree::iterator xml)
{
	// set initial state
	node = NULL;
	assert(stack.empty());

	try {
		// parse (skip dummy top-level node)
		XMLTree::sibling_iterator scan = xml.begin();
		XMLTree::sibling_iterator end  = xml.end();
		for (; scan != end; ++scan)
			parseNode(scan);

		// stack should be empty
		assert(stack.empty());

		// done with named nodes
		namedNodes.clear();

		// return top-level node
		return node;
	}
	catch (...) {
		// clean up
		node = NULL;
		namedNodes.clear();
		while (!stack.empty())
			pop();
		throw;
	}
}

void					SceneReader::parseNode(XMLTree::iterator xml)
{
	// data is unexpected
	if (xml->type != XMLNode::Tag)
		throw XMLIOException(xml->position,
							"unexpected data outside field");

	// check if tag opens a field
	if (!stack.empty()) {
		State& state = stack.back();

		// see if we've got a field reader for the tag.  if not then
		// try a catch-all reader.
		bool isGeneric = false;
		FieldReaders::iterator index = state.fieldReaders.find(xml->value);
		if (index == state.fieldReaders.end()) {
			index = state.fieldReaders.find("");
			if (index != state.fieldReaders.end())
				isGeneric = true;
		}

		// if we found a reader then let it parse.  if we're using
		// the catch-all reader (i.e. isGeneric) and it fails then
		// fall through to opening as a node.
		if (index != state.fieldReaders.end()) {
			SceneNodeFieldReader* fieldReader = index->second;
			if (fieldReader->parse(xml))
				return;
			assert(isGeneric);
		}
	}

	if (xml->value == "group") {
		push(xml, new SceneNodeGroup);
	}

	else if (xml->value == "transform") {
		SceneNodeTransform* node = new SceneNodeTransform;
		push(xml, node);
		readEnum(xml, node->type);
		addReader(new SceneNodeScalarReader<unsigned int>(&node->up));
		addReader(new SceneNodeVectorReader<float>(&node->center));
		addReader(new SceneNodeVectorReader<float>(&node->rotate));
		addReader(new SceneNodeVectorReader<float>(&node->scale));
		addReader(new SceneNodeVectorReader<float>(&node->scaleOrientation));
		addReader(new SceneNodeVectorReader<float>(&node->translate));
	}

	else if (xml->value == "matrix") {
		SceneNodeMatrixTransform* node = new SceneNodeMatrixTransform;
		push(xml, node);
		readEnum(xml, node->type);
		addReader(new SceneNodeScalarReader<unsigned int>(&node->up));
		addReader(new SceneNodeVectorReader<float>(&node->matrix));
	}

	else if (xml->value == "spherical") {
		SceneNodeSphereTransform* node = new SceneNodeSphereTransform;
		push(xml, node);
		readEnum(xml, node->type);
		addReader(new SceneNodeScalarReader<unsigned int>(&node->up));
		addReader(new SceneNodeVectorReader<float>(&node->azimuth));
		addReader(new SceneNodeVectorReader<float>(&node->altitude));
		addReader(new SceneNodeVectorReader<float>(&node->twist));
		addReader(new SceneNodeVectorReader<float>(&node->radius));
		addReader(new SceneNodeVectorReader<float>(&node->translate));
	}

	else if (xml->value == "billboard") {
		SceneNodeBillboard* node = new SceneNodeBillboard;
		push(xml, node);
		readEnum(xml, node->type);
		addReader(new SceneNodeScalarReader<unsigned int>(&node->up));
		addReader(new SceneNodeVectorReader<float>(&node->axis));
		addReader(new SceneNodeScalarReader<bool>(&node->turn));
		addReader(new SceneNodeScalarReader<bool>(&node->center));
	}

	else if (xml->value == "primitive") {
		SceneNodePrimitive* node = new SceneNodePrimitive;
		push(xml, node);
		readEnum(xml, node->type);
		addReader(new SceneNodeVectorReader<unsigned int>(&node->index));
	}

	else if (xml->value == "geometry") {
		SceneNodeGeometry* node = new SceneNodeGeometry;
		push(xml, node);
		addReader(new SceneNodeGeometryReader(node, SceneNodeGeometry::Stipple));
		addReader(new SceneNodeGeometryReader(node, SceneNodeGeometry::Color));
		addReader(new SceneNodeGeometryReader(node, SceneNodeGeometry::TexCoord));
		addReader(new SceneNodeGeometryReader(node, SceneNodeGeometry::Normal));
		addReader(new SceneNodeGeometryReader(node, SceneNodeGeometry::Vertex));
	}

	else if (xml->value == "gstate") {
		SceneNodeGState* node = new SceneNodeGState;
		push(xml, node);
		addReader(new SceneNodeGStateReader(node));
	}

	else if (xml->value == "animate") {
		SceneNodeAnimate* node = new SceneNodeAnimate;
		push(xml, node);
		readEnum(xml, node->type);
		addReader(new SceneNodeScalarReader<std::string>(&node->src));
		addReader(new SceneNodeScalarReader<std::string>(&node->dst));
		addReader(new SceneNodeScalarReader<float>(&node->start));
		addReader(new SceneNodeScalarReader<float>(&node->end));
		addReader(new SceneNodeScalarReader<float>(&node->bias));
		addReader(new SceneNodeScalarReader<float>(&node->scale));
		addReader(new SceneNodeScalarReader<float>(&node->step));
		addReader(new SceneNodeScalarReader<float>(&node->cycles));
	}

	else if (xml->value == "parameters") {
		SceneNodeParameters* node = new SceneNodeParameters;
		push(xml, node);
		addReader(new SceneNodeVectorReader<std::string>(&node->src));
		addReader(new SceneNodeVectorReader<std::string>(&node->dst));
		addReader(new SceneNodeVectorReader<float>(&node->scale));
		addReader(new SceneNodeVectorReader<float>(&node->bias));
	}

	else if (xml->value == "choice") {
		SceneNodeChoice* node = new SceneNodeChoice;
		push(xml, node);
		addReader(new SceneNodeVectorReader<unsigned int>(&node->mask));
	}

	else if (xml->value == "lod") {
		SceneNodeLOD* node = new SceneNodeLOD;
		push(xml, node);
		readEnum(xml, node->type);
		addReader(new SceneNodeVectorReader<float>(&node->sphere));
		addReader(new SceneNodeVectorReader<float>(&node->range));
	}

	else if (xml->value == "light") {
		SceneNodeLight* node = new SceneNodeLight;
		push(xml, node);
		addReader(new SceneNodeVectorReader<float>(&node->ambient));
		addReader(new SceneNodeVectorReader<float>(&node->diffuse));
		addReader(new SceneNodeVectorReader<float>(&node->specular));
		addReader(new SceneNodeVectorReader<float>(&node->position));
		addReader(new SceneNodeVectorReader<float>(&node->spotDirection));
		addReader(new SceneNodeVectorReader<float>(&node->spotExponent));
		addReader(new SceneNodeVectorReader<float>(&node->spotCutoff));
		addReader(new SceneNodeVectorReader<float>(&node->attenuation));
	}

	else if (xml->value == "metadata") {
		SceneNodeMetadata* node = new SceneNodeMetadata;
		push(xml, node);
		addReader(new SceneNodeScalarReader<std::string>(&node->data));
	}

	else if (xml->value == "particlesystem") {
		SceneNodeParticleSystem* node = new SceneNodeParticleSystem;
		push(xml, node);
		addReader(new SceneNodeVectorReader<float>(&node->locationV));
		addReader(new SceneNodeVectorReader<float>(&node->velocityV));
		addReader(new SceneNodeVectorReader<float>(&node->startColorV));
		addReader(new SceneNodeVectorReader<float>(&node->endColorV));
		addReader(new SceneNodeScalarReader<float>(&node->startSizeV));
		addReader(new SceneNodeScalarReader<float>(&node->endSizeV));
		addReader(new SceneNodeVectorReader<float>(&node->gravityV));
		addReader(new SceneNodeScalarReader<float>(&node->speedV));
		addReader(new SceneNodeScalarReader<float>(&node->lifeV));
		addReader(new SceneNodeScalarReader<float>(&node->fieldAngleV));
		addReader(new SceneNodeScalarReader<float>(&node->attractionPercentV));
		addReader(new SceneNodeScalarReader<unsigned int>(&node->creationSpeedV));
		addReader(new SceneNodeScalarReader<unsigned int>(&node->burstSizeV));
		addReader(new SceneNodeScalarReader<unsigned int>(&node->spreadMinV));
		addReader(new SceneNodeScalarReader<unsigned int>(&node->spreadMaxV));
		addReader(new SceneNodeScalarReader<float>(&node->spreadFactorV));
		addReader(new SceneNodeParticleSystemReader(node));
	}

	// see if it's a reference
	else if (xml->value == "ref") {
		// ref node must be empty
		if (xml.begin() != xml.end())
			throw XMLIOException(xml->position,
								"`ref' must be empty");

		// get id
		std::string id;
		if (!xml->getAttribute("id", id))
			throw XMLIOException(xml->position,
								"`ref' must have an `id' attribute");

		// find the named node
		NodeMap::const_iterator index = namedNodes.find(id);
		if (index == namedNodes.end())
			throw XMLIOException(xml->position, 
								string_util::format(
								"unknown node `%s'", id.c_str()));
		SceneNode* refNode = index->second;

		// check for multiple top level nodes
		if (stack.empty())
			throw XMLIOException(xml->position,
								"only one top-level node allowed");

		// check for child of a leaf
		if (stack.back().group == NULL)
			throw XMLIOException(xml->position,
								"leaf node cannot have children");

		// add node as child of current group
		stack.back().group->pushChild(refNode);
		return;
	}

	else {
		// unrecognized
		throw XMLIOException(xml->position,
							string_util::format(
								"unrecognized node `%s'",
								xml->value.c_str()));
	}

	// descend
	XMLTree::sibling_iterator scan = xml.begin();
	XMLTree::sibling_iterator end  = xml.end();
	for (; scan != end; ++scan)
		parseNode(scan);

	// clean up
	pop();
}

void					SceneReader::pushCommon(
								XMLTree::iterator xml,
								SceneNode* newNode)
{
	// push item onto stack
	State state;
	state.node        = newNode;
	state.group       = NULL;
	state.activeField = NULL;	// FIXME -- obsolete?
	stack.push_back(state);

	// saved named nodes
	xml->getAttribute("id", xmlSetMethod(this, &SceneReader::saveNamedNode));
}

void					SceneReader::push(XMLTree::iterator xml,
								SceneNode* newNode)
{
	// if top level node then save it
	if (node == NULL) {
		assert(stack.empty());
		node = newNode;
	}

	// check for multiple top level nodes
	else if (stack.empty()) {
		newNode->unref();
		throw XMLIOException(xml->position, "only one top-level node allowed");
	}

	// check for child of a leaf
	else if (stack.back().group == NULL) {
		newNode->unref();
		throw XMLIOException(xml->position, "leaf node cannot have children");
	}

	// add node as child of current group
	else {
		// add to end of parent group
		stack.back().group->pushChild(newNode);

		// parent now owns node
		newNode->unref();
	}

	pushCommon(xml, newNode);
}

void					SceneReader::push(XMLTree::iterator xml,
								SceneNodeGroup* newGroupNode)
{
	// do regular node stuff
	push(xml, static_cast<SceneNode*>(newGroupNode));

	// save group node
	stack.back().group = newGroupNode;
}

void					SceneReader::pop()
{
	assert(!stack.empty());
	State& state = stack.back();

	// clean up
	for (FieldReaders::iterator index = state.fieldReaders.begin();
								index != state.fieldReaders.end(); ++index)
		delete index->second;

	// close
	stack.pop_back();
}

void					SceneReader::readEnum(
								XMLTree::iterator xml,
								SceneNodeSFEnum& field)
{
	// see if there's an attribute for the field
	std::string value;
	if (!xml->getAttribute(field.getName(), value))
		return;

	// match the value to one of the enumerations
	unsigned int num = field.getNumEnums();
	for (unsigned int i = 0; i < num; ++i)
		if (value == field.getEnum(i)) {
			field.set(i);
			return;
		}

	// not found
	throw XMLIOException(xml->position, 
								string_util::format(
								"unknown enumerant `%s'", value.c_str()));
}

void					SceneReader::addReader(
								SceneNodeFieldReader* reader)
{
	assert(reader != NULL);
	assert(!stack.empty());
	assert(stack.back().fieldReaders.count(reader->getName()) == 0);

	stack.back().fieldReaders.insert(std::make_pair(reader->getName(), reader));
}

void					SceneReader::saveNamedNode(const std::string& id)
{
	assert(!stack.empty());
	State& state = stack.back();

	// replace existing name or add new name
	NodeMap::iterator index = namedNodes.find(id);
	if (index != namedNodes.end())
		index->second = state.node;
	else
		namedNodes.insert(std::make_pair(id, state.node));

	// save id in node
	state.node->setID(id);
}
