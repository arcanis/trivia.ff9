#include <stdexcept>
#include <vector>

#include <boost/spirit/include/qi.hpp>
#include <boost/cstdint.hpp>

#include "constants.hpp"
#include "memoryrange.hpp"
#include "parse.hpp"
#include "path.hpp"
#include "tim.hpp"
#include "vram.hpp"

namespace qi = boost::spirit::qi;

TIM TIM::fromRange( MemoryRange range )
{
    boost::uint32_t magicNumber;
    parse( range, qi::byte_, magicNumber );
    if ( magicNumber != 0x10 )
        throw std::runtime_error( "Bad magic number." );

    boost::uint32_t version;
    parse( range, qi::byte_, version );
    if ( version != 0 )
        throw std::runtime_error( "Unsupported version." );

    parse( range, qi::word );

    boost::uint32_t flags;
    parse( range, qi::dword, flags );

    boost::uint8_t bpp = flags & 0x00000003;
    bpp = bpp ? bpp * 8 : 4;

    parse( range, qi::dword );
    parse( range, qi::dword );

    boost::uint16_t imageXOrigin;
    parse( range, qi::dword, imageXOrigin );

    boost::uint16_t imageYOrigin;
    parse( range, qi::dword, imageYOrigin );

    boost::uint16_t imageWidth;
    parse( range, qi::dword, imageWidth );
    imageWidth = imageWidth * ( 16.0 / bpp );

    boost::uint16_t imageHeight;
    parse( range, qi::dword, imageHeight );

    return TIM( imageXOrigin, imageYOrigin, imageWidth, imageHeight, std::vector< boost::uint8_t >( range.current( ), range.end( ) ) );
}

TIM TIM::fromFile( std::string const & path )
{
    Path file( path );
    std::vector< boost::uint8_t > content = file.read( );

    MemoryRange range( content );
    return TIM::fromRange( range );
}

TIM const & TIM::apply( VRAM & vram ) const
{
    for ( int y = m_top, ry = 0; ry < m_height; ++ y, ++ ry ) {
        for ( int x = m_left, rx = 0; rx < m_width; ++ x, ++ rx ) {
            vram[ y * VRAM_WIDTH + x ] = m_data[ ry * m_width + rx ];
        }
    }

    return * this;
}

TIM & TIM::apply( VRAM & vram )
{
    const_cast< TIM const & >( * this ).apply( vram );

    return * this;
}
