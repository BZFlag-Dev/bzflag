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

#include <iostream>
#include <fstream>
#include "OggAudioFile.h"

OggAudioFile::OggAudioFile(std::istream* in) : AudioFile(in)
{
	stream = -1;

	ov_callbacks cb;
	cb.read_func = OAFRead;
	cb.seek_func = OAFSeek;
	cb.close_func = OAFClose;
	cb.tell_func = OAFTell;

	if(ov_open_callbacks(in, &file, NULL, 0, cb) < 0) {
		std::cout << "OggAudioFile() failed: call to ov_open_callbacks failed\n";
	}
	else {
		info = ov_info(&file, -1);
		int samples = ov_pcm_total(&file, -1);
		init(info->rate, info->channels, samples, 2);
	}
}

OggAudioFile::~OggAudioFile()
{
	ov_clear(&file);
}

std::string	OggAudioFile::getExtension()
{
	return ".ogg";
}

bool		OggAudioFile::read(void* buffer, int numFrames)
{
	int frames;
	int bytes = numFrames * info->channels * 2;
	frames = ov_read(&file, (char *) buffer, bytes, 0, 2, 1, &stream);
	if (frames < 0) {
		if (frames == OV_HOLE)
			// OV_HOLE is non-fatal
			return true;
		else
			return false;
	}
	return true;
}

size_t	OAFRead(void* ptr, size_t size, size_t nmemb, void* datasource)
{
	std::istream *in = (std::istream*) datasource;
	std::streampos pos1 = in->tellg();
	in->read((char*)ptr, size * nmemb);
	std::streampos pos2 = in->tellg();
	size_t bytesRead = pos2 - pos1;
	if(in->eof() && bytesRead == 0)
		return 0;
	return bytesRead;
}

int		OAFSeek(void* datasource, ogg_int64_t offset, int whence)
{
	std::istream *in = (std::istream*) datasource;
	switch (whence) {
		case SEEK_SET:
			in->seekg(offset, std::ios::beg);
			break;
		case SEEK_CUR:
			in->seekg(offset, std::ios::cur);
			break;
		case SEEK_END:
			in->seekg(offset, std::ios::end);
			break;
	}
	return 0;
}

int		OAFClose(void* datasource)
{
	// technically we should close here, but this is handled outside

	std::ifstream *in = (std::ifstream*) datasource;
	in->close();
	return 0;
}

long		OAFTell(void* datasource)
{
	std::istream *in = (std::istream*) datasource;
	return in->tellg();
}
// ex: shiftwidth=4 tabstop=4
