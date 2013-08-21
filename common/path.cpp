#include <algorithm>
#include <fstream>
#include <string>
#include <utility>

#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/tokenizer.hpp>
#include <boost/cstdint.hpp>

#include "memoryrange.hpp"
#include "path.hpp"

static boost::uint16_t native_to_little_u16( boost::uint16_t n ) {
    // todo if someone ask for it.
    // Please feel free to pull request, but it has to be portable.
    return n;
}

static boost::uint32_t native_to_little_u32( boost::uint32_t n ) {
    // todo if someone ask for it.
    // Please feel free to pull request, but it has to be portable.
    return n;
}

Path::Path( std::string const & orig )
{
    typedef boost::char_separator< char > separator;
    typedef boost::tokenizer< separator > tokenizer;

    separator sep( "/", "", boost::keep_empty_tokens );
    tokenizer tokens( orig, sep );

    for ( tokenizer::iterator it = tokens.begin( ); it != tokens.end( ); ++ it ) {
        this->m_partList.push_back( * it );
    }
}

std::vector< boost::uint8_t > Path::read( void ) const
{
    boost::filesystem::path pathname( this->string( ) );

    std::ifstream file( pathname.string( ).c_str( ), std::ios::in | std::ios::binary | std::ios::ate );
    std::ifstream::pos_type size = file.tellg( );
    file.seekg( 0, std::ios::beg );

    std::vector< boost::uint8_t > data( size );
    file.read( reinterpret_cast< char * >( & data[ 0 ] ), size );
    file.close( );

    return std::move( data );
}

Path const & Path::dump( char const * data, unsigned int size ) const
{
    boost::filesystem::path pathname( this->string( ) );

    std::string dirname = pathname.parent_path( ).string( );
    if ( ! dirname.empty( ) )
        boost::filesystem::create_directories( dirname );

    std::ofstream output;
    output.exceptions( std::ios_base::failbit | std::ios_base::badbit );
    output.open( pathname.string( ).c_str( ), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc );
    output.write( data, size );
    output.close( );

    return * this;
}

Path const & Path::dump( MemoryRange const & range ) const
{
    return this->dump( reinterpret_cast< char const * >( range.current( ) ), range.end( ) - range.current( ) );
}

Path const & Path::dump( std::string const & text ) const
{
    return this->dump( text.c_str( ), text.length( ) );
}

Path const & Path::dumpBmp( boost::uint16_t width, boost::uint16_t height, std::vector< boost::uint32_t > const & data ) const
{
    boost::filesystem::path pathname( this->string( ) );

    std::string dirname = pathname.parent_path( ).string( );
    if ( ! dirname.empty( ) )
        boost::filesystem::create_directories( dirname );

    std::ofstream output;
    output.exceptions( std::ios_base::failbit | std::ios_base::badbit );
    output.open( pathname.string( ).c_str( ), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc );

    boost::uint32_t rowByteCount = width * 3;
    if ( rowByteCount % 4 )
        rowByteCount += 4 - width % 4;

    boost::uint16_t littleEndianSize = native_to_little_u32( 14 + 12 + rowByteCount * height );

    output.write( "\x42\x4d", 2 );
    output.write( reinterpret_cast< char const * >( & littleEndianSize ), 4 );
    output.write( "\x00\x00", 2 );
    output.write( "\x00\x00", 2 );
    output.write( "\x00\x00\x00\x00", 4 );

    boost::uint16_t littleEndianWidth = native_to_little_u32( width );
    boost::uint16_t littleEndianHeight = native_to_little_u32( height );

    output.write( "\x0c\x00\x00\x00", 4 );
    output.write( reinterpret_cast< char const * >( & littleEndianWidth ), 2 );
    output.write( reinterpret_cast< char const * >( & littleEndianHeight ), 2 );
    output.write( "\x01\x00", 2 );
    output.write( "\x18\x00", 2 );

    boost::uint8_t * bmpdata = new boost::uint8_t[ rowByteCount * height ];
    for ( boost::uint16_t y = 0; y < height; ++ y ) {
        for ( boost::uint16_t x = 0; x < width; ++ x ) {
            boost::uint32_t color = data[ ( height - y - 1 ) * width + x ];
            bmpdata[ y * rowByteCount + x * 3 + 0 ] = ( color >>  0 ) & 0xFF;
            bmpdata[ y * rowByteCount + x * 3 + 1 ] = ( color >>  8 ) & 0xFF;
            bmpdata[ y * rowByteCount + x * 3 + 2 ] = ( color >> 16 ) & 0xFF;
        }
    }

    output.write( reinterpret_cast< char const * >( bmpdata ), rowByteCount * height );
    delete[] bmpdata;

    output.close( );

    return * this;
}

Path const & Path::dumpTga( boost::uint16_t width, boost::uint16_t height, std::vector< boost::uint32_t > const & data ) const
{
    boost::filesystem::path pathname( this->string( ) );

    std::string dirname = pathname.parent_path( ).string( );
    if ( ! dirname.empty( ) )
        boost::filesystem::create_directories( dirname );

    std::ofstream output;
    output.exceptions( std::ios_base::failbit | std::ios_base::badbit );
    output.open( pathname.string( ).c_str( ), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc );

    boost::uint16_t littleEndianWidth = native_to_little_u16( width );
    boost::uint16_t littleEndianHeight = native_to_little_u16( height );

    output.write( "\x00\x00\x02", 3 );                                           // no ID field, no color map, uncompressed true-color
    output.write( "\x00\x00\x00\x00\x00", 5 );                                   // color palette (no)
    output.write( "\x00\x00\x00\x00", 4 );                                       // image origin (x:0 & y:0)
    output.write( reinterpret_cast< char const * >( & littleEndianWidth ), 2 );  // image width
    output.write( reinterpret_cast< char const * >( & littleEndianHeight ), 2 ); // image height
    output.write( "\x20", 1 );                                                   // bpp (32)
    output.write( "\x20", 1 );                                                   // descriptor (origin in upper left-hand)

    boost::uint32_t littleEndianData[ width * height ];
    std::transform( data.begin( ), data.end( ), littleEndianData, & native_to_little_u32 );
    output.write( reinterpret_cast< char const * >( & littleEndianData ), sizeof( littleEndianData ) );

    output.close( );

    return * this;
}

std::string Path::filename( void ) const
{
    return boost::filesystem::path( this->string( ) ).filename( ).string( );
}

std::string Path::string( void ) const
{
    std::string final;
    bool isFirst = true;

    std::list< std::string>::const_iterator it;
    for ( it = m_partList.begin( ); it != m_partList.end( ); ++ it ) {

        std::string const & part = * it;

        if ( ! isFirst && part.length( ) > 0 && part[ 0 ] != '.' )
            final += '/';

        final += part;

        isFirst = false;

    }

    return final;
}
