/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef __BZAUTHD_THREAD_H__
#define __BZAUTHD_THREAD_H__

/* The GuardedSingleton class like other signletons ensures only one instance of 
   of its template type. It can be retrieved in the form GuardedSingleton::guard().instance().
   While the creation and destruction of the instance is not thread safe, it is ensured that
   only one thread can access any function of the class at a time.
*/

template < typename T >
class GuardedSingleton {
protected:

  // protection from instantiating a non-singleton Singleton
  GuardedSingleton() { }
  GuardedSingleton(T* instancePointer) { Guard::_instance = instancePointer; }
  GuardedSingleton(const GuardedSingleton &) { } // do not use
  GuardedSingleton& operator=(const GuardedSingleton&) { return *this; } // do not use
  ~GuardedSingleton() { Guard::_instance = 0; } // do not delete

public:
  friend class Guard;

  class Guard
  {
  private:
    static T* _instance;

    Guard() {
      init(); acquire();
    }

#if defined(HAVE_PTHREADS)
    pthread_mutex_t mutex;
    void init() { pthread_mutex_init(&mutex, NULL); }
    void acquire() { pthread_mutex_lock(&mutex); }
    void release() { pthread_mutex_unlock(&mutex); }
#elif defined(_WIN32)
    CRITICAL_SECTION critical;
    void init() { InitializeCriticalSection(&critical); }
    void acquire() { EnterCriticalSection(&critical); }
    void release() { LeaveCriticalSection(&critical); }
#else
#   error no guards
#endif

  public:
    ~Guard() {
      release();
    }

    static void destroy() {
      if ( _instance != 0 ) {
        delete(_instance);
        _instance = 0;
      }
    }

    friend class GuardedSingleton;

     /** returns a singleton
     */
    inline static T& instance() {
      if ( _instance == 0 ) {
        _instance = new T;
        // destroy the singleton when the application terminates
#ifdef HAVE_ATEXIT
        atexit(destroy);
#endif
      }
      return *_instance;
    }

    /** returns a singleton pointer
     */
    inline static T* pInstance() {
      if (_instance == 0) {
        _instance = new T;
#ifdef HAVE_ATEXIT
        atexit(destroy);
#endif
      }
      return _instance;
    }

    /** returns a const singleton reference
     */
    inline static const T& constInstance() { return *instance(); }
  };

  static Guard guard() {
    return Guard();
  }
};

#define INSTANTIATE_GUARDED_SINGLETON(T) template <> T* GuardedSingleton<T>::Guard::_instance = (T*)0;

#endif // __BZAUTHD_THREAD_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
