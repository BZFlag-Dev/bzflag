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

/*
 * compresses data using burrows/wheeler transform followed by a move
 * to front transform, followed by two huffman encoding passes.
 */

#include "Compressor.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

Compressor::Compressor(int blockSize) : chunks(NULL),
				lastChunk(NULL),
				inputCount(0),
				inputCapacity(blockSize)
{
    input = new byte_t[inputCapacity];
}

Compressor::~Compressor()
{
    delete[] input;

    Chunk* scan = chunks;
    while (scan) {
	Chunk* next = scan->next;
	delete[] scan->data;
	delete scan;
	scan = next;
    }
}

void			Compressor::write(const void* _data, int numBytes)
{
    const byte_t* data = (const byte_t*)_data;

    // append data to input buffer until overflow then compress and repeat
    while (inputCount + numBytes >= inputCapacity) {
	const int n = inputCapacity - inputCount;
	memcpy(input + inputCount, data, n);
	addChunk(compress(input, inputCapacity));
	inputCount = 0;
	data += n;
	numBytes -= n;
    }

    // append remaining data
    if (numBytes > 0) {
	memcpy(input + inputCount, data, numBytes);
	inputCount += numBytes;
    }
}

unsigned char*		Compressor::getCompressed(int* numBytes)
{
    // anything left to compress?
    if (inputCount > 0) {
	addChunk(compress(input, inputCount));
	inputCount = 0;
    }

    // compute total size
    int size = 0;
    const Chunk* scan = chunks;
    while (scan) {
	size += scan->size;
	scan = scan->next;
    }

    // allocate space
    unsigned char* buffer = new unsigned char[size];

    // copy
    size = 0;
    scan = chunks;
    while (scan) {
	memcpy(buffer + size, scan->data, scan->size);
	size += scan->size;
	scan = scan->next;
    }

    // return data
    *numBytes = size;
    return buffer;
}

void			Compressor::addChunk(Chunk* chunk)
{
    if (lastChunk) {
	lastChunk->next = chunk;
	lastChunk = chunk;
    }
    else {
	chunks = chunk;
	lastChunk = chunk;
    }
    chunk->next = NULL;
}

Compressor::Chunk*	Compressor::compress(byte_t* data, int numBytes) const
{
    // allocate space
    byte_t* tmp1 = new byte_t[numBytes + 4];

    // burrows/wheeler.  save index and input block size.
    uint32_t index;
    transformBurrowsWheeler(tmp1 + 4, &index, data, numBytes);
    tmp1[0] = (byte_t)((index      ) & 0xfflu);
    tmp1[1] = (byte_t)((index >>  8) & 0xfflu);
    tmp1[2] = (byte_t)((index >> 16) & 0xfflu);
    tmp1[3] = (byte_t)((index >> 24) & 0xfflu);

    // move to front
    transformMoveToFront(tmp1, tmp1, numBytes + 4);
    numBytes += 4;

    // huffman (1st pass).  remember number of bits in last bytes.
    uint32_t numBits;
    byte_t* tmp2 = transformHuffman(&numBits, tmp1, numBytes, 1);
    tmp2[0] = (byte_t)(numBits & 7lu);
    numBytes = (numBits + 7) / 8 + 1;

    // huffman (2nd pass).  remember total number of output bits.
    byte_t* tmp3 = transformHuffman(&numBits, tmp2, numBytes, 4);
    tmp3[0] = (byte_t)((numBits      ) & 0xfflu);
    tmp3[1] = (byte_t)((numBits >>  8) & 0xfflu);
    tmp3[2] = (byte_t)((numBits >> 16) & 0xfflu);
    tmp3[3] = (byte_t)((numBits >> 24) & 0xfflu);
    numBytes = (numBits + 7) / 8 + 4;

    // clean up
    delete[] tmp2;
    delete[] tmp1;

    // now stuff data in a chunk and return it
    Chunk* chunk = new Chunk;
    chunk->next  = NULL;
    chunk->data  = tmp3;
    chunk->size  = numBytes;
    return chunk;
}

//
// BurrowsWheeler stuff
//

byte_t*			Compressor::bwString = NULL;
uint32_t		Compressor::bwStringLength = 0;

int			Compressor::bwCompare(const void* a, const void* b)
{
  const uint32_t index1 = *((uint32_t*)a);
  const uint32_t index2 = *((uint32_t*)b);
  uint32_t n = bwStringLength - ((index1 > index2) ? index1 : index2);
  const byte_t* string1 = bwString + index1;
  const byte_t* string2 = bwString + index2;
  int diff = memcmp(string1, string2, n);
  if (diff == 0)
    return index2 - index1;
  return diff;
}

