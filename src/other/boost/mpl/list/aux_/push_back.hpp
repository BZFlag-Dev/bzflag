
#ifndef BOOST_MPL_LIST_AUX_PUSH_BACK_HPP_INCLUDED
#define BOOST_MPL_LIST_AUX_PUSH_BACK_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source$
// $Date: 2004-09-02 17:41:37 +0200 (Thu, 02 Sep 2004) $
// $Revision: 24874 $

#include <boost/mpl/push_back_fwd.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/list/aux_/tag.hpp>

namespace boost { namespace mpl {

template< typename Tag > struct has_push_back_impl;

template<>
struct has_push_back_impl< aux::list_tag >
{
    template< typename Seq > struct apply
        : false_
    {
    };
};

}}

#endif // BOOST_MPL_LIST_AUX_PUSH_BACK_HPP_INCLUDED
