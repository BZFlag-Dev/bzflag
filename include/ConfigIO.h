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

#ifndef BZF_CONFIG_IO_H
#define BZF_CONFIG_IO_H

#include "common.h"
#include "BzfString.h"
#include "bzfio.h"
#include <map>
#include <vector>

class ConfigReader {
public:
	typedef std::map<BzfString, BzfString> Values;

	typedef bool (*OpenCallback)(ConfigReader*, const BzfString& tag,
							const Values&, void* userData);
	typedef bool (*CloseCallback)(ConfigReader*,
							const BzfString& tag, void* userData);
	typedef bool (*DataCallback)(ConfigReader*,
							const BzfString& data, void* userData);

	ConfigReader();
	~ConfigReader();

	// push/pop callbacks to handle various tokens in a stream.  a NULL
	// callback is allowed and simply prevents that callback from being
	// invoked.  so, for example, if you didn't want the data between
	// two tags, you'd push(openCB, closeCB, NULL) in the open callback
	// for the tag.  openCB and closeCB will handle any tags within the
	// tag but any data is discarded.
	void				push(OpenCallback, CloseCallback, DataCallback);
	void				pop();

	// only one call to read() may be active at a time.  the current
	// open callback is invoked when a tag is encountered and the
	// current close callback will be invoked when its matching close
	// tag is encountered, popping any callbacks added since the open.
	// if a tag has the form <tag/> then it both opens and closes the
	// tag and both callbacks are called.  if any data is encountered
	// between the open and close tags, the data callback is invoked
	// for each line of that data.  no whitespace except the newline
	// is stripped from the data.
	bool				read(istream&, void* userData);

	// get the current position (valid in read())
	BzfString			getPosition() const;

	// escape a string so it will be correctly read by read() if
	// it's contained in double quotes.
	static BzfString	escape(const BzfString& string);

private:
	enum Type { End, Open, Close, OpenClose };
	bool				read(istream* stream,
							const BzfString& filename, void* userData);
	bool				doRead(void* userData);
	BzfString			readData();
	Type				readTag(BzfString* tag, Values* values);
	BzfString			readToken();
	bool				readQuoted(BzfString*);
	bool				readUnquoted(BzfString*);
	bool				readValues(Values*);
	void				skipWhitespace();
	char				skipTo(const char* set);

	char				get();
	void				unget(char);
	void				skip();
	bool				okay();
	bool				eof();
	bool				fail();

	static unsigned char fromhex(char hexDigit);

private:
	typedef std::vector<std::pair<BzfString, int> > StateStack;
	struct Position {
	public:
		istream*		stream;
		BzfString		filename;
		int				line;
		int				column;
		StateStack		state;
	};
	struct Callbacks {
	public:
		OpenCallback	open;
		CloseCallback	close;
		DataCallback	data;
	};
	typedef std::vector<Callbacks> CallbackStack;
	typedef std::vector<Position> PositionStack;

	char				putback;
	bool				isPutback;

	PositionStack		positions;
	CallbackStack		callbacks;
	StateStack			state;
};

#endif