int			Compressor::compare(
				const byte_t* string,
				uint32_t length,
				uint32_t prefix,
				uint32_t index1,
				uint32_t index2,
				uint32_t* equalLength)
{
  const uint32_t n = length - ((index1 > index2) ? index1 : index2) - prefix;
  const byte_t* string1 = string + index1 + prefix;
  const byte_t* string2 = string + index2 + prefix;
  for (uint32_t i = 0; i < n; ++i) {
    if (string1[i] != string2[i]) {
      *equalLength = prefix + i;
      return string1[i] - string2[i];
    }
  }
  *equalLength = prefix + n;
  return index2 - index1;
}

void			Compressor::quicksort(
				uint32_t* map,
				const byte_t* string,
				uint32_t length,
				uint32_t prefix,
				uint32_t left,
				uint32_t right)
{
  // handle small sorts using bubble sort
  if (right - left < 20) {
    uint32_t n;
    for (uint32_t i = left; i < right; ++i)
      for (uint32_t j = i + 1; j <= right; ++j)
	if (compare(string, length, prefix, map[i], map[j], &n) > 0) {
	  const uint32_t tmp = map[j];
	  map[j]             = map[i];
	  map[i]             = tmp;
	}
    return;
  }

  // find pivot
  uint32_t lIndex    = left;
  uint32_t rIndex    = right;
  uint32_t pivot     = lIndex;
  uint32_t newPrefix = length;
  while (lIndex < rIndex) {
    uint32_t n;

    // scan right to left
    while (lIndex < rIndex) {
      if (compare(string, length, prefix, map[lIndex], map[rIndex], &n) > 0) {
	if (n < newPrefix) newPrefix = n;
	const uint32_t tmp = map[rIndex];
	map[rIndex]        = map[lIndex];
	map[lIndex]        = tmp;
	pivot              = rIndex;
	++lIndex;
	break;
      }
      --rIndex;
      if (n < newPrefix) newPrefix = n;
    }

    // scan left to right
    while (lIndex < rIndex) {
      if (compare(string, length, prefix, map[lIndex], map[rIndex], &n) > 0) {
	if (n < newPrefix) newPrefix = n;
	const uint32_t tmp = map[rIndex];
	map[rIndex]        = map[lIndex];
	map[lIndex]        = tmp;
	pivot              = lIndex;
	--rIndex;
	break;
      }
      ++lIndex;
      if (n < newPrefix) newPrefix = n;
    }
  }

  // recurse
  if (left < pivot) {
    quicksort(map, string, length, newPrefix, left, pivot - 1);
  }
  if (pivot < right) {
    quicksort(map, string, length, newPrefix, pivot + 1, right);
  }
}

void			Compressor::transformBurrowsWheeler(
				byte_t* output, uint32_t* index,
				const byte_t* input, uint32_t length) const
{
  uint32_t i;

  // FIXME -- the sorting is really slow

  // count number of instances of each byte pair.  pretend the
  // input ends with two bytes that are different from all other
  // bytes.
  uint32_t radixCount[257 * 257];
  for (i = 0; i < 257 * 257; i++)
    radixCount[i] = 0;
  int v = (int)input[0];
  for (i = 0; i < length - 1; i++) {
    int v2 = (int)input[i + 1];
    radixCount[(v << 8) + v + v2 + 258]++;
    v = v2;
  }
  radixCount[(v << 8) + v + 257]++;
  radixCount[0]++;

  // radix sort on first two bytes
  uint32_t radixIndex[257 * 257], sum = 0;
  for (i = 0; i < 257 * 257; i++) {
    radixIndex[i] = sum;
    sum += radixCount[i];
  }
  uint32_t* map = new uint32_t[length + 1];
  v = (int)input[0];
  for (i = 0; i < length - 1; i++) {
    int v2 = (int)input[i + 1];
    int index = (v << 8) + v + v2 + 258;
    v = v2;
    map[radixIndex[index]++] = i;
  }
  map[radixIndex[(v << 8) + v + 257]] = length - 1;
  map[0] = length;

  // compute offsets to each set with the same first two bytes
  for (sum = 0, i = 0; i < 257 * 257; i++) {
    radixIndex[i] = sum;
    sum += radixCount[i];
  }

  // quicksort each set
  for (i = 0; i < 257 * 257; i++)
    if (radixCount[i] > 1)
      quicksort(map, input, length, 2, radixIndex[i],
				radixIndex[i] + radixCount[i] - 1);

  // find the index.  this is the map entry that maps to zero.
  for (i = 0; i <= length; i++)
    if (map[i] == 0) {
      *index = i;
      break;
    }
  assert(i != length);

  // fill in output buffer
  for (i = 0; i < *index; i++)
    output[i] = input[map[i] - 1];
  for (++i; i <= length; i++)
    output[i - 1] = input[map[i] - 1];

  // clean up
  delete[] map;
}

