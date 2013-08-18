#include <fstream>
#include <iomanip>
#include <iostream>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/cstdint.hpp>

#include "constants.hpp"
#include "memoryrange.hpp"
#include "parse.hpp"
#include "path.hpp"
#include "tim.hpp"
#include "vram.hpp"

namespace po = boost::program_options;
namespace qi = boost::spirit::qi;

boost::uint8_t primitivePalette[] = {

      0,   8,  16,  24,
     32,  40,  48,  56,
     65,  73,  81,  89,
    100, 108, 118, 125,
    135, 143, 152, 162,
    170, 178, 186, 194,
    202, 210, 217, 225,
    232, 240, 246, 255

};

//
//

void parseTextures( VRAM const & vram, MemoryRange range, Path outputPath, boost::uint16_t textureCount )
{
    for ( boost::uint16_t textureIndex = 0; textureIndex < textureCount; ++ textureIndex ) {

        // binary packet structure :
        // aaaaaaaa aabbbbbb ???????? ccccdddd
        //
        // a : palyy
        // b : palxx
        //
        // c : texture Y
        // d : texture X

        boost::uint32_t packet;
        parse( range, qi::dword, packet );

        boost::uint8_t palX = ( ( packet >> 16 ) & 0x3F ) * 16 * 2;
        boost::uint8_t palY = ( ( packet >> 22 ) );
        boost::uint8_t texX = ( packet >> 0 ) & 0xF;
        boost::uint8_t texY = ( packet >> 4 ) & 0x1;

        // Palette generation

        boost::uint32_t palette[ SIZE( BATTLESCENE_PALETTE ) ] = { };

        for ( int u = 0; u < SIZE( BATTLESCENE_PALETTE ); ++ u ) {

            boost::uint16_t colorIndex = vram[ palY * VRAM_WIDTH + palX + u ];

            palette[ u ]
                = ( primitivePalette[ ( colorIndex >>  0 ) & 0x1f ] << 16 )
                | ( primitivePalette[ ( colorIndex >>  5 ) & 0x1f ] <<  8 )
                | ( primitivePalette[ ( colorIndex >> 10 ) & 0x1f ] <<  0 )
                ;

            if ( ( colorIndex & 0x8000 ) == 0x8000 ) {
                palette[ u ] |= 0xff000000;
            }

        }

        // Image restitution

        std::vector< boost::uint32_t > data( SIZE( BATTLESCENE_TEXTURE ) );

        for ( boost::uint32_t y = 0; y < BATTLESCENE_TEXTURE_HEIGHT; ++ y ) {
            for ( boost::uint32_t x = 0; x < BATTLESCENE_TEXTURE_WIDTH; ++ x ) {
                boost::uint32_t absoluteX = texX * BATTLESCENE_CELL_WIDTH + x;
                boost::uint32_t absoluteY = texY * BATTLESCENE_CELL_HEIGHT + y;
                data[ y * BATTLESCENE_TEXTURE_HEIGHT + x ] = palette[ vram[ absoluteY * VRAM_WIDTH + absoluteX ] & 0xFF ];
            }
        }

        // Store to disk

        std::cout << std::endl;
        std::cout << " - Processing #" << textureIndex << std::endl;

        std::ostringstream pathBuilder;
        pathBuilder << std::setfill( '0' ) << std::setw( 3 ) << textureIndex << ".tga";

        Path subOutputPath( outputPath );
        subOutputPath.push( pathBuilder.str( ) );

        subOutputPath.dumpTga( BATTLESCENE_TEXTURE_WIDTH, BATTLESCENE_TEXTURE_HEIGHT, data );

    }
}

////////////
// 4 bytes : ???
// 2 bytes : object count
// 2 bytes : ???
// 2 bytes : texture count
// 2 bytes : textures offset
// 2 bytes : ???
// 2 bytes : object offset

void parseBattleScene( VRAM const & vram, MemoryRange range, Path outputPath )
{
    parse( range, qi::dword );

    boost::uint16_t objectCount;
    parse( range, qi::word, objectCount );

    parse( range, qi::word );

    boost::uint16_t textureCount;
    parse( range, qi::word, textureCount );

    boost::uint16_t texturesOffset;
    parse( range, qi::word, texturesOffset );

    parse( range, qi::word );

    boost::uint16_t verticesOffset;
    parse( range, qi::word, verticesOffset );

    std::cout << "Object count  : " << objectCount << std::endl;
    std::cout << "Texture count : " << textureCount << std::endl;
    std::cout << std::endl;

    MemoryRange subTexturesRange( range );
    subTexturesRange.seek( MemoryRange::SeekSet, texturesOffset );

    std::cout << "Parsing textures :" << std::endl;
    parseTextures( vram, subTexturesRange, outputPath, textureCount );
}

int main( int argc, char ** argv )
{
    po::options_description options( "Allowed options" );
    options.add_options( )( "tim", po::value< std::vector< std::string > >( )->default_value( std::vector< std::string >( ), "" ), "Image clusters (TIM files)" );
    options.add_options( )( "input", po::value< std::string >( )->required( ) );
    options.add_options( )( "output", po::value< std::string >( )->required( ) );

    po::positional_options_description positional;
    positional.add( "input", 1 );
    positional.add( "output", 1 );

    po::variables_map vm;
    po::store( po::command_line_parser( argc, argv ).options( options ).positional( positional ).run( ), vm );
    po::notify( vm );

    if ( vm.count( "input" ) && vm.count( "output" ) ) {

        VRAM vram = { };

        auto textures = vm[ "tim" ].as< std::vector< std::string > >( );
        for ( std::string const & path : textures ) {
            TIM tim = TIM::fromFile( path );

            std::cout << "Loading " << path << " into VRAM ..." << std::endl;
            std::cout << "    Import takes place at X " << tim.left( ) << ", Y " << tim.top( ) << ", W " << tim.width( ) << " and H " << tim.height( ) << std::endl;
            std::cout << std::endl;

            tim.apply( vram );
        }

        Path input( vm[ "input" ].as< std::string >( ) );
        Path output( vm[ "output" ].as< std::string >( ) );

        auto content = input.read( );
        MemoryRange range( content );

        parseBattleScene( vram, range, output );

        return 0;

    } else {

        std::cerr << "Usage: " << argv[ 0 ] << " [options] <source path.ff9bs> <destination path>" << std::endl;
        std::cerr << options;

        return -1;

    }
}
