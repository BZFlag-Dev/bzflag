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

#include "common.h"
#include <stack>
#include <iostream>
#include <string.h>
#include <ctype.h>
#include "XMLTree.h"
#include "FileManager.h"

//
// common enumerations
//

const XMLParseEnumList<bool> s_xmlEnumBool[] = {
	{ "false", false },
	{ "true",  true  },
	{ NULL, false }
};


//
// XMLStreamPosition
//

XMLStreamPosition::XMLStreamPosition() :
								filename(),
								line(1),
								column(1)
{
	// do nothing
}

XMLStreamPosition::XMLStreamPosition(const std::string& filename_,
								unsigned int line_, unsigned int column_) :
								filename(filename_),
								line(line_),
								column(column_)
{
	// do nothing
}

void					XMLStreamPosition::swap(XMLStreamPosition& p)
{
	int tmp;
	tmp      = line;
	line     = p.line;
	p.line   = tmp;
	tmp      = column;
	column   = p.column;
	p.column = tmp;
	filename.swap(p.filename);
}


//
// XMLIOException
//

XMLIOException::XMLIOException(const XMLStreamPosition& p,
								const char* msg) :
								runtime_error(msg),
								position(p)
{
	// do nothing
}

XMLIOException::XMLIOException(const XMLStreamPosition& p,
								const std::string& msg) :
								runtime_error(msg.c_str()),
								position(p)
{
	// do nothing
}


//
// XMLNode
//

void					XMLNode::swap(XMLNode& n)
{
	Type tmp = type;
	type     = n.type;
	n.type   = tmp;
	value.swap(n.value);
	attributes.swap(n.attributes);
	position.swap(n.position);
}

bool					XMLNode::getAttribute(
							const std::string& name,
							std::string& value) const
{
	Attributes::const_iterator index = attributes.find(name);
	if (index == attributes.end())
		return false;
	value = index->second.second;
	return true;
}

XMLStreamPosition		XMLNode::getAttributePosition(
							const std::string& name) const
{
	Attributes::const_iterator index = attributes.find(name);
	if (index == attributes.end())
		return XMLStreamPosition();
	return index->second.first;
}


//
// XMLTree::XMLStream
//

XMLTree::XMLStream::XMLStream(istream* s) :
								position(),
								stream(s),
								eofIsOkay(false)
{
	// do nothing
}

XMLTree::XMLStream::XMLStream(istream* s, const XMLStreamPosition& p) :
								position(p),
								stream(s),
								eofIsOkay(false)
{
	// do nothing
}

char					XMLTree::XMLStream::get()
{
	// get next character.  if stream runs out then pop the stack and retry.
	char c = stream->get();
	if (stream->eof() && !streamStack.empty()) {
		pop();
		c = stream->get();
	}

	// check for unexpected end of stream
	if (stream->fail() && !eofIsOkay) {
		setFail();
		throw XMLIOException(position, "unexpected end of stream");
	}

	// advance position
	if (c == '\n') {
		++position.line;
		position.column = 1;
	}
	else if (c == '\t') {
		position.column = ((position.column - 1) & ~7) + 9;
	}
	else {
		++position.column;
	}

	return c;
}

char					XMLTree::XMLStream::peek()
{
	// get next character.  if stream runs out then pop the stack and retry.
	char c = stream->peek();
	if (stream->eof() && !streamStack.empty()) {
		pop();
		c = stream->peek();
	}

	// check for unexpected end of stream
	if (stream->fail() && !eofIsOkay) {
		setFail();
		throw XMLIOException(position, "unexpected end of stream");
	}

	return c;
}

void					XMLTree::XMLStream::skip()
{
	get();
}

XMLTree::XMLStream::operator void*() const
{
	return stream->fail() ? 0 : (void*)this;
}

bool					XMLTree::XMLStream::operator!() const
{
	return stream->fail();
}

