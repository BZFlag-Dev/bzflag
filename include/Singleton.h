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

#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <stdlib.h>


/* Singleton template class
 *
 * This template class pattern provides the Singleton pattern, allowing control
 * over how an object is created/destroyed and what kind of threading model
 * to use.
 */


/** CreationPolicy using new and delete
 */
template < typename T >
class CreateUsingNew {

 private:

  CreateUsingNew(CreateUsingNew const&) { } // unused
  CreateUsingNew& operator=(CreateUsingNew const&) { return *this; } // usused

 protected:

  CreateUsingNew() { } // unused
  ~CreateUsingNew() { } // unused

  static T* CreateInstance() {
    return new T();
  }

  static void DestroyInstance(T* instance) {
    delete instance;
    instance = NULL;
  }
};


/** CreationPolicy using malloc and free
 */
template < typename T >
class CreateUsingMalloc {

 private:

  CreateUsingMalloc(CreateUsingMalloc const&) { } // unused
  //  CreatUsingMalloc& operator=(CreateUsingMalloc const&) { return *this; } //usused

 protected:

  CreateUsingMalloc() { } // unused
  ~CreateUsingMalloc() { } // unused

  static T* CreateInstance() {
    return malloc(sizeof(T));
  }

  static void DestroyInstance(T* instance) {
    free(instance);
    instance = NULL;
  }
};


/* CreationPolicy using a pure static on the stack is left as an exercise
 * to the needy.  :P
 */


/** ThreadingModel that assumes single threaded access.
 * basically does nothing, but it exposes the interface that needs to
 * be implmented for a multi-threading model.
 */
class SingleThreaded {

 private:

  friend class LockThread;
  SingleThreaded(SingleThreaded const&) { }
  SingleThreaded& operator=(SingleThreaded const&) { return *this; }

 protected:

  SingleThreaded() { }
  ~SingleThreaded() { }

 public:

  class LockThread {

   private:

    static void Lock() { }
    static void Unlock() { }

   public:

    LockThread() {
      Lock();
    }
    ~LockThread() {
      Unlock();
    }
  };
};


/* ThreadingModel for multithreaded access is left as an exercise to the
 * the needy. :P
 */


/** This template class implements a traditional singleton interface.
 * You provide a class, a creation policy, and a threading model.  Creation
 * policies need to define CreateInstance() and DestroyInstance().  Threading
 * model needs to define a default LockThread class.
 */
template < typename T, typename CreationPolicy = CreateUsingNew<T>, typename ThreadingModel = SingleThreaded >
class Singleton : public CreationPolicy, public ThreadingModel {

 private:

  static T* _instance;

  Singleton(const Singleton &) { } // do not use
  Singleton& operator=(const Singleton&) { return *this; } // do not use

  static void destroy() {
    if ( _instance != 0 ) {
      LockThread lock;
      if ( _instance != 0 ) {
	CreationPolicy::DestroyInstance(_instance);
	_instance = 0;
      }
    }
  }

 protected:

  // protection from instantiating a non-singleton Singleton
  Singleton() { }
  ~Singleton() { _instance = 0; } // do not delete

 public:

  /** returns a singleton
   */
  static T& instance() {
    if ( _instance == 0 ) {
      LockThread lock;
      if ( _instance == 0 ) {
	_instance = CreationPolicy::CreateInstance();
	// destroy the singleton when the application terminates
	std::atexit(Singleton::destroy);
      }
    }
    return *(Singleton::_instance);
  }

  /** returns a const singleton reference
   */
  inline static const T& constInstance() { return instance(); }
};

// statically initialize the instance to nothing
template < typename T, typename CreationPolicy, typename ThreadingModel >
T* Singleton<T, CreationPolicy, ThreadingModel>::_instance = 0;

#endif /* __SINGLETON_H__ */

/* ex: shiftwidth=2 tabstop=8
 * Local Variables: ***
 * mode:C++ ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 */

