#ifndef FOREACH_H
#define FOREACH_H


#include <stdint.h>

#include "typeof.h"


//============================================================================//
//
//  NOTE:
//
//    for (std::set<std::string>::const_iterator it;
//         it = constSet.begin(); it != constSet.end(); ++it) {
//      std::cout << *it << std::endl;
//    }
//
//    becomes
//
//    foreach (str, constSet) {
//      std::cout << str << std::endl;
//    }
//
//============================================================================//
//
// NOTE: the 'tmp' variants can be used with temporary containers
//
//   foreach             (varname, container) { ... }
//
//   foreach_reverse     (varname, container) { ... }
//
//   foreach_tmp         (varname, container) { ... }
//
//   foreach_reverse_tmp (varname, container) { ... }
//
//============================================================================//

#define foreach_iterator(VAR) (VAR##_FOREACH_ITER)

//============================================================================//

template <typename T>
inline bool foreach_set_inc(T& next, T& iter) {
  next = iter;
  ++next;
  return true;
}

//============================================================================//

template <typename T>
struct foreach_container_ptr {
  foreach_container_ptr(T& t) : ptr(&t), breaked(false) {}
  T* operator->() { return ptr; }
  operator bool() const { return false; } // for the "if (X) {} else" setup
  T* ptr;
  bool breaked; // used to make `break` work inside foreach loops
};

//============================================================================//
//
//  for the `_tmp` variants
//

template <typename T>
struct foreach_container_tmp : public T {
  foreach_container_tmp(const T& t) : T(t), breaked(false) {}
  T* operator->() { return this; }
  operator bool() const { return false; } // for the "if (X) {} else" setup
  bool breaked; // used to make `break` work inside foreach loops
};

//============================================================================//

#define FOREACH_VAR_SETUP(ITER, ITNEXT, BEGIN) \
  typeof(BEGIN) ITNEXT, ITER = (BEGIN)

