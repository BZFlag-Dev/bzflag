/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_CALLBACK_LIST_H
#define BZF_CALLBACK_LIST_H

// system headers
#include <utility>
#include <list>
#include <map>

// local implementation headers
#include "common.h"

template <class F>
class CallbackList {
public:
  typedef bool (*Callback)(F, void* userData, void* iterateUserData);

  CallbackList();
  ~CallbackList();

  // add/remove callback.  adding an existing callback has no effect.
  void				add(F func, void* userData);
  void				remove(F func, void* userData);

  // iterate over callbacks.  this is done by invoking the given
  // callback function for each stored callback.  it is safe to
  // add and remove callbacks during iteration.  stops iterating
  // if the callback returns false.
  void				iterate(Callback, void* userData) const;

private:
  void				doIterate(Callback, void* userData);

private:
  typedef std::pair<F, void*> Item;
  typedef std::list<Item> ItemList;
  typedef std::map<Item, typename ItemList::iterator> ItemMap;

  ItemList			items;
  ItemMap			itemMap;
};

//
// CallbackList
//

template <class F>
CallbackList<F>::CallbackList()
{
  // do nothing
}

template <class F>
CallbackList<F>::~CallbackList()
{
  // do nothing
}

template <class F>
void			CallbackList<F>::add(F callback, void* userData)
{
  Item item = std::make_pair(callback, userData);
  if (itemMap.find(item) == itemMap.end()) {
    typename ItemList::iterator index = items.insert(items.end(), item);
    itemMap.insert(std::make_pair(item, index));
  }
}

template <class F>
void			CallbackList<F>::remove(F callback, void* userData)
{
  Item item = std::make_pair(callback, userData);
  typename ItemMap::iterator index = itemMap.find(item);
  if (index != itemMap.end()) {
    items.erase(index->second);
    itemMap.erase(index);
  }
}

template <class F>
void			CallbackList<F>::iterate(Callback callback,
						 void* userData) const
{
  const_cast<CallbackList<F>*>(this)->doIterate(callback, userData);
}

template <class F>
void			CallbackList<F>::doIterate(Callback callback,
						   void* userData)
{
  // insert a dummy item into the list.  this is our safe harbor
  // in case the list is modified while we're iterating over it.
  // the dummy item will remain no matter what other changes
  // occur to the list.  as we invoke each callback we move the
  // dummy item forward.
  Item dummyItem = std::make_pair((F)NULL, (void*)NULL);
  typename ItemList::iterator dummyIndex = items.insert(items.begin(), dummyItem);

  // now invoke each callback
  typename ItemList::iterator index = dummyIndex;
  for (; ++index != items.end(); index = dummyIndex) {
    // move dummy past the item we're about to invoke
    items.splice(dummyIndex, items, index);

    // invoke callback.  skip dummy items (any item with a NULL function).
    // stop if a callback returns false.
    if (index->first != NULL)
      if (!callback(index->first, index->second, userData))
	break;
  }

  // now remove the dummy item
  items.erase(dummyIndex);
}

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

