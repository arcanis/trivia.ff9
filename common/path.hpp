#pragma once

#include <list>
#include <string>

#include "memoryrange.hpp"

class Path
{

public:

    inline Path( void );

    inline Path( Path const & path );

    Path( std::string const & orig );

public:

    inline Path & push( std::string const & part );

    inline Path & pop( void );

public:

    std::string string(void) const;

public:

    Path const & dump( MemoryRange const & range ) const;

private:

    std::list< std::string > m_partList;

};

Path::Path( void )
{
}

Path::Path( Path const & path )
    : m_partList( path.m_partList )
{
}

Path & Path::push( std::string const & part )
{
    this->m_partList.push_back( part );

    return * this;
}

Path & Path::pop( void )
{
    this->m_partList.pop_back( );

    return * this;
}
