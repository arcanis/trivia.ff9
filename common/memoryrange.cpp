#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <boost/cstdint.hpp>

#include "memoryrange.hpp"

MemoryRange::MemoryRange( std::vector< boost::uint8_t > const & data )
    : m_begin( & data[ 0 ] )
    , m_current( m_begin )
    , m_end( m_begin + data.size( ) )
{
}

MemoryRange::MemoryRange( boost::uint8_t const * begin, boost::uint8_t const * end )
{
    if ( begin > end )
        std::swap( begin, end );

    this->m_begin = this->m_current = begin;
    this->m_end = end;
}

unsigned long MemoryRange::seek( SeekSource whence, long offset )
{
    boost::uint8_t const * current;

    switch ( whence ) {

        case SeekSet:
            current = this->m_begin + offset;
        break;

        case SeekCur:
            current = this->m_current + offset;
        break;

        case SeekEnd:
            current = this->m_end - offset;
        break;

    }

    if ( current < this->m_begin || current > this->m_end ) {

        std::ostringstream errorBuilder;

        void const * ac = reinterpret_cast< void const * >( current );

        void const * bb = reinterpret_cast< void const * >( this->m_begin )
                 , * be = reinterpret_cast< void const * >( this->m_end );

        errorBuilder << ac;
        errorBuilder << " is not inside ";
        errorBuilder << "(" << bb << "," << be << ")";

        throw std::out_of_range( "Invalid seek (" + errorBuilder.str( ) + ")" );

    }

    this->m_current = current;

    return this->m_current - this->m_begin;
}

unsigned long MemoryRange::crop( SeekSource whence, unsigned long offset, unsigned long size )
{
    boost::uint8_t const * begin, * end;

    switch ( whence ) {

        case SeekSet:
            begin = this->m_begin + offset;
        break;

        case SeekCur:
            begin = this->m_current + offset;
        break;

        case SeekEnd:
            begin = this->m_end - offset;
        break;

    }

    end = begin + size;

    if ( begin < this->m_begin || begin > this->m_end || end < this->m_begin || end > this->m_end ) {

        std::ostringstream errorBuilder;

        void const * ab = reinterpret_cast< void const * >( begin )
                 , * ae = reinterpret_cast< void const * >( end );

        void const * bb = reinterpret_cast< void const * >( this->m_begin )
                 , * be = reinterpret_cast< void const * >( this->m_end );

        errorBuilder << "(" << ab << "," << ae << ")";
        errorBuilder << " is not inside ";
        errorBuilder << "(" << bb << "," << be << ")";

        throw std::out_of_range( "Invalid crop (" + errorBuilder.str( ) + ")" );

    }

    this->m_begin = this->m_current = begin;
    this->m_end = end;

    return this->m_end - this->m_begin;
}
