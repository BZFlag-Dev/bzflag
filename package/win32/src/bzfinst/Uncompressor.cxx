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

/*
 * uncompresses data compressed by Compressor in makedb
 */

#include "Uncompressor.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

Uncompressor::Uncompressor(const void* buffer, int count) :
				d_data((const byte_t*)buffer),
				d_ptr(0),
				d_len(count),
				d_raw(NULL),
				d_rawPtr(0),
				d_rawLen(0)
{
    // do nothing
}

Uncompressor::~Uncompressor()
{
    delete[] d_raw;
}

byte_t			Uncompressor::getByte()
{
    byte_t b;
    if (getData(&b, 1) != 1)
	return 0;
    return b;
}

uint32_t		Uncompressor::getUInt32()
{
    byte_t b[4];
    if (getData(b, 4) != 4)
	return (uint32_t)-1;
    return ((uint32_t)b[3] << 24) + ((uint32_t)b[2] << 16) +
	   ((uint32_t)b[1] << 8)  +  (uint32_t)b[0];
}

int			Uncompressor::getString(char* buffer, int n)
{
    const int len = (int)getUInt32();
    if (len == -1) return -1;
    if (len >= n) return -1;
    n = getData(buffer, len);
    buffer[n] = 0;
    if (d_rawLen == 0)
      return -1;
    return n;
}

int			Uncompressor::getData(void* buffer, int count)
{
    int copied = 0;
    while (count > 0) {
	if (d_rawPtr == d_rawLen) {
	    decompress();
	    if (d_rawPtr == d_rawLen)
		return copied;
	}

	int n = d_rawLen - d_rawPtr;
	if (n > count) n = count;
	memcpy(buffer, d_raw + d_rawPtr, n);
	buffer    = ((byte_t*)buffer) + n;
	count    -= n;
	d_rawPtr += n;
	copied   += n;
    }
    return copied;
}

int			Uncompressor::eof() const
{
    return (d_ptr == d_len && d_rawPtr == d_rawLen);
}

void			Uncompressor::decompress()
{
    delete[] d_raw;
    d_raw    = NULL;
    d_rawPtr = 0;
    d_rawLen = 0;

    // done if not enough header data
    if (d_len - d_ptr < 4) return;

    // huffman (1st pass).  first get number of bits.
    const byte_t* b  = d_data + d_ptr;
    uint32_t numBits =  ((uint32_t)b[3] << 24) + ((uint32_t)b[2] << 16) +
			((uint32_t)b[1] << 8)  +  (uint32_t)b[0];
    if (d_len - d_ptr < (int)(numBits + 7) / 8) return;
    uint32_t numBytes;
    byte_t* tmp1 = transformReverseHuffman(&numBytes, b + 4, numBits);
    d_ptr += (numBits + 7) / 8 + 4;

    // huffman (2nd pass).  first get number of bits in last byte.
    numBits = 8 * (numBytes - 1);
    if (tmp1[0] != 0) numBits = numBits - 8 + (uint32_t)tmp1[0];
    byte_t* tmp2 = transformReverseHuffman(&numBytes, tmp1 + 1, numBits);
    delete[] tmp1;

    // move to front
    transformReverseMoveToFront(tmp2, tmp2, numBytes);

    // burrows/wheeler.  first get index.
    tmp1 = new byte_t[numBytes - 4];
    uint32_t index = ((uint32_t)tmp2[3] << 24) + ((uint32_t)tmp2[2] << 16) +
		     ((uint32_t)tmp2[1] << 8)  +  (uint32_t)tmp2[0];
    transformReverseBurrowsWheeler(tmp1, index, tmp2 + 4, numBytes - 4);
    delete[] tmp2;

    // ready uncompressed read buffer
    d_raw    = tmp1;
    d_rawPtr = 0;
    d_rawLen = numBytes - 4;
}

//
// Huffman stuff
//

Uncompressor::Node::Node(byte_t _value, uint32_t _count, Node* zero, Node* one) :
				value(_value),
				count(_count)
{
    next[0] = zero;
    next[1] = one;
}

Uncompressor::Node::~Node()
{
    delete next[0];
    delete next[1];
}

byte_t			Uncompressor::Node::findCode(const Node* scan,
				const byte_t* data, uint32_t* bits)
{
  uint32_t d;
  uint32_t bitCount = 0;

  assert(scan != NULL && data != NULL && bits != NULL);

  /* prepare the pointer into the input data */
  data += (*bits >> 3);
  d = *data;

  /* traverse tree, moving left or right depending on the next bit,
   * until we hit a leaf node.  this is an unrolled loop;  the switch
   * is used to jump into the right test for the first bit's position. */
  switch (*bits & 7) {
loop:
    case 0:
      bitCount++;
      if (d & 0x80)
	scan = scan->next[1];
      else
	scan = scan->next[0];
      if (scan->next[0] == NULL)
	break;

    case 1:
      bitCount++;
      if (d & 0x40)
	scan = scan->next[1];
      else
	scan = scan->next[0];
      if (scan->next[0] == NULL)
	break;

    case 2:
      bitCount++;
      if (d & 0x20)
	scan = scan->next[1];
      else
	scan = scan->next[0];
      if (scan->next[0] == NULL)
	break;

    case 3:
      bitCount++;
      if (d & 0x10)
	scan = scan->next[1];
      else
	scan = scan->next[0];
      if (scan->next[0] == NULL)
	break;

    case 4:
      bitCount++;
      if (d & 0x08)
	scan = scan->next[1];
      else
	scan = scan->next[0];
      if (scan->next[0] == NULL)
	break;

    case 5:
      bitCount++;
      if (d & 0x04)
	scan = scan->next[1];
      else
	scan = scan->next[0];
      if (scan->next[0] == NULL)
	break;

    case 6:
      bitCount++;
      if (d & 0x02)
	scan = scan->next[1];
      else
	scan = scan->next[0];
      if (scan->next[0] == NULL)
	break;

    case 7:
      bitCount++;
      if (d & 0x01)
	scan = scan->next[1];
      else
	scan = scan->next[0];
      if (scan->next[0] == NULL)
	break;

      d = *++data;
      goto loop;
  }

  /* update number of bits read */
  *bits += bitCount;

  /* return the code */
  assert(scan->next[0] == NULL && scan->next[1] == NULL);
  return scan->value;
}