#define FOREACH_BASIS(VAR, CON, BEGIN, END, CON_WRAPPER) \
  if (CON_WRAPPER<typeof(CON)> VAR##_FOREACH_CON = (CON)) {} else \
  for (FOREACH_VAR_SETUP(VAR##_FOREACH_ITER, \
                         VAR##_FOREACH_NEXT, \
                         VAR##_FOREACH_CON->BEGIN()); \
       bool VAR##_FOREACH_ONCE = /* bool used in the next for statement */ \
         !VAR##_FOREACH_CON.breaked && \
         (VAR##_FOREACH_CON.breaked = true) && \
         (VAR##_FOREACH_ITER != VAR##_FOREACH_CON->END()) && \
         foreach_set_inc(VAR##_FOREACH_NEXT, VAR##_FOREACH_ITER); \
       VAR##_FOREACH_ITER = VAR##_FOREACH_NEXT) \
    /* the following for-loop is used to setup the VAR reference */ \
    for (typeof(*(VAR##_FOREACH_CON->BEGIN())) VAR = *(VAR##_FOREACH_ITER); \
         VAR##_FOREACH_ONCE; \
         VAR##_FOREACH_ONCE = false, VAR##_FOREACH_CON.breaked = false)

#define foreach(VAR, CON)             FOREACH_BASIS(VAR, CON,  begin,  end, foreach_container_ptr)
#define foreach_tmp(VAR, CON)         FOREACH_BASIS(VAR, CON,  begin,  end, foreach_container_tmp)
#define foreach_reverse(VAR, CON)     FOREACH_BASIS(VAR, CON, rbegin, rend, foreach_container_ptr)
#define foreach_reverse_tmp(VAR, CON) FOREACH_BASIS(VAR, CON, rbegin, rend, foreach_container_tmp)

//============================================================================//
//============================================================================//
//
//  Numbers specialization
//

template <typename T>
struct foreach_container_number {
  struct container {
    container(T t) : count(t) {}
    struct iterator {
      iterator() {}
      iterator(T i) : index(i) {}
      iterator& operator++() { ++index; return *this; }
      bool operator!=(const iterator& i) const { return i.index != index; }
      const T& operator*() { return index; }
      T index;
    };
    struct reverse_iterator {
      reverse_iterator() {}
      reverse_iterator(T i) : index(i) {}
      reverse_iterator& operator++() { --index; return *this; }
      bool operator!=(const reverse_iterator& i) const { return i.index != index; }
      const T& operator*() { return index; }
      T index;
    };
    iterator         begin()  { return iterator((T)0); }
    iterator         end()    { return iterator(count); }
    reverse_iterator rbegin() { return reverse_iterator(count - (T)1); }
    reverse_iterator rend()   { return reverse_iterator((T)(-1)); }
    T count;
  };
  foreach_container_number(T t) : con(t), breaked(false) {}
  container* operator->() { return &con; }
  operator bool() const { return false; }
  container con;
  bool breaked;
};

#define FOREACH_NUMBER_CONTAINER(T) \
  template <> struct foreach_container_ptr<T> \
            : public foreach_container_number<T> { \
    foreach_container_ptr<T>(T t) : foreach_container_number<T>(t) {} }

FOREACH_NUMBER_CONTAINER(int8_t);
FOREACH_NUMBER_CONTAINER(int16_t);
FOREACH_NUMBER_CONTAINER(int32_t);
FOREACH_NUMBER_CONTAINER(int64_t);
FOREACH_NUMBER_CONTAINER(uint8_t);
FOREACH_NUMBER_CONTAINER(uint16_t);
FOREACH_NUMBER_CONTAINER(uint32_t);
FOREACH_NUMBER_CONTAINER(uint64_t);
FOREACH_NUMBER_CONTAINER(float);
FOREACH_NUMBER_CONTAINER(double);
FOREACH_NUMBER_CONTAINER(long double);

//============================================================================//
//
//  Array specialization - does not work, wrong type...
//

/*
template <typename T>
struct foreach_container_ptr<T[]> {
  struct container {
    container(T* p) : base(p) {}
    struct iterator {
      iterator() {}
      iterator(T* p) : ptr(p) {}
      iterator& operator++() { ++ptr; return *this; }
      bool operator!=(const iterator& i) const { return i.ptr != ptr; }
      T& operator*() { return *ptr; }
      T* ptr;
    };
    struct reverse_iterator {
      reverse_iterator() {}
      reverse_iterator(T* p) : ptr(p) {}
      reverse_iterator& operator++() { --ptr; return *this; }
      bool operator!=(const reverse_iterator& i) const { return i.ptr != ptr; }
      T& operator*() { return *ptr; }
      T* ptr;
    };
    iterator         begin()  { return iterator(base); }
    iterator         end()    { return iterator(base + sizeof(T)/sizeof(T)); }
    reverse_iterator rbegin() { return reverse_iterator(base + (sizeof(T) - 1)); }
    reverse_iterator rend()   { return reverse_iterator(base - 1); }
    T* const base;
  };
  foreach_container_ptr(T* p) : con(p), breaked(false) {}
  container* operator->() { return &con; }
  operator bool() const { return false; }
  container con;
  bool breaked;
};
*/

//============================================================================//
//
//  String specialization   (just `char*` and `const char*`; no arrays, yet)
//

template <typename  T>
struct foreach_container_string {
  struct container {
    container(T* p) : first(p), last(findnul(p)) {}
    T* findnul(T* s) {
      while (*s) { s++; } return s;
    }
    struct iterator {
      iterator() {}
      iterator(T* p) : ptr(p) {}
      iterator& operator++() { ++ptr; return *this; }
      bool operator!=(const iterator& p) const { return p.ptr != ptr; }
      T& operator*() { return *ptr; }
      T* ptr;
    };
    struct reverse_iterator {
      reverse_iterator() {}
      reverse_iterator(T* p) : ptr(p) {}
      reverse_iterator& operator++() { ptr--; return *this; }
      bool operator!=(const reverse_iterator& p) const { return p.ptr != ptr; }
      T& operator*() { return *ptr; }
      T* ptr;
    };
    iterator         begin()  { return iterator(first); }
    iterator         end()    { return iterator(last); }
    reverse_iterator rbegin() { return reverse_iterator(last - 1); }
    reverse_iterator rend()   { return reverse_iterator(first - 1); }
    T* const first;
    T* const last;
  };
  foreach_container_string(T* t) : con(t), breaked(false) {}
  container* operator->() { return &con; }
  operator bool() const { return false; }
  container con;
  bool breaked;
};

#define FOREACH_STRING_CONTAINER(T) \
  template <> struct foreach_container_ptr<T*> \
            : public foreach_container_string<T> { \
    foreach_container_ptr<T*>(T* t) : foreach_container_string<T>(t) {} }

FOREACH_STRING_CONTAINER(char);
FOREACH_STRING_CONTAINER(const char);

//============================================================================//

#endif // FOREACH_H
