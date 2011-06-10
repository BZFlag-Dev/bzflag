
#ifndef TYPEOF_H
#define TYPEOF_H


#if defined(__GNUC__)
#  define typeof(x) __typeof__(x)
#elif defined(__clang__) // clang also defines __GNUC__, so this is useless
#  define typeof(x) __typeof__(x)
#else
#  include <boost/typeof/typeof.hpp>
#  define typeof(x) BOOST_TYPEOF(x)
#endif


#endif // TYPEOF_H
