
#ifndef BOOST_MPL_MULTISET_AUX_MULTISET0_HPP_INCLUDED
#define BOOST_MPL_MULTISET_AUX_MULTISET0_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source$
// $Date: 2004-09-02 17:41:37 +0200 (Thu, 02 Sep 2004) $
// $Revision: 24874 $

#include <boost/mpl/multiset/aux_/tag.hpp>
#include <boost/mpl/int.hpp>

namespace boost { namespace mpl {

template< int dummy_ = 0 >
struct multiset0
{
    typedef aux::multiset_tag tag;

    typedef int_<1> count_;
    static char (& key_count(...) )[count_::value];
    static char (& ref_key_count(...) )[count_::value];
};

}}

#endif // BOOST_MPL_MULTISET_AUX_MULTISET0_HPP_INCLUDED
