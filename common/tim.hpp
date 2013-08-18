#pragma once

#include <string>

#include <boost/cstdint.hpp>

#include "memoryrange.hpp"
#include "vram.hpp"

class TIM
{

public:

    static TIM fromRange( MemoryRange range );

    static TIM fromFile( std::string const & path );

public:

    inline TIM( boost::uint32_t left, boost::uint32_t top, boost::uint32_t width, boost::uint32_t height, boost::uint8_t bpp, std::vector< boost::uint8_t > data );

public:

    inline boost::uint32_t left( void ) const;

    inline boost::uint32_t top( void ) const;

    inline boost::uint32_t width( void ) const;

    inline boost::uint32_t height( void ) const;

    inline boost::uint8_t bpp( void ) const;

    inline std::vector< boost::uint8_t > const & data( void ) const;

public:

    TIM const & apply( VRAM & vram ) const;

    TIM & apply( VRAM & vram );

private:

    boost::uint32_t m_left;

    boost::uint32_t m_top;

    boost::uint32_t m_width;

    boost::uint32_t m_height;

    boost::uint8_t m_bpp;

    std::vector< boost::uint8_t > m_data;

};

TIM::TIM( boost::uint32_t left, boost::uint32_t top, boost::uint32_t width, boost::uint32_t height, boost::uint8_t bpp, std::vector< boost::uint8_t > data )
    : m_left( left )
    , m_top( top )
    , m_width( width )
    , m_height( height )
    , m_bpp( bpp )
    , m_data( data )
{
}

boost::uint32_t TIM::left( void ) const
{
    return m_left;
}

boost::uint32_t TIM::top( void ) const
{
    return m_top;
}

boost::uint32_t TIM::width( void ) const
{
    return m_width;
}

boost::uint32_t TIM::height( void ) const
{
    return m_height;
}

boost::uint8_t TIM::bpp( void ) const
{
    return m_bpp;
}

std::vector< boost::uint8_t > const & TIM::data( void ) const
{
    return m_data;
}
