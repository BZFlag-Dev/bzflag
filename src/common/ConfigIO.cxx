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

#include "ConfigIO.h"
#include "ErrorHandler.h"
#include "FileManager.h"
#include <assert.h>
#include <ctype.h>
#include <iostream>

ConfigReader::ConfigReader()
{
	// do nothing
}

ConfigReader::~ConfigReader()
{
	// do nothing
}

void					ConfigReader::push(
								OpenCallback open,
								CloseCallback close,
								DataCallback data)
{
	Callbacks cb;
	cb.open  = open;
	cb.close = close;
	cb.data  = data;
	callbacks.push_back(cb);
}

void					ConfigReader::pop()
{
	callbacks.pop_back();
}

BzfString				ConfigReader::escape(const BzfString& string)
{
	static const char* hex = "0123456789abcdef";

	// check for characters to escape
	const char* cstr = string.c_str();
	for (; *cstr; ++cstr)
		if (!isprint(*cstr))
			break;

	// if no unprintable characters then the input string is fine
	if (*cstr == '\0')
		return string;

	// construct new string with escapes
	BzfString output;
	for (cstr = string.c_str(); *cstr; ++cstr) {
		if (*cstr == '\t')
			output += "\\t";
		else if (*cstr == '\n')
			output += "\\n";
		else if (*cstr == '\r')
			output += "\\r";
		else if (!isprint(*cstr)) {
			output += "\\x";
			output.append(&hex[(*cstr >> 4) & 0x0f], 1);
			output.append(&hex[*cstr & 0x0f], 1);
		}
		else
			output.append(cstr, 1);
	}
	return output;
}

bool					ConfigReader::read(istream& s, void* userData)
{
	isPutback = false;
	return read(&s, "", userData);
}

BzfString				ConfigReader::getPosition() const
{
	assert(!positions.empty());
	assert(positions.back().stream != NULL);

	// accumulate includes
	BzfString pos;
	unsigned int n = positions.size();
	for (unsigned int i = 0; i < n - 1; ++i) {
		const Position& position = positions[i];
		if (position.filename.empty())
			pos += BzfString::format("File included at line %d, column %d;\n",
								position.line, position.column);
		else
			pos += BzfString::format("%s, file included at line %d, column %d;\n",
								position.filename.c_str(),
								position.line, position.column);
	}

	// last (i.e. current) file
	const Position& position = positions.back();
	if (position.filename.empty())
		pos += BzfString::format("At line %d, column %d",
								position.line, position.column);
	else
		pos += BzfString::format("%s, line %d, column %d",
								position.filename.c_str(),
								position.line, position.column);

	return pos;
}

bool					ConfigReader::read(istream* stream,
								const BzfString& filename, void* userData)
{
	// prepare position
	positions.push_back(Position());
	Position& position = positions.back();
	position.stream    = stream;
	position.filename  = filename;
	position.line      = 1;
	position.column    = 0;

	const bool result = doRead(userData);

	// pop position
	assert(!positions.empty());
	positions.pop_back();

	return result;
}

bool					ConfigReader::doRead(void* userData)
{
	assert(callbacks.size() >= 1);

#ifndef NDEBUG
	// we should end with same number of callbacks as we started with
	const int startSize = (int)callbacks.size();
#endif

	// parse
	while (!eof()) {
		char c = get();
		unget(c);
		while (!eof() && c != '<') {
			// read data until we find a non-commented open bracket or new line
			BzfString data = readData();

			// callback with data
			if (!data.empty() && callbacks.back().data != NULL)
				if (!(*callbacks.back().data)(this, data, userData))
					return false;

			// peek at next character
			c = get();
			unget(c);
		}

		// read the tag
		BzfString tag;
		Values values;
		Type type = readTag(&tag, &values);
		if (type == End)
			break;

		// open new state level
		if (type == Open || type == OpenClose) {
			// push state
			Position& position = positions.back();
			position.state.push_back(std::make_pair(tag, (int)callbacks.size()));

			// handle include directives
			if (tag == "include") {
				// get filename
				Values::const_iterator index = values.find("filename");
				if (index == values.end()) {
					printError("%s: include tag missing filename",
								getPosition().c_str());
					return false;
				}

				// open file
				istream* included = FILEMGR->createDataInStream(index->second);
				if (!included) {
					printError("%s: cannot open file %s",
								getPosition().c_str(),
								index->second.c_str());
					return false;
				}

				// include it
				if (!read(included, index->second, userData)) {
					delete included;
					return false;
				}
				delete included;
			}

			// invoke open callback
			else if (callbacks.back().open != NULL) {
				if (!(*callbacks.back().open)(this, tag, values, userData))
					return false;
			}
		}

		// close current level
		if (type == Close || type == OpenClose) {
			// open and close tags must match
			Position& position = positions.back();
			if (position.state.empty() || position.state.back().first != tag) {
				printError("%s: mismatched tag %s, expected %s",
								getPosition().c_str(), tag.c_str(),
								position.state.back().first.c_str());
				return false;
			}

			// pop stack back to level when open tag was encountered
			assert(position.state.back().second > 0);
			assert((int)callbacks.size() >= position.state.back().second);
			callbacks.erase(callbacks.begin() + position.state.back().second,
														callbacks.end());
			position.state.pop_back();

			// invoke close callback
			if (tag != "include" && callbacks.back().close != NULL)
				if (!(*callbacks.back().close)(this, tag, userData))
					return false;
		}
	}

	// verify we've closed correctly
	Position& position = positions.back();
	if (!position.state.empty()) {
		printError("%s: unmatched tag: %s",
								getPosition().c_str(),
								position.state.back().first.c_str());
		return false;
	}
	assert((int)callbacks.size() == startSize);
	assert(!isPutback);

	return true;
}

