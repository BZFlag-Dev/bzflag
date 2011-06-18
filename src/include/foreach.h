#ifndef FOREACH_H
#define FOREACH_H


#include "typeof.h"


// NOTE: the 'copy' variants can be used with temporary containers


// foreach              (varname, container) { ... }

// foreach_reverse      (varname, container) { ... }

// foreach_copy         (varname, container) { ... }

// foreach_copy_reverse (varname, container) { ... }


#define FOREACH_ITERATOR(VAR) (VAR##_ITER)


//============================================================================//

template <typename T>
inline bool foreach_set_inc(T& next, T& iter) {
  next = iter;
  next++;
  return true;
}

#define FOREACH_VAR_SETUP(ITER, ITNEXT, BEGIN) \
  typeof(BEGIN) ITNEXT, ITER = (BEGIN)

//============================================================================//

template <typename T>
struct foreach_container_pointer {
  foreach_container_pointer(T& t) : ptr(&t) {}
  T* operator->() { return ptr; }
  operator bool() const { return false; } // for the "if (X) {} else" setup
  T* ptr;
};

#define FOREACH_BASIS(VAR, CON, BEGIN, END) \
  if (foreach_container_pointer<typeof(CON)> FOREACH_CON = (CON)) {} else \
  for (FOREACH_VAR_SETUP(VAR##_ITER, VAR##_NEXT, FOREACH_CON->BEGIN()); \
       bool VAR##_FOREACH_ONCE = /* bool used in the next for statement */ \
         (VAR##_ITER != FOREACH_CON->END()) \
         && foreach_set_inc(VAR##_NEXT, VAR##_ITER); \
       VAR##_ITER = VAR##_NEXT) \
    /* the following for-loop is used to setup the VAR reference */ \
    for (typeof(*(FOREACH_CON->BEGIN())) VAR = *(VAR##_ITER); \
         VAR##_FOREACH_ONCE; \
         VAR##_FOREACH_ONCE = false)

#define foreach(VAR, CON)         FOREACH_BASIS(VAR, CON,  begin,  end)
#define foreach_reverse(VAR, CON) FOREACH_BASIS(VAR, CON, rbegin, rend)

//============================================================================//

template <typename T>
struct foreach_container_copy : public T {
  foreach_container_copy(const T& t) : T(t) {}
  operator bool() const { return false; } // for the "if (X) {} else" setup
};

#define FOREACH_COPY_BASIS(VAR, CON, BEGIN, END) \
  if (foreach_container_copy<typeof(CON)> FOREACH_CON = (CON)) {} else \
  for (FOREACH_VAR_SETUP(VAR##_ITER, VAR##_NEXT, FOREACH_CON.BEGIN()); \
       bool VAR##_FOREACH_ONCE = /* bool used in the next for statement */ \
         (VAR##_ITER != FOREACH_CON.END()) \
         && foreach_set_inc(VAR##_NEXT, VAR##_ITER); \
       VAR##_ITER = VAR##_NEXT) \
    /* the following for-loop is used to setup the VAR reference */ \
    for (typeof(*(FOREACH_CON.BEGIN())) VAR = *(VAR##_ITER); \
         VAR##_FOREACH_ONCE; \
         VAR##_FOREACH_ONCE = false)

#define foreach_copy(VAR, CON)         FOREACH_COPY_BASIS(VAR, CON,  begin,  end)
#define foreach_copy_reverse(VAR, CON) FOREACH_COPY_BASIS(VAR, CON, rbegin, rend)

//============================================================================//


#endif // FOREACH_H
