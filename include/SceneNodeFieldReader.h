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
#ifndef BZF_SCENE_NODE_FIELD_READER_H
#define BZF_SCENE_NODE_FIELD_READER_H

#include "XMLTree.h"
#include "SceneNode.h"
#include <vector>

//
// SceneNodeFieldReader
//

class SceneNodeFieldReader {
public:
	SceneNodeFieldReader();
	virtual ~SceneNodeFieldReader();

	virtual bool		parse(XMLTree::iterator);
	virtual const char*	getName() const = 0;

protected:
	virtual void		parseBegin(XMLTree::iterator);
	virtual void		parseData(XMLTree::iterator, const std::string&);
	virtual void		parseEnd(XMLTree::iterator);
};

//
// SceneNodeScalarReader<T>
//

template<class T>
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

template<class T>
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
	typedef std::vector<T>	Buffer;
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

#endif
// ex: shiftwidth=4 tabstop=4
