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

    parse( range, qi::word );
    parse( range, qi::word );

    boost::uint16_t imageXOrigin;
    parse( range, qi::word, imageXOrigin );

    boost::uint16_t imageYOrigin;
    parse( range, qi::word, imageYOrigin );

    boost::uint16_t imageWidth;
    parse( range, qi::word, imageWidth );
    imageWidth = imageWidth * ( 16.0 / bpp );

    boost::uint16_t imageHeight;
    parse( range, qi::word, imageHeight );

    return TIM( imageXOrigin, imageYOrigin, imageWidth, imageHeight, bpp, std::vector< boost::uint8_t >( range.current( ), range.end( ) ) );
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
    #define CEIL( n, factor ) ( ( n ) + ( ( n ) % ( factor ) ? ( factor ) - ( n ) % ( factor ) : 0 ) )
    unsigned char bytes = CEIL( m_bpp, 8 ) / 8;
    unsigned int byteWidth = CEIL( m_width * m_bpp, 8 ) / 8;

    for ( unsigned int y = m_top, ry = 0; ry < m_height; ++ y, ++ ry ) {
        boost::uint8_t const * from = & m_data[ ry * byteWidth ];
        boost::uint8_t * to = reinterpret_cast< boost::uint8_t * >( & vram[ y * VRAM_WIDTH + m_left ] );

        for ( int t = 0; t < byteWidth; ++ t ) {
            to[ t ] = from[ t ];
        }
    }

    return * this;
}

TIM & TIM::apply( VRAM & vram )
{
    const_cast< TIM const & >( * this ).apply( vram );

    return * this;
}
