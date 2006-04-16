/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <string>
#include <vector>

struct BatchItem
{
	std::string font;
	int size;
	bool antiAlias;
	bool bold;
	bool italic;
	std::string filename;
};

class TextToolBatch
{
public:
	TextToolBatch( std::string file );
	TextToolBatch();
	~TextToolBatch();

	void loadFile( std::string file );

	bool getNext( BatchItem &item );

	void error( std::string msg );

private:
	unsigned int position;
	std::vector < BatchItem > items;
};
