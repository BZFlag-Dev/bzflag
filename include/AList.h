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

/* AList & BZF_DEFINE_ALIST
 *
 * A list (implemented as an array) class instatiator macro.  An
 * abstract base class does most of the real work.  The derived
 * classes have only inline member functions.  This lets us re-
 * instantiate the class in multiple places without getting linker
 * errors.
 *
 * A template would make sense here, but when this was written
 * most template support was pretty bad.
 *
 * Description of selected members:
 *   swap(int, int)			Exchanges items at pos1 and pos2
 *   rotate(int a, int b)		Shifts items a..b-1 one index towards
 *					b, moves item at b to index a.
 */

#ifndef BZF_ALIST_H
#define	BZF_ALIST_H

#include "common.h"

class AListNode {
  public:
			AListNode() {}
    virtual		~AListNode() {}
    virtual void*	getItem() = 0;
};

class AList {
  friend class AListIterator;
  friend class AListCIterator;
  public:
    int			getLength() const;
    void		remove(int position);
    void		removeAll();
    void		swap(int pos1, int pos2);
    void		rotate(int from, int to);
    void		setShrinkable(boolean = True);

  protected:
			AList();
			AList(const AList&);
    virtual		~AList();
    AList&		operator=(const AList&);

    void		voidInsert(const void*, int);
    void*		voidGet(int pos);
    const void*		voidGet(int pos) const;
    AListNode**		copy() const;
    virtual AListNode*	makeNode(const void*) const = 0;

  private:
    boolean		shrink;
    int			length;			// number of used elements
    int			size;			// number of total elements
    AListNode**		list;
    static const int	defaultSize;
};

class AListBIterator {
  public:
    void		reset() { current = 0; }
    void		inc() { current++; }
    int			getIndex() const { return current; }

  protected:
			AListBIterator();
			~AListBIterator();

  private:
    // disallow copy and assignment
			AListBIterator(const AListBIterator&);
    AListBIterator&	operator=(const AListBIterator&);

  private:
    int			current;
};

class AListCIterator : public AListBIterator {
  public:
    boolean		isDone() const;
    void		next();
    const AList&	getList() const;

  protected:
			~AListCIterator();
			AListCIterator(const AList&);
    const void*		voidGetItem() const;

  private:
    // disallow copy and assignment
			AListCIterator(const AListCIterator&);
    AListCIterator&	operator=(const AListCIterator&);

  private:
    const AList&	list;
};

class AListIterator : public AListBIterator {
  public:
    boolean		isDone() const;
    void		next();
    AList&		getList() const;

  protected:
			~AListIterator();
			AListIterator(AList&);
    void*		voidGetItem() const;

  private:
    // disallow copy and assignment
			AListIterator(const AListIterator&);
    AListIterator&	operator=(const AListIterator&);

  private:
    AList&		list;
};

#define	BZF_DEFINE_ALIST(name, type)					\
class name ## Iterator;							\
class name ## CIterator;						\
    class name ## AListNode : public AListNode {				\
      public:								\
    typedef type	Type;						\
			name ## AListNode(const Type& _item) : item(_item) {}\
			~name ## AListNode() {}				\
	void*		getItem() { return &item; }			\
      public:								\
	Type		item;						\
    };									\
class name : public AList {						\
  public:								\
    typedef type	Type;						\
			name() : AList() {/*CONSTRUCT(#name)*/}		\
			name(const name& _list) :			\
				AList(_list) {/*CONSTRUCT(#name)*/}	\
			~name() {/*DESTRUCT(#name)*/}			\
    name&		operator=(const name& _list)			\
				{ AList::operator=(_list);		\
				  return *this; }			\
    Type&		operator[](int pos)				\
				{ return *(Type*)voidGet(pos); }	\
    const Type&		operator[](int pos) const			\
				{ return *(const Type*)voidGet(pos); }	\
    void		prepend(const Type& item)			\
				{ voidInsert(&item, 0); }		\
    void		insert(const Type& item, int pos)		\
				{ voidInsert(&item, pos); }		\
    void		append(const Type& item)			\
				{ voidInsert(&item, getLength()); }	\
    name ## Iterator*	newIterator();					\
    name ## CIterator*	newCIterator() const;				\
									\
  protected:								\
									\
  protected:								\
    AListNode*		makeNode(const void* _item) const		\
				{ return new name ## AListNode(		\
						*(const Type*)_item); }	\
};									\
									\
class name ## CIterator : public AListCIterator {			\
  public:								\
    typedef type	Type;						\
			name ## CIterator(const name& _list) :		\
				AListCIterator(_list) {}		\
			~name ## CIterator() {}				\
    const Type&		getItem() const					\
				{ return *(const Type*)			\
						voidGetItem(); }	\
};									\
									\
class name ## Iterator : public AListIterator {				\
  public:								\
    typedef type	Type;						\
			name ## Iterator(name& _list) :			\
				AListIterator(_list) {}			\
			~name ## Iterator() {}				\
    Type&		getItem() const					\
				{ return *(Type*)voidGetItem(); }	\
};									\
									\
inline name ## Iterator*	name::newIterator()			\
{									\
  return new name ## Iterator(*this);					\
}									\
									\
inline name ## CIterator*	name::newCIterator() const		\
{									\
  return new name ## CIterator(*this);					\
}									\
									\
class name ## CIteratorPtr {						\
  public:								\
			name ## CIteratorPtr(name ## CIterator* _iterator) :\
				iterator(_iterator) {}			\
			~name ## CIteratorPtr() { delete iterator; }	\
    name ## CIterator&	operator*() { return *iterator; }		\
    name ## CIterator*	operator->() { return iterator; }		\
									\
  private:								\
    name ## CIterator*	iterator;					\
};									\
									\
class name ## IteratorPtr {						\
  public:								\
			name ## IteratorPtr(name ## Iterator* _iterator) :\
				iterator(_iterator) {}			\
			~name ## IteratorPtr() { delete iterator; }	\
    name ## Iterator&	operator*() { return *iterator; }		\
    name ## Iterator*	operator->() { return iterator; }		\
									\
  private:								\
    name ## Iterator*	iterator;					\
}

inline int		AList::getLength() const
{
  return length;
}

inline void*		AList::voidGet(int pos)
{
  assert(pos >= 0 && pos < length);
  return list[pos]->getItem();
}

inline const void*	AList::voidGet(int pos) const
{
  assert(pos >= 0 && pos < length);
  return list[pos]->getItem();
}

#endif // BZF_ALIST_H
