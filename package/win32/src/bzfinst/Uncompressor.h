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

#ifndef BZF_UNCOMPRESSOR_H
#define BZF_UNCOMPRESSOR_H

typedef unsigned char	byte_t;
typedef unsigned int	uint32_t;

class Uncompressor {
  public:
    Uncompressor(const void* buffer, int count);
    ~Uncompressor();

    byte_t		getByte();
    uint32_t		getUInt32();
    int			getString(char* buffer, int maxLen);
    int			getData(void* buffer, int count);
    int			eof() const;

  private:
    void		decompress();

    // Burrows/Wheeler stuff
    void		transformReverseBurrowsWheeler(
				byte_t* output, uint32_t index,
				const byte_t* input, uint32_t length);

    // move to front stuff
    void		transformReverseMoveToFront(
				byte_t* output,
				const byte_t* input, uint32_t length);

    // Huffman stuff
    byte_t*		transformReverseHuffman(
				uint32_t* outputLength,
				const byte_t* input,
				uint32_t inputLengthInBits);
    class Node {
      public:
	Node(byte_t value, uint32_t count, Node* zero, Node* one);
	~Node();
	static byte_t	findCode(const Node* scan,
				const byte_t* data, uint32_t* bits);
      public:
	byte_t		value;
	uint32_t	count;
	Node*		next[2];
    };

  private:
    const byte_t*	d_data;
    int			d_ptr;
    int			d_len;
    byte_t*		d_raw;
    int			d_rawPtr;
    int			d_rawLen;
};

#endif
