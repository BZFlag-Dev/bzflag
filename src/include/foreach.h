#ifndef FOREACH_H
#define FOREACH_H

#include "typeof.h"

template <typename T>
inline bool foreach_set_inc(T& next, T& iter) {
  next = iter;
  next++;
  return true;
}

#define FOREACH_VAR_SETUP(ITER, ITNEXT, BEGIN) \
  typeof(BEGIN) ITNEXT, ITER = (BEGIN)

#define FOREACH_BASIS(VAR, CON, BEGIN, END) \
  for (FOREACH_VAR_SETUP(VAR##_ITER, VAR##_NEXT, (CON).BEGIN()); \
       bool VAR##_FOREACH_ONCE = /* bool used in the next for statement */ \
         (VAR##_ITER != (CON).END()) \
         && foreach_set_inc(VAR##_NEXT, VAR##_ITER); \
       VAR##_ITER = VAR##_NEXT) \
  /* the following for loop is used to setup the VAR reference */ \
  for (typeof(*((CON).BEGIN())) VAR = *(VAR##_ITER); \
       VAR##_FOREACH_ONCE; \
       VAR##_FOREACH_ONCE = false)

#define foreach(VAR, CON)         FOREACH_BASIS(VAR, CON,  begin,  end)
#define foreach_reverse(VAR, CON) FOREACH_BASIS(VAR, CON, rbegin, rend)


#endif // FOREACH_H