ConfigReader::Type		ConfigReader::readTag(BzfString* tag, Values* values)
{
	// check for end of file or error
	char c = get();
	if (c != '<') {
		if (!eof())
			printError("%s: read error", getPosition().c_str());
		return End;
	}

	// read close character if it's there
	Type type = Open;
	skipWhitespace();
	c = get();
	if (c == '/')
		type = Close;
	else
		unget(c);

	// read tag
	BzfString data;
	data = readToken();
	if (data.empty())
		return End;
	*tag = data;

	// read values
	if (!readValues(values))
		return End;
	if (values->size() > 0 && type != Open) {
		printError("%s: closing tags may not have parameters",
								getPosition().c_str());
		return End;
	}

	// read close character if it's there
	skipWhitespace();
	c = get();
	if (c == '/') {
		if (type == Close) {
			printError("%s: invalid character in tag: %c",
								getPosition().c_str(), c);
			return End;
		}
		else {
			type = OpenClose;
			skipWhitespace();
		}
	}
	else {
		unget(c);
	}

	// read closing bracket
	c = get();
	if (c != '>') {
		printError("%s: expected >", getPosition().c_str());
		return End;
	}

	return type;
}

BzfString				ConfigReader::readData()
{
	BzfString data;
	char c = get();
	while (okay()) {
		// if we found a comment then skip rest of line and return what
		// we got before the comment
		if (c == '#') {
			skipTo("\n");
			break;
		}

		// if we found a tag then put back the tag open and return data
		if (c == '<') {
			unget(c);
			break;
		}

		// if we found the end of a line then return data
		if (c == '\n')
			break;

		// check for escapes
		if (c == '\\') {
			c = get();
			if (!okay())
				break;

			switch (c) {
				case 't':  c = '\t'; break;
				case 'n':  c = '\n'; break;
				case 'r':  c = '\r'; break;
				case '\\': c = '\\'; break;
				case '\"': c = '\"'; break;
				case '<':  break;
				case 'x': {
					char ch, cl;
					if (!isxdigit(ch = get()) || !okay() ||
				!isxdigit(cl = get()) || !okay()) {
						printError("%s: invalid hex escape", getPosition().c_str());
						c = '?';
						break;
					}
					c = static_cast<char>((fromhex(ch) << 4) | fromhex(cl));
					break;
				}
			}
		}

		// accumulate data
		data.append(&c, 1);

		// next character
		c = get();
	}

	return data;
}

char					ConfigReader::skipTo(const char* set)
{
	while (okay()) {
		char c = get();
		if (strchr(set, c) != NULL)
			return c;
	}
	return '\0';
}

void					ConfigReader::skipWhitespace()
{
	while (okay()) {
		char c = get();
		if (!isspace(c)) {
			if (c == '#') {
				skipTo("\n");
			}
			else {
				unget(c);
				return;
			}
		}
	}
}

BzfString				ConfigReader::readToken()
{
	skipWhitespace();

	BzfString buffer;
	while (okay()) {
		char c = get();
		if (!isalnum(c)) {
			unget(c);
			break;
		}
		buffer.append(&c, 1);
	}

	if (buffer.empty() || !isalpha(buffer.c_str()[0])) {
		printError("%s: invalid token", getPosition().c_str());
		return BzfString();
	}

	return buffer;
}

