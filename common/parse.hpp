#pragma once

#include <boost/spirit/include/qi.hpp>

#include <iostream>
#include <stdexcept>

#include "memoryrange.hpp"

template < typename Type >
void parse( MemoryRange & range, Type const & grammar ) {
    char const * current = range.current( );
    if ( ! boost::spirit::qi::parse( current, range.end( ), grammar ) )
        throw std::runtime_error( "Parsing failed" );
    range.current( current );
}

template < typename Type, typename Output >
void parse( MemoryRange & range, Type const & grammar, Output & result ) {
    char const * current = range.current( );
    if ( ! boost::spirit::qi::parse( current, range.end( ), grammar, result ) )
        throw std::runtime_error( "Parsing failed" );
    range.current( current );
}