//
// MoveToFront stuff
//

void			Compressor::transformMoveToFront(byte_t* output,
				const byte_t* input, uint32_t length) const
{
    // initialize alphabet table
    uint32_t i, table[256][2];
    for (i = 0; i < 256; ++i) {
	table[i][0] = i;
	table[i][1] = i + 1;
    }

    // do move to front
    uint32_t first = 0;
    for (i = 0; i < length; ++i) {
	const uint32_t value = input[i];
	if (table[first][0] == value) {
	    // already at front
	    output[i] = 0;
	}
	else {
	    // find the item with the value
	    uint32_t prev, scan = first, index = 0;
	    do {
		prev = scan;
		scan = table[scan][1];
		assert(scan != 256);
		++index;
	    } while (table[scan][0] != value);

	    // move to front
	    table[prev][1] = table[scan][1];
	    table[scan][1] = first;
	    first = scan;
	    output[i] = (byte_t)index;
	}
    }
}

//
// Huffman stuff
//

const byte_t		Compressor::Code::mask[] = {
				0x80, 0x40, 0x20, 0x10,
				0x08, 0x04, 0x02, 0x01
			};

Compressor::Code::Code() : lengthInBits(0)
{
  for (uint32_t i = 0; i < sizeof(code) / sizeof(code[0]); ++i)
    code[i] = 0;
}

Compressor::Code::~Code()
{
  // do nothing
}

void			Compressor::Code::addBit(uint32_t bit)
{
  uint32_t index    = (lengthInBits >> 3);
  uint32_t bitIndex = (lengthInBits & 0x7);
  if (bit)
    code[index] |=  mask[bitIndex];
  else
    code[index] &= ~mask[bitIndex];
  lengthInBits++;
}

void			Compressor::Code::removeBit()
{
  --lengthInBits;
}

void			Compressor::Code::appendCodeToBuffer(
				byte_t* buffer, uint32_t bitsFilled)
{
  for (uint32_t i = 0; i < lengthInBits; i++) {
    if (code[i >> 3] & mask[i & 7])
      buffer[bitsFilled >> 3] |= mask[bitsFilled & 7];
    bitsFilled++;
  }
}

Compressor::Node::Node(byte_t _value, uint32_t _count, Node* zero, Node* one) :
				value(_value),
				count(_count)
{
    next[0] = zero;
    next[1] = one;
}

Compressor::Node::~Node()
{
    delete next[0];
    delete next[1];
}

void			Compressor::Node::makeDictionary(
				Code* codes, Code* code)
{
  assert((next[0] == NULL) == (next[1] == NULL));

  if (next[0] == NULL) {
    /* bottom of tree.  emit code */
    codes[value] = *code;
  }
  else {
    /* append to code and recurse */
    assert(value == 0);
    code->addBit(0);
    next[0]->makeDictionary(codes, code);
    code->removeBit();
    code->addBit(1);
    next[1]->makeDictionary(codes, code);
    code->removeBit();
  }
}

void			Compressor::Node::listNodes(Node** nodes)
{
  assert((next[0] == NULL) == (next[1] == NULL));

  if (next[0] == NULL) {
    /* bottom of tree.  emit node */
    nodes[value] = this;
  }
  else {
    /* recurse */
    assert(value == 0);
    next[0]->listNodes(nodes);
    next[1]->listNodes(nodes);
  }
}

