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

#include "OggAudioFile.h"

OggAudioFile::OggAudioFile(std::istream* in) : AudioFile(in)
{
	stream = -1;

	ov_callbacks cb;
	cb.read_func = OAFRead;
	cb.seek_func = OAFSeek;
	cb.close_func = OAFClose;
	cb.tell_func = OAFTell;

	ov_open_callbacks(in, &file, NULL, 0, cb);
	if (!&file) {
		cerr << "OggAudioFile() failed: call to ov_open_callbacks failed\n";
		open = false;
	}
	else {
		open = true;
	}
	info = ov_info(&file, -1);
}

OggAudioFile::~OggAudioFile()
{
	open = false;
	ov_clear(&file);
}

std::string	OggAudioFile::getExtension()
{
	return ".ogg";
}

bool		OggAudioFile::read(void* buffer, int numFrames)
{
	if (!open)
		return false;
	int frames;
	int bytes = numFrames * 2;
	int stream = -1;
	frames = ov_read(&file, (char *) buffer, bytes, 0, 2, 1, &stream);
	if (frames < 0)
		return false;
	return true;
}

size_t	OAFRead(void* ptr, size_t size, size_t nmemb, void* datasource)
{
	std::istream *in = (std::istream*) datasource;
	in->read(ptr, size * nmemb);
	return size * nmemb;
}

int		OAFSeek(void*, ogg_int64_t, int)
{
	// return -1 always, disabling seeking
	return -1;
}

int		OAFClose(void*)
{
	// technically we should close here, but this is handled outside

//	std::istream *in = (std::istream*) datasource;
//	in->close();
	return 0;
}

long		OAFTell(void*)
{
	// do nothing, since seeking isn't implemented
	return 0;
}
// ex: shiftwidth=4 tabstop=4
