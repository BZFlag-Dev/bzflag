/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_COMPRESSOR_H
#define BZF_COMPRESSOR_H

typedef unsigned char	byte_t;
typedef unsigned int	uint32_t;

class Compressor {
  public:
    Compressor(int blockSize = 1024 * 1024);
    ~Compressor();

    // append more data to compress
    void		write(const void* buffer, int numBytes);

    // get compressed data.  caller must free with delete[].
    byte_t*		getCompressed(int* numBytes);

    class Chunk {
      public:
	Chunk*		next;
	byte_t*		data;
	int		size;
    };

  private:
    void		addChunk(Chunk*);
    Chunk*		compress(byte_t* data, int numBytes) const;

    // Burrows/Wheeler stuff
    void		transformBurrowsWheeler(byte_t* output,
				uint32_t* index, const byte_t* input,
				uint32_t inputBytes) const;
    static int		bwCompare(const void*, const void*);
    static int		compare(const byte_t* string,
				uint32_t length, uint32_t prefix,
				uint32_t index1, uint32_t index2,
				uint32_t* equalLength);
    static void		quicksort(uint32_t* map,
				const byte_t* string,
				uint32_t length, uint32_t prefix,
				uint32_t left, uint32_t right);

    // move to front stuff
    void		transformMoveToFront(byte_t* output,
				const byte_t* input, uint32_t inputBytes) const;

    // Huffman stuff
    byte_t*		transformHuffman(uint32_t* numBits,
				const byte_t* input, uint32_t inputBytes,
				uint32_t extraBytes) const;
    class Code {
      public:
	Code();
	~Code();
	void		addBit(uint32_t);
	void		removeBit();
	void		appendCodeToBuffer(byte_t* buffer, uint32_t bitsFilled);
      public:
	uint32_t	lengthInBits;
	byte_t		code[256 / 8];
	static const byte_t mask[];
    };
    class Node {
      public:
	Node(byte_t value, uint32_t count, Node* zero, Node* one);
	~Node();
	void		makeDictionary(Code* codes, Code* code);
	void		listNodes(Node** nodes);
	static byte_t	findCode(const Node* scan,
				const byte_t* data, uint32_t* bits);
      public:
	byte_t		value;
	uint32_t	count;
	Node*		next[2];
    };

  private:
    Chunk*		chunks;
    Chunk*		lastChunk;
    byte_t*		input;
    int			inputCount;
    int			inputCapacity;

    // Burrows/Wheeler stuff
    static byte_t*	bwString;
    static uint32_t	bwStringLength;
};

#endif