void					XMLTree::XMLStream::setFail()
{
	stream->setstate(std::ios::failbit);
}

void					XMLTree::XMLStream::push(istream* s)
{
	push(s, XMLStreamPosition());
}

void					XMLTree::XMLStream::push(istream* stream_,
							const XMLStreamPosition& position_)
{
	// save current stream and position
	streamStack.push_back(std::make_pair(stream, position));

	// update stream and position
	stream   = stream_;
	position = position_;
}

void					XMLTree::XMLStream::pop()
{
	assert(!streamStack.empty());

	// we own all streams except the top-most
	delete stream;

	// restore saved stream and position
	stream   = streamStack.back().first;
	position = streamStack.back().second;
	streamStack.pop_back();
}


//
// XMLTree::XMLTreeIOException
//

XMLTree::XMLTreeIOException::XMLTreeIOException(
								const XMLStream& s, const char* msg) :
								XMLIOException(s.position, msg)
{
	// do nothing
}

XMLTree::XMLTreeIOException::XMLTreeIOException(
								const XMLStream& s, const std::string& msg) :
								XMLIOException(s.position, msg)
{
	// do nothing
}


//
// XMLTree
//

XMLTree::XMLTree()
{
	// do nothing
}

XMLTree::XMLTree(const XMLTree& t) : tree<XMLNode>(t)
{
	// do nothing
}

XMLTree::~XMLTree()
{
	// do nothing
}

void					XMLTree::read(XMLStream& stream, XMLTree::iterator root)
{
	std::stack<XMLTree::iterator> tagStack;
	tagStack.push(root);

	// read XML elements.  we relax the XML requirement for exactly one
	// top-level element.
	XMLNode node;
	XMLTree::NodeType type = readNode(stream, &node, true);
	while (type != End) {
		switch (type) {
			case End:
				assert(0);
				return;

			case Open: {
				// open tag
				XMLTree::iterator newNode =
								append_child(tagStack.top(), XMLNode());
				newNode->swap(node);
				tagStack.push(newNode);
				break;
			}

			case Empty: {
				XMLTree::iterator newNode =
								append_child(tagStack.top(), XMLNode());
				newNode->swap(node);
				break;
			}

			case Close:
				// verify that tags match
				if (tagStack.size() <= 1) {
					stream.setFail();
					throw XMLTreeIOException(stream,
							string_util::format(
								"</%s> without matching <%s>",
								node.value.c_str(),
								node.value.c_str()));
				}
				if (node.value != tagStack.top()->value) {
					stream.setFail();
					throw XMLTreeIOException(stream,
							string_util::format(
								"unexpected tag </%s>, expected </%s>",
								node.value.c_str(),
								tagStack.top()->value.c_str()));
				}

				// correct match.  pop stack.
				tagStack.pop();
				break;

			case Data: {
				// append data
				XMLTree::iterator newNode =
								append_child(tagStack.top(), XMLNode());
				newNode->swap(node);
				break;
			}

			case Include:
			break;
		}
		type = readNode(stream, &node, tagStack.size() <= 1);
	}
}