byte_t			Compressor::Node::findCode(const Node* scan,
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

byte_t*			Compressor::transformHuffman(
				uint32_t* outputLengthInBits,
				const byte_t* input, uint32_t length,
				uint32_t extraLength) const
{
  static const uint32_t MaxBitLength = 256;

  uint32_t i, j, num;

  /* compute frequencies */
  uint32_t count[257];
  for (i = 0; i < 256; i++)
    count[i] = 0;
  for (i = 0; i < length; i++)
    count[input[i]]++;

  /* initialize nodes */
  Node* nodes[256];
  for (i = 0; i < 256; i++)
    nodes[i] = new Node((byte_t)i, count[i], NULL, NULL);

  /* make dictionary tree */
  /* FIXME -- priority queue would be more efficient */
  num = 256;
  while (num > 1) {
    /* find the lowest probability node */
    Node* node, *min1, *min2;
    uint32_t minIndex1 = 0, minIndex2;
    uint32_t minCount1 = nodes[minIndex1]->count, minCount2;
    for (i = 1; i < num; i++) {
      if (nodes[i]->count < minCount1 || (nodes[i]->count == minCount1 &&
	  nodes[i]->value < nodes[minIndex1]->value)) {
	minIndex1 = i;
	minCount1 = nodes[minIndex1]->count;
      }
    }
    min1 = nodes[minIndex1];
    nodes[minIndex1] = nodes[num - 1];
    num--;

    /* find the next lowest probability node */
    minIndex2 = 0;
    minCount2 = nodes[minIndex2]->count;
    for (i = 1; i < num; i++) {
      if (nodes[i]->count < minCount2 || (nodes[i]->count == minCount2 &&
	  nodes[i]->value < nodes[minIndex2]->value)) {
	minIndex2 = i;
	minCount2 = nodes[minIndex2]->count;
      }
    }
    min2 = nodes[minIndex2];
    nodes[minIndex2] = nodes[num - 1];
    num--;

    assert(min1->count <= min2->count);
    assert(min1 != NULL);
    assert(min2 != NULL);

    /* merge */
    node = new Node(0, min1->count + min2->count, min1, min2);

    /* insert new node */
    nodes[num] = node;
    num++;
  }
  Node* tree1 = nodes[0];

  /* make dictionary */
  Code code, codes[256];
  tree1->makeDictionary(codes, &code);

  /* re-write dictionary so that shorter codes lexographically
   * preceed longer ones. first count number of codes using each
   * bit length */
  count[0] = 0;
  for (i = 1; i <= MaxBitLength; i++)
    count[i] = 0;
  for (i = 0; i < 256; i++)
    count[codes[i].lengthInBits]++;

  /* compute total number of nodes preceeding first node of given length */
  uint32_t index[MaxBitLength + 1];
  index[0] = 0;
  for (i = 1; i <= MaxBitLength; i++)
    index[i] = count[i] += count[i - 1];

  /* gets nodes into list */
  Node* nodeList[256];
  tree1->listNodes(nodeList);

  /* insert nodes into list shortest first, equal length codes
   * in lexographic order */
  for (i = 0; i < 256; i++) {
    uint32_t length = codes[i].lengthInBits - 1;
    assert(count[length] < 256);
    assert(count[length] >= index[length]);
    assert(count[length] < index[length+1]);
    nodes[count[length]++] = new Node(nodeList[i]->value, 0, NULL, NULL);
  }

  /* build tree by repeatedly merging last two nodes for each code length,
   * then merging these new nodes with those of the next smaller code length. */
  num = 256;
  for (j = MaxBitLength; j-- > 0; ) {
    const uint32_t b = index[j];
    const uint32_t n = num - index[j];
    assert((n & 1) == 0);
    for (i = 0; i < n; i += 2) {
      nodes[b + (i >> 1)] = new Node(0, 0, nodes[b + i], nodes[b + i + 1]);
    }
    num -= (n >> 1);
  }
  assert(num == 1);
  Node* tree2 = nodes[0];

  /* make the dictionary (again) */
  code = Code();
  tree2->makeDictionary(codes, &code);

  /* prepare output buffer */
  uint32_t tmpLength = 0;
  uint32_t tmpCapacity = 8 * length + 8 + 8 * 256;
  byte_t* tmpOutput = new byte_t[tmpCapacity / 8 + extraLength] + extraLength;
  memset(tmpOutput, 0, tmpCapacity / 8);

  /* make output dictionary.  store code lengths in lexographic order. */
  tmpOutput[0] = 255;
  for (i = 0; i < 256; i++)
    tmpOutput[i + 1] = codes[i].lengthInBits;
  tmpLength = 8 + 8 * 256;

  /* encode */
  for (i = 0; i < length; i++) {
    const uint32_t bits = codes[input[i]].lengthInBits;
    if (tmpLength + bits > tmpCapacity) {
      const uint32_t newCapacity = 2 * tmpCapacity;
      byte_t* newOutput = new byte_t[newCapacity / 8 + extraLength] + extraLength;
      memcpy(newOutput, tmpOutput, tmpCapacity / 8);
      memset(newOutput + tmpCapacity / 8, 0, tmpCapacity / 8);
      delete[] (tmpOutput - extraLength);
      tmpCapacity = newCapacity;
      tmpOutput   = newOutput;
    }

    codes[input[i]].appendCodeToBuffer(tmpOutput, tmpLength);
    tmpLength += bits;
  }

  /* clean up */
  *outputLengthInBits = tmpLength;
  delete tree1;
  delete tree2;
  return tmpOutput - extraLength;
}