byte_t*			Uncompressor::transformReverseHuffman(
				uint32_t* outputLength,
				const byte_t* input,
				uint32_t inputLengthInBits)
{
  // NOTE -- this code isn't generic enough handle changes to MaxBitLength
  static const int MaxBitLength = 256;

  uint32_t i, j, num;

  // separate dictionary from data
  uint32_t dictionaryLength = (uint32_t)(*input) + 1;
  const byte_t* dictionary = ++input;
  input += dictionaryLength;
  inputLengthInBits -= (dictionaryLength + 1) * 8;

  // reconstruct dictionary;  first count the number of codes of each length
  uint32_t count[MaxBitLength + 1];
  for (i = 0; i <= MaxBitLength; i++)
    count[i] = 0;
  for (i = 0; i < dictionaryLength; i++)
    count[dictionary[i]]++;
  assert(count[0] == 0);

  // compute total number of codes preceding first code of given length
  uint32_t index[MaxBitLength + 1];
  index[0] = 0;
  for (i = 1; i <= MaxBitLength; i++)
    index[i] = count[i] += count[i - 1];

  // gets nodes into list in dictionary order for a given bit length,
  // shorter codes first.
  Node* nodes[MaxBitLength];
  for (i = 0; i < dictionaryLength; i++) {
    assert(dictionary[i] - 1 < MaxBitLength);
    assert(count[dictionary[i] - 1] < MaxBitLength);
    nodes[count[dictionary[i] - 1]++] = new Node((byte_t)i, 0, NULL, NULL);
  }

  /* build tree by repeatedly merging last two nodes for each code length,
   * then merging these new nodes with those of the next smaller code length. */
  num = dictionaryLength;
  for (j = MaxBitLength; j-- > 0; ) {
    const uint32_t b = index[j];
    const uint32_t n = num - index[j];
    assert(num <= dictionaryLength);
    assert(b <= MaxBitLength);
    assert(n <= MaxBitLength);
    assert((n & 1) == 0);
    for (i = 0; i < n; i += 2) {
      nodes[b + (i >> 1)] = new Node(0, 0, nodes[b + i], nodes[b + i + 1]);
    }
    num -= (n >> 1);
  }
  assert(num == 1);

  /* prepare output buffer */
  uint32_t tmpLength   = 0;
  uint32_t tmpCapacity = 2 * inputLengthInBits;
  byte_t* tmpOutput    = new byte_t[tmpCapacity];

  /* decode */
  num = 0;
  while (num < inputLengthInBits) {
    if (tmpLength >= tmpCapacity) {
      const uint32_t newCapacity = 2 * tmpCapacity;
      byte_t* newOutput = new byte_t[newCapacity];
      memcpy(newOutput, tmpOutput, tmpCapacity);
      delete[] tmpOutput;
      tmpCapacity = newCapacity;
      tmpOutput   = newOutput;
    }

    /* get the next code */
    tmpOutput[tmpLength++] = Node::findCode(nodes[0], input, &num);
  }

  /* clean up */
  *outputLength = tmpLength;
  delete nodes[0];
  return tmpOutput;
}

//
// move to front stuff
//

void			Uncompressor::transformReverseMoveToFront(
				byte_t* output,
				const byte_t* input, uint32_t length)
{
  uint32_t i;

  // initialize the alphabet list
  byte_t list[256];
  for (i = 0; i < 256; i++)
    list[i] = (byte_t)i;

  // decode
  // FIXME -- this is inefficient
  for (i = 0; i < length; i++) {
    const uint32_t j  = input[i];
    const byte_t data = list[j];
    memmove(list + 1, list, j);
    list[0]   = data;
    output[i] = data;
  }
}

// Burrows/Wheeler stuff

void			Uncompressor::transformReverseBurrowsWheeler(
				byte_t* output, uint32_t index,
				const byte_t* input, uint32_t length)
{
  uint32_t i;

  // zero initial total for each character
  uint32_t total[256];
  for (i = 0; i < 256; i++)
    total[i] = 0;

  // after this loop, count[i] is the number of instances of input[i]
  // in input[0..i-1] and total[i] is the number of occurrences of i
  // anywhere in input.
  uint32_t* count = new uint32_t[length];
  for (i = 0; i < length; i++) {
    count[i] = total[input[i]];
    total[input[i]]++;
  }

  // after this loop, total[i] is the number of instances in input
  // of characters preceding i in the alphabet.
  uint32_t sum = 1;
  for (i = 0; i < 256; i++) {
    sum += total[i];
    total[i] = sum - total[i];
  }

  // now get the original string
  uint32_t origIndex = index;
  index = 0;
  for (i = length; i-- > 0; ) {
    output[i] = input[index];
    index = count[index] + total[input[index]];
    if (index > origIndex)
      index--;
  }

  // clean up
  delete[] count;
}
