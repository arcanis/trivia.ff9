#pragma once

#include <list>
#include <string>
#include <vector>

#include <boost/cstdint.hpp>

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

    std::vector< boost::uint8_t > read( void ) const;

public:

    Path const & dump( char const * data, unsigned int size ) const;

    Path const & dump( MemoryRange const & range ) const;

    Path const & dumpTga( boost::uint16_t width, boost::uint16_t height, std::vector< boost::uint32_t > const & data ) const;

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
