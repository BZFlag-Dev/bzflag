/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * creates an install database as a COFF object file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include "install.h"
#include "Compressor.h"


typedef int		(*CmdFunc)(char** argv);
static unsigned char	buffer[4096];
static FILE*		outFile = NULL;
static int		size = 0;
static bool		verbose = false;
static Compressor*	compressor = NULL;

static void		write8(byte_t value, FILE* file)
{
    fwrite(&value, 1, 1, file);
}

static void		write16(uint32_t value, FILE* file)
{
    byte_t buffer[2];
    buffer[0] = (byte_t)((value      ) & 0xff);
    buffer[1] = (byte_t)((value >>  8) & 0xff);
    fwrite(buffer, 2, 1, file);
}

static void		write32(uint32_t value, FILE* file)
{
    byte_t buffer[4];
    buffer[0] = (byte_t)((value      ) & 0xff);
    buffer[1] = (byte_t)((value >>  8) & 0xff);
    buffer[2] = (byte_t)((value >> 16) & 0xff);
    buffer[3] = (byte_t)((value >> 24) & 0xff);
    fwrite(buffer, 4, 1, file);
}

static int		mfwrite(const void* b, size_t size, size_t n)
{
    if (size > 0 && n > 0)
	compressor->write(b, size * n);
    return size * n;
}

void			writeByte(byte_t n)
{
    mfwrite(&n, 1, 1);
}

void			writeDWord(uint32_t n)
{
    byte_t buffer[4];
    buffer[0] = (byte_t)((n      ) & 0xff);
    buffer[1] = (byte_t)((n >>  8) & 0xff);
    buffer[2] = (byte_t)((n >> 16) & 0xff);
    buffer[3] = (byte_t)((n >> 24) & 0xff);
    mfwrite(&buffer, 1, 4);
}

int			noop(char** argv)
{
    return 0;
}

int			addFile(char** argv)
{
    FILE* f = fopen(argv[0], "rb");
    if (!f) {
	fprintf(stderr, "can't open %s\n", argv[0]);
	return 1;
    }

    writeByte(INST_FILE);
    writeDWord(strlen(argv[1]));
    mfwrite(argv[1], 1, strlen(argv[1]));
    if (fseek(f, 0, SEEK_END) < 0) {
	perror("seek failed");
	return 1;
    }
    const long pos = ftell(f);
    if (pos < 0) {
	perror("tell failed");
	return 1;
    }
    writeDWord(pos);
    if (fseek(f, 0, SEEK_SET) < 0) {
	perror("seek failed");
	return 1;
    }
    while (!feof(f)) {
	const int n = fread(buffer, 1, sizeof(buffer), f);
	if (n < 0) {
	    fclose(f);
	    perror("read failed");
	    return 1;
	}
	mfwrite(buffer, 1, n);
    }
    fclose(f);
    size += (int)pos;
    if (verbose)
	fprintf(stdout, "added %s as %s: %d bytes\n",
				argv[0], argv[1], pos);
    return 0;
}

int			addDLL(char** argv)
{
    assert(0 && "not implemented");
    return 0;
}

int			setInstDir(char**)
{
    writeByte(INST_SETINSTDIR);
    return 0;
}

int			setWinDir(char**)
{
    writeByte(INST_SETSYSDIR);
    return 0;
}

int			setSysDir(char**)
{
    writeByte(INST_SETWINDIR);
    return 0;
}

int			pushDir(char** argv)
{
    writeByte(INST_PUSHDIR);
    writeDWord(strlen(argv[0]));
    mfwrite(argv[0], 1, strlen(argv[0]));
    return 0;
}

int			popDir(char**)
{
    writeByte(INST_POPDIR);
    return 0;
}

int			addRegKey(char** argv)
{
    writeByte(INST_REGKEY);
    writeDWord(strlen(argv[0]));
    mfwrite(argv[0], 1, strlen(argv[0]));
    writeDWord(strlen(argv[1]));
    mfwrite(argv[1], 1, strlen(argv[1]));
    return 0;
}

int			addRegString(char** argv)
{
    writeByte(INST_REGSTRING);
    writeDWord(strlen(argv[0]));
    mfwrite(argv[0], 1, strlen(argv[0]));
    writeDWord(strlen(argv[1]));
    mfwrite(argv[1], 1, strlen(argv[1]));
    writeDWord(strlen(argv[2]));
    mfwrite(argv[2], 1, strlen(argv[2]));
    return 0;
}

int			addRegDWord(char** argv)
{
    writeByte(INST_REGDWORD);
    writeDWord(strlen(argv[0]));
    mfwrite(argv[0], 1, strlen(argv[0]));
    writeDWord(strlen(argv[1]));
    mfwrite(argv[1], 1, strlen(argv[1]));
    writeDWord(atoi(argv[2]));
    return 0;
}

