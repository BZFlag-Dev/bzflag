
#ifndef BOOST_MPL_MAP_AUX_ERASE_IMPL_HPP_INCLUDED
#define BOOST_MPL_MAP_AUX_ERASE_IMPL_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2003-2004
// Copyright David Abrahams 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source$
// $Date: 2004-09-05 11:45:19 +0200 (Sun, 05 Sep 2004) $
// $Revision: 24912 $

#include <boost/mpl/erase_fwd.hpp>
#include <boost/mpl/map/aux_/erase_key_impl.hpp>
#include <boost/mpl/map/aux_/tag.hpp>

namespace boost { namespace mpl {

template<>
struct erase_impl< aux::map_tag >
{
    template< 
          typename Map
        , typename Pos
        , typename unused_
        > 
    struct apply
        : erase_key_impl<aux::map_tag>
            ::apply<Map,typename Pos::type::first>
    {
    };
};

}}

#endif // BOOST_MPL_MAP_AUX_ERASE_IMPL_HPP_INCLUDED
