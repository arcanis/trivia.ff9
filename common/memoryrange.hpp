#pragma once

#include <vector>

#include <boost/cstdint.hpp>

class MemoryRange {

public:

    enum SeekSource {
	SeekSet,
	SeekCur,
	SeekEnd
    };

public:

    MemoryRange( std::vector< boost::uint8_t > const & data );

    MemoryRange( boost::uint8_t const * begin, boost::uint8_t const * end );

public:

    inline unsigned long size( void ) const;

public:

    unsigned long seek( SeekSource whence, long offset );

    unsigned long crop( SeekSource whence, unsigned long offset, unsigned long size );

public:

    inline boost::uint8_t const * begin( void ) const;

    inline boost::uint8_t const * end( void ) const;

public:

    inline boost::uint8_t const * current( void ) const;

    inline MemoryRange & current( boost::uint8_t const * current );

private:

    boost::uint8_t const * m_begin;

    boost::uint8_t const * m_current;

    boost::uint8_t const * m_end;

};

unsigned long MemoryRange::size( void ) const
{
    return this->m_end - this->m_begin;
}

boost::uint8_t const * MemoryRange::begin( void ) const
{
    return this->m_begin;
}

boost::uint8_t const * MemoryRange::end( void ) const
{
    return this->m_end;
}

boost::uint8_t const * MemoryRange::current( void ) const
{
    return this->m_current;
}

MemoryRange & MemoryRange::current( boost::uint8_t const * current )
{
    this->m_current = current;

    return * this;
}