int			addReadme(char** argv)
{
    writeByte(INST_ADDREADME);
    writeDWord(strlen(argv[0]));
    mfwrite(argv[0], 1, strlen(argv[0]));
    return 0;
}

int			addShortcut(char** argv)
{
    writeByte(INST_ADDSHORTCUT);
    writeDWord(strlen(argv[0]));
    mfwrite(argv[0], 1, strlen(argv[0]));
    writeDWord(strlen(argv[1]));
    mfwrite(argv[1], 1, strlen(argv[1]));
    writeDWord(strlen(argv[2]));
    mfwrite(argv[2], 1, strlen(argv[2]));
    writeDWord(strlen(argv[3]));
    mfwrite(argv[3], 1, strlen(argv[3]));
    return 0;
}

int			addError(char**)
{
    writeByte(INST_NONE);
    return 0;
}

int			parseCommand(char* cmd, char** cmdArgs, int* numArgs)
{
    // scan through cmd for argument strings

    int n = 0;
    while (*cmd && n < *numArgs) {
	// skip whitespace
	while (*cmd && isspace(*cmd))
	    cmd++;
	if (*cmd == 0)
	    break;

	// starts with a quote?
	const bool quoted = (*cmd == '\"');
	if (quoted)
	    cmd++;

	// remember arg
	cmdArgs[n] = cmd;
	n++;

	// scan for whitespace or quote
	if (quoted)
	    while (*cmd && *cmd != '\"')
		cmd++;
	else
	    while (*cmd && !isspace(*cmd))
		cmd++;

	// delimit arg
	if (*cmd != 0)
	    *cmd++ = 0;
    }

    // error if not at end of string for any reason
    if (*cmd)
	return 1;

    // save number of arguments
    *numArgs = n;

    return 0;
}


struct FuncEntry {
    const char*		cmd;
    const char*		help;
    int			args;
    CmdFunc		func;
};
static FuncEntry	map[] = {
			"-file", "<src> <dst>", 2, addFile,
			"-dll", "<src> <dst>", 2, addDLL,
			"-instdir", "", 0, setInstDir,
			"-windir", "", 0, setWinDir,
			"-sysdir", "", 0, setSysDir,
			"-pushdir", "<subdir>", 1, pushDir,
			"-popdir", "", 0, popDir,
			"-regkey", "<parent> <name>", 2, addRegKey,
			"-regstring", "<parent> <name> <value>", 3, addRegString,
			"-regdword", "<parent> <name> <value>", 3, addRegDWord,
			"-readme", "<dst>", 1, addReadme,
			"-shortcut",
				"<dst> <shortcut-name> <args> <working-dir>",
				4, addShortcut,
			"-error", "", 0, addError
			};
/* shortcut args and working-dir can use the following:
 *  %c -- replace with current directory
 *  %i -- replace with base install directory
 *  %s -- replace with system directory
 *  %w -- replace with windows directory
 *  %% -- replace with %
 */

static void usage(const char* arg0)
{
    fprintf(stderr, "usage: %s [-i <input-file>] [-o <output-file>]\n", arg0);
    fprintf(stderr, "\tinput file is a text file, one command per line:\n");
    const int n = sizeof(map) / sizeof(map[0]);
    for (int i = 0; i < n; i++)
	fprintf(stderr, "\t%-20s %s\n", map[i].cmd, map[i].help);
    exit(1);
}

