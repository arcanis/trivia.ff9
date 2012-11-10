#pragma once

class MemoryRange {

public:

    enum SeekSource {
	SeekSet,
	SeekCur,
	SeekEnd
    };

public:

    MemoryRange( char const * begin, char const * end );

public:

    MemoryRange & seek( SeekSource whence, long offset );

    MemoryRange & crop( unsigned long offset, unsigned long size );

public:

    inline char const * begin( void ) const;

    inline char const * end( void ) const;

public:

    inline char const * current( void ) const;

    inline MemoryRange & current( char const * current );

private:

    char const * m_begin;

    char const * m_current;

    char const * m_end;

};

char const * MemoryRange::begin( void ) const
{
    return this->m_begin;
}

char const * MemoryRange::end( void ) const
{
    return this->m_end;
}

char const * MemoryRange::current( void ) const
{
    return this->m_current;
}

MemoryRange & MemoryRange::current( char const * current )
{
    this->m_current = current;

    return * this;
}
