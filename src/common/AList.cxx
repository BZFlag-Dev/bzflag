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

/*
 * To make insertion and deletion more efficient time-wise, we double
 * the size of the list whenever it gets full and shrink it only when
 * it's less than a quarter full.
 */

#include <string.h>
#include "AList.h"

//
// AList
//

const int		AList::defaultSize = 2;

AList::AList() : shrink(True), length(0), size(defaultSize),
				list(new AListNode*[defaultSize])
{
  // do nothing
}

AList::AList(const AList& _list) : shrink(True),
				length(_list.length),
				size(_list.size),
				list(_list.copy())
{
  // do nothing
}

AList::~AList()
{
  removeAll();
  delete[] list;
}

AList&			AList::operator=(const AList& _list)
{
  if (this != &_list) {
    removeAll();
    delete[] list;
    length = _list.length;
    size = _list.size;
    list = _list.copy();
  }
  return *this;
}

void			AList::setShrinkable(boolean _shrink)
{
  shrink = _shrink;
}

void			AList::remove(int pos)
{
  rotate(length-1, pos);
  length--;
  delete list[length];
  if (shrink && length < (size >> 2) && size > defaultSize) {
    // shrink by half
    size >>= 1;
    AListNode** newList = new AListNode*[size];
    ::memcpy(newList, list, length * sizeof(AListNode*));
    delete[] list;
    list = newList;
  }
}

void			AList::removeAll()
{
  for (int i = 0; i < length; i++)
    delete list[i];
  delete[] list;

  length = 0;
  size = defaultSize;
  list = new AListNode*[size];
}

void			AList::swap(int pos1, int pos2)
{
#ifdef DEBUG
  // NOTE: should throw an exception instead
  assert(pos1 >= 0 && pos1 < length);
  assert(pos2 >= 0 && pos2 < length);
#endif

  AListNode* tmp = list[pos1];
  list[pos1] = list[pos2];
  list[pos2] = tmp;
}

void			AList::rotate(int pos1, int pos2)
{
#ifdef DEBUG
  // NOTE: should throw an exception instead
  assert(pos1 >= 0 && pos1 < length);
  assert(pos2 >= 0 && pos2 < length);
#endif

  AListNode* tmp = list[pos2];
  if (pos1 < pos2) {
    for (int i = pos2; i > pos1; i--)
      list[i] = list[i-1];
  }
  else {
    for (int i = pos2; i < pos1; i++)
      list[i] = list[i+1];
  }
  list[pos1] = tmp;
}

void			AList::voidInsert(const void* item, int pos)
{
#ifdef DEBUG
  // NOTE: should throw an exception instead
  assert(pos >= 0 && pos <= length);
#endif

  if (length == size) {
    // overflowed -- double the size
    size <<= 1;
    AListNode** newList = new AListNode*[size];
    ::memcpy(newList, list, length * sizeof(AListNode*));
    delete[] list;
    list = newList;
  }

  // add new guy and rotate it into position
  length++;
  list[length-1] = makeNode(item);
  rotate(pos, length-1);
}

AListNode**		AList::copy() const
{
  AListNode** newList = new AListNode*[size];
  for (int i = 0; i < length; i++)
    newList[i] = makeNode(list[i]->getItem());
  return newList;
}

//
// AListBIterator
//

AListBIterator::AListBIterator() : current(0)
{
  // do nothing
}

AListBIterator::~AListBIterator()
{
  // do nothing
}

//
// AListCIterator
//

AListCIterator::AListCIterator(const AList& _list) : list(_list)
{
  // do nothing
}

AListCIterator::~AListCIterator()
{
  // do nothing
}

boolean			AListCIterator::isDone() const
{
  return (getIndex() == list.length);
}

void			AListCIterator::next()
{
  if (!isDone()) inc();
}

const AList&		AListCIterator::getList() const
{
  return list;
}

const void*		AListCIterator::voidGetItem() const
{
  // NOTE: should throw exception if isDone()
  return list.voidGet(getIndex());
}

//
// AListIterator
//

AListIterator::AListIterator(AList& _list) : list(_list)
{
  // do nothing
}

AListIterator::~AListIterator()
{
  // do nothing
}

boolean			AListIterator::isDone() const
{
  return (getIndex() == list.length);
}

void			AListIterator::next()
{
  if (!isDone()) inc();
}

AList&			AListIterator::getList() const
{
  return list;
}

void*			AListIterator::voidGetItem() const
{
  // NOTE: should throw exception if isDone()
  return list.voidGet(getIndex());
}