int main(int argc, char** argv)
{
    const char* inFileName = NULL, *outFileName = NULL;
    int blockSize = 1024 * 1024;

    // parse
    for (int i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-i") == 0) {
	    if (inFileName)
		usage(argv[0]);
	    if (i + 1 >= argc) {
		fprintf(stderr, "missing argument for %s\n", argv[i]);
		usage(argv[0]);
	    }
	    inFileName = argv[i + 1];
	    i++;
	}
	else if (strcmp(argv[i], "-o") == 0) {
	    if (outFileName)
		usage(argv[0]);
	    if (i + 1 >= argc) {
		fprintf(stderr, "missing argument for %s\n", argv[i]);
		usage(argv[0]);
	    }
	    outFileName = argv[i + 1];
	    i++;
	}
	else if (strcmp(argv[i], "-v") == 0) {
	    verbose = true;
	}
	else if (strcmp(argv[i], "-b") == 0) {
	    if (i + 1 >= argc) {
		fprintf(stderr, "missing argument for %s\n", argv[i]);
		usage(argv[0]);
	    }
	    const int newBlockSize = atoi(argv[i + 1]);
	    if (newBlockSize < 256 * 256) {
		fprintf(stderr, "block size %d too small\n", argv[i + 1]);
		return 1;
	    }
	    if (newBlockSize >= 256 * 256 * 256) {
		fprintf(stderr, "block size %d too large\n", argv[i + 1]);
		return 1;
	    }
	    blockSize = newBlockSize;
	    i++;
	}
	else {
	    usage(argv[0]);
	}
    }

    // open the files
    FILE* inFile = stdin;
    if (inFileName) {
	inFile = fopen(inFileName, "r");
	if (!inFile) {
	    fprintf(stderr, "can't open input file %s\n", inFileName);
	    return 1;
	}
    }
    setmode(fileno(stdout), _O_BINARY);
    outFile = stdout;
    if (outFileName) {
	outFile = fopen(outFileName, "wb");
	if (!outFile) {
	    fprintf(stderr, "can't open output file %s\n", outFileName);
	    return 1;
	}
    }

    compressor = new Compressor(blockSize);

    // read and execute commands
    char cmd[4096];
    char* cmdArgs[256];
    const int n = sizeof(map) / sizeof(map[0]);
    while (fgets(cmd, sizeof(cmd), inFile)) {
	// parse command line
	int numArgs = sizeof(cmdArgs) / sizeof(cmdArgs[0]);
	if (parseCommand(cmd, cmdArgs, &numArgs) != 0)
	    return 1;

	// execute
	int result = -1;
	for (int i = 0; i < n; i++)
	    if (strcmp(cmdArgs[0], map[i].cmd) == 0) {
		if (map[i].args != numArgs - 1) {
		    fprintf(stderr, "bad arguments for %s\n", cmdArgs[0]);
		    return 1;
		}
		result = (*map[i].func)(cmdArgs + 1);
		break;
	    }
	if (result == -1) {
	    fprintf(stderr, "unknown command\n");
	    return 1;
	}
	if (result != 0)
	    return result;
    }

    if (inFile != stdin)
	fclose(inFile);

    // get the compressed data
    int compressedSize;
    byte_t* compressedData = compressor->getCompressed(&compressedSize);
    delete compressor;

    // dump data.  this is written as a COFF file.  the file contains
    // one C symbol: database.  database is an array of bytes.  the
    // first 4 bytes is the original data size (little endian), the next
    // 4 bytes is the length of the compressed data (little endian), and
    // the rest is the compressed data.
    //
    // note that this isn't very portable.

    // first write the file header
    write16(0x14c, outFile);				// magic
    write16(1, outFile);				// number of sections
    write32(time(NULL), outFile);			// time stamp
    write32(58 + 8 + compressedSize, outFile);		// ptr to symtab
    write32(3, outFile);				// num symtab entries
    write16(0, outFile);				// opthdr size
    write16(0, outFile);				// flags

    // now write the data section
    fwrite(".data   ", 8, 1, outFile);
    write32(0, outFile);				// physical address
    write32(0, outFile);				// virtual address
    write32(compressedSize + 8, outFile);		// section size
    write32(58, outFile);				// ptr to data
    write32(0, outFile);				// ptr to relocation
    write32(0, outFile);				// ptr to line numbers
    write16(0, outFile);				// num relocation
    write16(0, outFile);				// num line numbers
    write16(0x40, outFile);				// init data flag

    // now the original size
    write32(size, outFile);

    // now the compressed size
    write32(compressedSize, outFile);

    // now the compressed data
    fwrite(compressedData, compressedSize, 1, outFile);

    // now the symbol table
    // .data section symbol
    fwrite(".data   ", 8, 1, outFile);
    write32(0, outFile);				// value
    write16(1, outFile);				// section number
    write16(0, outFile);				// type (none)
    write8(3, outFile);					// class (static sym)
    write8(1, outFile);					// num aux
    write32(compressedSize + 8, outFile);		// value
    write32(0, outFile);				// zeros
    write32(0 /* FIXME */, outFile);			// checksum
    write32(0, outFile);				// zeros
    write16(0, outFile);				// zeros
    // database symbol
    write32(0, outFile);				// zeros
    write32(4, outFile);				// offset
    write32(0, outFile);				// value
    write16(1, outFile);				// section number
    write16(0, outFile);				// type (none)
    write8(2, outFile);					// class (extern sym)
    write8(0, outFile);					// num aux

    // finally the string table
    write32(14, outFile);				// length
    fwrite("_database", 10, 1, outFile);

    delete[] compressedData;
    if (outFile != stdout)
	fclose(outFile);

    return 0;
}
// ex: shiftwidth=2 tabstop=8
