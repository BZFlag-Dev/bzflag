
#ifndef BOOST_MPL_INTEGRAL_C_TAG_HPP_INCLUDED
#define BOOST_MPL_INTEGRAL_C_TAG_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source$
// $Date: 2004-09-28 15:56:59 +0200 (Tue, 28 Sep 2004) $
// $Revision: 25453 $


#include <boost/mpl/aux_/adl_barrier.hpp>
#include <boost/mpl/aux_/config/static_constant.hpp>

BOOST_MPL_AUX_ADL_BARRIER_NAMESPACE_OPEN
struct integral_c_tag { BOOST_STATIC_CONSTANT(int, value = 0); };
BOOST_MPL_AUX_ADL_BARRIER_NAMESPACE_CLOSE
BOOST_MPL_AUX_ADL_BARRIER_DECL(integral_c_tag)

#endif // BOOST_MPL_INTEGRAL_C_TAG_HPP_INCLUDED
