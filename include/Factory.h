/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __FACTORY_H__
#define __FACTORY_H__

#include "common.h"

/* system headers */
#include <iostream>
#include <map>


/**
 * a Factory is an abstract means to generate objects of some
 * conceptually equivalent base type.  object types are registered
 * with the factory using some unique key (usually a string or
 * integer).  it is up to classes inheriting from Factory to also
 * inherit from a Singleton if they want to limit multiple factory
 * instantiations.
 */
template <typename BaseType, typename UniqueTypeID>
class Factory
{
protected:
  typedef BaseType* (*Manufacture)();

  /** manufacturing member function for generating objects of some
   *  given BaseType compatible type
   */
  template <typename ObjectType>
  static BaseType *CreateObject()
  {
    std::cout << "Creating a new object" << std::endl;
    return new ObjectType();
  }

public:
  typedef typename std::map<UniqueTypeID, Manufacture>::iterator iterator;
  typedef typename std::map<UniqueTypeID, Manufacture>::const_iterator const_iterator;

  /**
   * register some object type with the factory providing some unique
   * key to differentiate.  the object will generally be a class that
   * extends from a provided base class type.
   */
  template <typename ObjectType>
  bool Register(UniqueTypeID id) {
    // re-registering simply replaces previous
    objectFactories[id] = &CreateObject<ObjectType>;
    return true;
  }

  /** return truthfully whether the object is already registered
   */
  bool IsRegistered(UniqueTypeID id) const {
    if (objectFactories.size() > 0) {
      const_iterator objectEntry = objectFactories.find(id);
      if (objectEntry != objectFactories.end()) {
	return true;
      }
    }
    return false;
  }

  /**
   * print the stored type identifiers. assumes << operator is
   * defined. usually some string identifier.
   */
  void Print(std::ostream &stream) const {
    if (objectFactories.size() > 0) {
      const_iterator objectEntry = objectFactories.begin();
      while (objectEntry != objectFactories.end()) {
	stream << (*objectEntry).first << std::endl;
	objectEntry++;
      }
    }
  }

  /**
   * have the factory generate a "default" object, the first object
   * type registered.
   */
  BaseType *Create() {
    iterator entry = objectFactories.begin();
    return ((*entry).second)();
  }

  /**
   * have the factory generate an object of some requested type.
   */
  BaseType *Create(UniqueTypeID id) {
    if (objectFactories.size() > 0) {
      iterator objectEntry = objectFactories.find(id);
      if (objectEntry != objectFactories.end()) {
	return ((*objectEntry).second)();
      }
    }
    return NULL;
  }

protected:

  /** registration map of available object types */
  std::map<UniqueTypeID, Manufacture> objectFactories;

  /* don't allow this class to be directly instantiated */
  Factory() {
  }
  virtual ~Factory() {
  }
};



#endif /* __FACTORY_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=4 tabstop=8
