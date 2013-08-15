#pragma once

#include <iostream>
#include <stdexcept>

#include <boost/spirit/include/qi.hpp>

#include "memoryrange.hpp"

template < typename Type >
void parse( MemoryRange & range, Type const & grammar )
{
    boost::uint8_t const * current = range.current( );

    if ( ! boost::spirit::qi::parse( current, range.end( ), grammar ) )
        throw std::runtime_error( "Parsing failed" );

    range.current( current );
}

template < typename Type, typename Output >
void parse( MemoryRange & range, Type const & grammar, Output & result )
{
    boost::uint8_t const * current = range.current( );

    if ( ! boost::spirit::qi::parse( current, range.end( ), grammar, result ) )
        throw std::runtime_error( "Parsing failed" );

    range.current( current );
}