XMLTree::NodeType		XMLTree::readNode(XMLStream& stream,
								XMLNode* node,
								bool eofIsOkay)
{
	char c;

	// discard leading whitespace
retry:
	if (eofIsOkay)
		stream.setEOFIsOkay(true);
	skipWhitespace(stream);
	c = stream.get();
	if (eofIsOkay)
		stream.setEOFIsOkay(false);
	if (!stream)
		return End;

	// initialize node
	node->type     = XMLNode::Data;
	node->value    = "";
	node->position = stream.position;
	node->attributes.clear();

	// check for tag
	if (!stream) {
		return End;
	}
	else if (c == '<') {
		// next character reveals all
		c = stream.peek();
		switch (c) {
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				// error -- whitespace before name
				throw XMLTreeIOException(stream,
							"syntax error -- whitespace before tag name");

			case '!': {
				// comment or CDATA (or our custom INCLUDE)
				stream.skip();
				switch (stream.get()) {
					case '-':
						readComment(stream);
						goto retry;

					case '[':
						return readCDATA(stream, node);

					case 'I':
						readINCLUDE(stream);
						goto retry;

					default:
						throw XMLTreeIOException(stream, "malformed tag");
				}
			}

			case '?': {
				// processing instruction.  read target.
				stream.skip();
				node->value = readToken(stream,
								isNameFirstChar, isNameChar,
								" \n\r\t", "target");

				// discard whitespace
				skipWhitespace(stream);

				// read instruction up to ?>
				std::string data;
				int match = 0;
				while (stream && match != 2) {
					c = stream.get();
					if (c == '?') {
						if (match == 1)
							data += '?';	// found ??, add first ?
						else
							match = 1;		// found [^?]?
					}
					else if (c == '>') {
						if (match == 1)
							match = 2;		// found end of PI
						else
							data += c;		// found [^?]>
					}
					else {
						data += c;			// found [^?>]
						match = 0;
					}
				}

				// append instructions to target
				node->value += " ";
				node->value += data;

				node->type = XMLNode::PI;
				return Data;
			}

			case '/':
				// closing tag
				stream.skip();
				node->value = readToken(stream,
								isNameFirstChar, isNameChar,
								" \n\r\t>", "tag");

				// eat tag close
				skipWhitespace(stream);
				if (stream.get() != '>')
					throw XMLTreeIOException(stream,
							"syntax error -- invalid character in close tag");
				return Close;

			default: {
				// open or empty tag
				NodeType type = Open;
				node->value = readToken(stream,
								isNameFirstChar, isNameChar,
								" \n\r\t/>", "tag");
				readParameters(stream, node, "/>");

				// eat close
				skipWhitespace(stream);
				c = stream.get();
				if (c == '/') {
					type = Empty;
					c    = stream.get();
				}
				if (c != '>')
					throw XMLTreeIOException(stream,
							"syntax error -- invalid character in open tag");

				node->type = XMLNode::Tag;
				return type;
			}
		}
	}

	// not a tag so it must be data.  read up to a tag.  compress
	// whitespace and handle references.
	else {
		node->value += c;
		bool didWhitespace = isSpace(c);
		while (stream && (c = stream.peek()) != '<') {
			c = stream.get();
			if (c == '&') {
				// found a reference
				c              = readReference(stream);
				didWhitespace  = isSpace(c);
				node->value   += c;
			}
			else if (!didWhitespace) {
				// character after non-whitespace
				didWhitespace = isSpace(c);
				if (didWhitespace)
					node->value += " ";
				else
					node->value += c;
			}
			else if (!isSpace(c)) {
				// non-whitespace after whitespace
				didWhitespace  = false;
				node->value   += c;
			}
		}
		node->type = XMLNode::Data;
		return Data;
	}
}

void					XMLTree::readComment(XMLStream& stream)
{
	eat(stream, "-", "malformed comment");

	// comment -- skip up to and including first `-->'
	int match = 0;
	while (match != 3) {
		char c = stream.get();
		if (c != '-') {
			if (match == 2)
				if (c == '>')
					++match;
				else
					throw XMLTreeIOException(stream,
						"the string -- may not appear in comments");
			else
				match = 0;
		}
		else {
			if (match < 2)
				++match;
			else
				throw XMLTreeIOException(stream,
					"the string -- may not appear in comments");
		}
	}
}

XMLTree::NodeType		XMLTree::readCDATA(XMLStream& stream,
							XMLNode* node)
{
	eat(stream, "CDATA[", "malformed CDATA section");

	// accumulate data up to but not including first `]]>'
	std::string data;
	int match = 0;
	while (match != 3) {
		char c = stream.get();
		if (c != ']') {
			if (match == 2 && c == '>')
				++match;
			else
				match = 0;
		}
		else {
			if (match < 2)
				++match;
		}
	}

	// save data without the ]]> suffix
	node->type  = XMLNode::PI;
	node->value = data.substr(0, data.size() - 3);
	return Data;
}