bool					ConfigReader::readValues(Values* values)
{
	// values are in the form: name=value
	// value may be delimited by double quotes or by whitespace.  name is
	// alphanumeric except the first character which may not be numeric.
	// this method reads name/value pairs until it reaches a character
	// that can't be a name or encounters an error.
	while (okay()) {
		// skip to name
		skipWhitespace();

		// verify first character.  return if it can't be a name.
		if (!okay())
			break;
		char c = get();
		unget(c);
		if (!isalpha(c))
			return true;

		// read name
		BzfString name = readToken();
		if (name.empty())
			return false;

		// next non-whitespace must be `='
		skipWhitespace();
		if (!okay())
			break;
		c = get();
		if (c != '=') {
			printError("%s: expected =", getPosition().c_str());
			return false;
		}
		skipWhitespace();

		// next character is ", >, /, or beginning of value
		if (!okay())
			break;
		c = get();
		if (c == '>' || c == '/') {
			printError("%s: expected >", getPosition().c_str());
			return false;
		}

		BzfString value;
		if (c == '\"') {
			if (!readQuoted(&value))
				return false;
		}
		else {
			unget(c);
			if (!readUnquoted(&value))
				return false;
		}

		// insert value
		values->insert(std::make_pair(name, value));
	}

	// stream is in error or reached EOF.  both are bad.
	return false;
}

bool					ConfigReader::readQuoted(BzfString* value)
{
	// read until ending quote.  escape certain characters.
	bool escaped = false;
	*value = "";
	while (okay()) {
		char c = get();
		if (escaped) {
			switch (c) {
				case 't':  c = '\t'; break;
				case 'n':  c = '\n'; break;
				case 'r':  c = '\r'; break;
				case '\\': c = '\\'; break;
				case '\"': c = '\"'; break;
				case 'x': {
					char ch, cl;
					if (!okay() || !isxdigit(ch = get()) ||
				!okay() || !isxdigit(cl = get())) {
						printError("%s: invalid hex escape", getPosition().c_str());
						return false;
					}
					c = static_cast<char>((fromhex(ch) << 4) | fromhex(cl));
					break;
				}
			}
			value->append(&c, 1);
			escaped = false;
		}
		else if (c == '\\') {
			escaped = true;
		}
		else if (c == '\"') {
			return true;
		}
		else if (c == '\n') {
			printError("%s: newline in string", getPosition().c_str());
			return false;
		}
		else {
			value->append(&c, 1);
		}
	}
	return false;
}

bool					ConfigReader::readUnquoted(BzfString* value)
{
	// read until next whitespace or / or >
	*value = "";
	while (okay()) {
		char c = get();
		if (isspace(c) || c == '#' || c == '/' || c == '>') {
			unget(c);
			return true;
		}
		value->append(&c, 1);
	}
	return false;
}

char					ConfigReader::get()
{
	assert(!positions.empty());
	assert(positions.back().stream != NULL);

	if (isPutback) {
		isPutback = false;
		return putback;
	}
	else {
		Position& position = positions.back();
		char c;
		position.stream->get(c);
		if (c == '\t') {
			position.column = (position.column & 7) + 9;
		}
		else if (c == '\n') {
			position.column = 0;
			++position.line;
		}
		else {
			++position.column;
		}
		return c;
	}
}

void					ConfigReader::unget(char c)
{
	assert(!isPutback);
	isPutback = true;
	putback   = c;
}

void					ConfigReader::skip()
{
	get();
}

bool					ConfigReader::okay()
{
	assert(!positions.empty());
	assert(positions.back().stream != NULL);
	return !positions.back().stream->fail() && !positions.back().stream->eof();
}

bool					ConfigReader::eof()
{
	assert(!positions.empty());
	assert(positions.back().stream != NULL);
	return positions.back().stream->eof();
}

bool					ConfigReader::fail()
{
	assert(!positions.empty());
	assert(positions.back().stream != NULL);
	return positions.back().stream->fail();
}

unsigned char			ConfigReader::fromhex(char hexDigit)
{
	switch (hexDigit) {
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'A':
		case 'a': return 10;
		case 'B':
		case 'b': return 11;
		case 'C':
		case 'c': return 12;
		case 'D':
		case 'd': return 13;
		case 'E':
		case 'e': return 14;
		case 'F':
		case 'f': return 15;
	}
	assert(0 && "bad hex digit");
	return 0;
}
