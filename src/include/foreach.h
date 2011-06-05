#ifndef FOREACH_H
#define FOREACH_H

#include "typeof.h"

template <typename T>
inline bool foreach_set_inc(T& dst, T& src) {
  dst = src; dst++; return true;
}

#define FOREACH_VAR_SETUP(IT, ITNEXT, BEGIN) \
  typeof(BEGIN) ITNEXT, IT = (BEGIN)

#define foreach(IT, CON)\
  for (FOREACH_VAR_SETUP(IT, IT##_NEXT, (CON).begin()); \
       (IT != (CON).end()) && foreach_set_inc(IT##_NEXT, IT); \
        IT = IT##_NEXT)

#define foreach_reverse(IT, CON)\
  for (FOREACH_VAR_SETUP(IT, IT##_NEXT, (CON).rbegin()); \
       (IT != (CON).rend()) && foreach_set_inc(IT##_NEXT, IT); \
        IT = IT##_NEXT)

#endif // FOREACH_H