void					XMLTree::readINCLUDE(XMLStream& stream)
{
	eat(stream, "NCLUDE", "malformed INCLUDE");

	// read filename
	skipWhitespace(stream);
	std::string filename = readValue(stream);

	// eat tag close
	skipWhitespace(stream);
	eat(stream, "/>", "malformed INCLUDE");

	// open the file
	istream* included = FILEMGR->createDataInStream(filename);
	if (included == NULL)
		throw XMLTreeIOException(stream,
							string_util::format(
								"cannot open INCLUDE file `%s'",
								filename.c_str()));

	// push the stream
	stream.push(included, XMLStreamPosition(filename));
}

std::string				XMLTree::readToken(XMLStream& stream,
							CharClass firstCharClass,
							CharClass otherCharClass,
							const char* delimCharClass,
							const char* tokenType)
{
	char c;
	std::string token;

	// first character is special
	c = stream.get();
	if (!firstCharClass(c))
		throw XMLTreeIOException(stream,
							string_util::format("invalid %s", tokenType));
	token += c;

	// remaining characters
	while (stream) {
		c = stream.peek();
		if (otherCharClass(c))
			token += stream.get();
		else if (strchr(delimCharClass, c) != NULL)
			break;
		else
			throw XMLTreeIOException(stream,
							string_util::format("invalid %s", tokenType));
	}

	return token;
}

std::string				XMLTree::readValue(XMLStream& stream)
{
	// get quote
	char delim = stream.get();
	if (delim != '\"' && delim != '\'')
		throw XMLTreeIOException(stream, "attribute value needs quotes");

	// read until we find a matching quote or an invalid character
	std::string value;
	while (stream) {
		char c = stream.get();
		if (c == delim) {
			break;
		}
		else if (c == '<') {
			// error -- invalid character
			throw XMLTreeIOException(stream,
							"syntax error -- invalid character in attribute value");
		}
		else if (c == '&') {
			// reference
			value += readReference(stream);
		}
		else {
			value += c;
		}
	}

	return value;
}

void					XMLTree::readParameters(
								XMLStream& stream, XMLNode* node,
								const char* delims)
{
	while (stream) {
		// skip leading whitespace
		skipWhitespace(stream);

		// return if next character delimits end of attribute
		if (strchr(delims, stream.peek()) != NULL)
			return;

		// read name up to =
		XMLStreamPosition position(stream.position);
		std::string name = readToken(stream,
								isNameFirstChar, isNameChar,
								"=", "attribute name");

		// skip =
		if (stream.get() != '=')
			throw XMLTreeIOException(stream,
							"syntax error -- expected = in attribute");

		// read value
		std::string value = readValue(stream);

		// save it
		node->attributes[name] = std::make_pair(position, value);
	}
}

char					XMLTree::readReference(XMLStream& stream)
{
	// handle character references
	char c = stream.peek();
	if (c == '#') {
		stream.skip();
		return readCharReference(stream);
	}

	// read up to a semicolon
	std::string name = readToken(stream,
							isNameFirstChar, isNameChar,
							";", "reference");

	// verify delimiter
	if (stream.get() != ';')
		throw XMLTreeIOException(stream,
							string_util::format(
								"reference `%s' missing delimiter",
								name.c_str()));

	// handle it
	if (name == "amp")
		return '&';
	if (name == "lt")
		return '<';
	if (name == "gt")
		return '>';
	if (name == "apos")
		return '\'';
	if (name == "quot")
		return '\"';

	// unknown reference
	throw XMLTreeIOException(stream,
							string_util::format(
								"unknown reference &%s;", name.c_str()));
}

