/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __SINGLETON_H__
#define __SINGLETON_H__

/* system headers */
#ifdef HAVE_ATEXIT
#	ifdef HAVE_CSTDLIB
#include <cstdlib>
using std::atexit;
#	else
#include <stdlib.h>
#	endif
#endif

/* Singleton template class
 *
 * This template class pattern provides a traditional Singleton pattern.
 * Allows you to designate a single-instance global class by inheriting
 * off of the template class.
 *
 * Example:
 *
 *   class Whatever : public Singleton<Whatever> ...
 *
 * The class will need to provide either a public or a protected friend
 * constructor:
 *
 *   friend class Singleton<Whatever>;
 *
 * The class will also need to initialize it's own instance in a single
 * compilation unit (a .cxx file):
 *
 *   // statically initialize the instance to nothing
 *   template <>
 *   Whatever* Singleton<Whatever>::_instance = 0;
 *
 * The class can easily be extended to support different allocation
 * mechanisms or multithreading access.  This implementation, however,
 * only uses new/delete and is not thread safe.
 *
 * The Singleton will automatically get destroyed when the application
 * terminates (via an atexit() hook) unless the inheriting class has an
 * accessible destructor.
 */
template < typename T >
class Singleton {

private:

  static T* _instance;

protected:

  // protection from instantiating a non-singleton Singleton
  Singleton() { }
  Singleton(T* pInstance) { _instance = pInstance; }
  Singleton(const Singleton &) { } // do not use
  Singleton& operator=(const Singleton&) { return *this; } // do not use
  ~Singleton() { _instance = 0; } // do not delete

  static void destroy() {
    if ( _instance != 0 ) {
      delete(_instance);
      _instance = 0;
    }
  }

public:

  /** returns a singleton
   */
  inline static T& instance() {
    if ( _instance == 0 ) {
      _instance = new T;
      // destroy the singleton when the application terminates
#ifdef HAVE_ATEXIT
      atexit(Singleton::destroy);
#endif
    }
    return *Singleton::_instance;
  }

  /** returns a singleton pointer
   */
  inline static T* pInstance() {
    if (_instance == 0) {
      _instance = new T;
#ifdef _WIN32
      atexit(Singleton::destroy);
#else
      std::atexit(Singleton::destroy);
#endif
    }
    return Singleton::_instance;
  }

  /** returns a const singleton reference
   */
  inline static const T& constInstance() { return *instance(); }
};

#endif /* __SINGLETON_H__ */


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