char					XMLTree::readCharReference(XMLStream& stream)
{
	// slurp up characters
	int base;
	std::string number;
	char c = stream.peek();
	if (c == 'x') {
		stream.skip();
		number = readToken(stream, isHexDigit, isHexDigit, ";", "reference");
		base   = 16;
	}
	else {
		number = readToken(stream, isDigit, isDigit, ";", "reference");
		base   = 10;
	}

	// verify delimiter
	if (stream.get() != ';')
		throw XMLTreeIOException(stream,
							string_util::format(
								"reference `%s' missing delimiter",
								number.c_str()));

	// parse number
	unsigned int value = strtoul(number.c_str(), NULL, base);

	// check range
	if (value > 255)
		throw XMLTreeIOException(stream,
							string_util::format(
								"character reference `%s' out of range",
								number.c_str()));

	return static_cast<char>(value);
}

void					XMLTree::eat(XMLStream& stream,
							const char* skip, const char* err)
{
	while (stream && *skip)
		if (stream.get() != *skip++)
			throw XMLTreeIOException(stream, err);
}

void					XMLTree::skipWhitespace(XMLStream& stream)
{
	while (stream && isSpace(stream.peek()))
		stream.skip();
}

bool					XMLTree::isLetter(char c)
{
	return isalpha(c);
}

bool					XMLTree::isDigit(char c)
{
	return isdigit(c);
}

bool					XMLTree::isHexDigit(char c)
{
	return isxdigit(c);
}

bool					XMLTree::isSpace(char c)
{
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

bool					XMLTree::isNameFirstChar(char c)
{
	return (isLetter(c) || c == '_' || c == ':');
}

bool					XMLTree::isNameChar(char c)
{
	return (isLetter(c) || isDigit(c) ||
							c == '.' || c == '-' ||
							c == '_' || c == ':');
}

istream&				XMLTree::read(istream& is,
							const XMLStreamPosition& position)
{
	return read(is, position, begin());
}

istream&				XMLTree::read(istream& is,
							const XMLStreamPosition& position,
							XMLTree::iterator node)
{
	// construct stream wrapper
	XMLTree::XMLStream stream(&is, position);

	// if tree is empty then add a dummy root node.  this is because
	// the tree class can have only one root node but we want to allow
	// multiple top-level XML items.  clients should ignore this node.
	if (size() == 0) {
		node = append_child(node, XMLNode());
		node->type = XMLNode::Data;
	}

	// parse
	read(stream, node);

	return is;
}

istream&				operator>>(istream& is, XMLTree& xml)
{
	// construct stream wrapper
	XMLTree::XMLStream stream(&is);

	// clear out existing tree
	xml.clear();

	// parse
	xml.read(stream, xml.begin());

	return is;
}

/*
ostream& operator<<(ostream& os, const XMLTree& tree)
{
	// FIXME
}
*/

std::string				XMLTree::escape(const std::string& string)
{
	static const char* hex = "0123456789abcdef";

	// check for characters to escape
	const char* cstr = string.c_str();
	for (; *cstr; ++cstr)
		if (!isprint(*cstr) &&
			*cstr != '\n' &&
			*cstr != '\r' &&
			*cstr != '\t' &&
			*cstr != '&' &&
			*cstr != '<' &&
			*cstr != '>' &&
			*cstr != '\'' &&
			*cstr != '\"')
			break;

	// if no unprintable characters then the input string is fine
	if (*cstr == '\0')
		return string;

	// construct new string with escapes
	std::string output;
	for (cstr = string.c_str(); *cstr; ++cstr) {
		switch (*cstr) {
			case '\n':
			case '\r':
			case '\t':
			case '&':
			case '<':
			case '>':
			case '\'':
			case '\"':
				output += "&#x";
				output += hex[(*cstr >> 4) & 0x0f];
				output += hex[*cstr & 0x0f];
				output += ";";
				break;

			default:
				output += *cstr;
				break;
		}
	}
	return output;
}
