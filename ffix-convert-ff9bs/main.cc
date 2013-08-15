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

void parseTextures( VRAM const & vram, MemoryRange range, Path outputPath )
{
    int textureCount = range.size( ) / 4;

    for ( int t = 0; t < textureCount; ++ t ) {

        // binary packet structure :
        // aaaaaaaa bbbbbbbb cccccccc dddddddd
        //
        // a : palyy
        // b : palxx
        //
        // c : texture Y
        // d : texture X

        boost::uint8_t palY;
        parse( range, qi::byte_, palY );

        boost::uint8_t palX;
        parse( range, qi::byte_, palX );

        boost::uint8_t texY;
        parse( range, qi::byte_, texY );

        boost::uint8_t texX;
        parse( range, qi::byte_, texX );

        // Palette generation

        boost::uint32_t palette[ SIZE( BATTLESCENE_PALETTE ) ];

        for ( int u = 0; u < SIZE( BATTLESCENE_PALETTE ); ++ u ) {

            boost::uint16_t colorIndex = vram[ palY * VRAM_HEIGHT + palX + u ];

            palette[ u ]
                = ( primitivePalette[ ( colorIndex >>  0 ) & 0x1f ] <<  0 )
                | ( primitivePalette[ ( colorIndex >>  5 ) & 0x1f ] <<  8 )
                | ( primitivePalette[ ( colorIndex >> 10 ) & 0x1f ] << 16 )
                ;

            if ( ( colorIndex & 0x8000 ) == 0x8000 ) {
                palette[ u ] |= 0xff000000;
            }

        }

        // Image restitution

        std::vector< boost::uint32_t > data( SIZE( BATTLESCENE_TEXTURE ) );

        for ( boost::uint32_t y = 0; y < BATTLESCENE_TEXTURE_HEIGHT; ++ y ) {
            for ( boost::uint32_t x = 0; x < BATTLESCENE_TEXTURE_WIDTH; ++ x ) {
                data[ y * BATTLESCENE_TEXTURE_HEIGHT + x ] = palette[
                    + texX * BATTLESCENE_CELL_WIDTH
                    + texY * BATTLESCENE_CELL_HEIGHT * VRAM_WIDTH
                    + x
                    + y * VRAM_WIDTH
                ];
            }
        }

        // Store to disk

        outputPath.dumpTga( BATTLESCENE_TEXTURE_WIDTH, BATTLESCENE_TEXTURE_HEIGHT, data );

    }
}

////////////
// 2 bytes : size
// 2 bytes : ???
// 2 bytes : object count
// 2 bytes : object structures offset
// 2 bytes : GPU dummy packet count
// 2 bytes : vertex count
// 2 bytes : 0b00000111
// 2 bytes : 0b00000111
// 2 bytes : 0b00000111
// 2 bytes : 0b00000111
// 2 bytes : 0b00000111
// 2 bytes : 0b00000111

void parseBattleScene( VRAM const & vram, MemoryRange range, Path outputPath )
{
    //
}

int main( int argc, char ** argv )
{
    po::options_description options( "Allowed options" );
    options.add_options( )( "texture", po::value< std::vector< std::string > >( ) );
    options.add_options( )( "input", po::value< std::string >( ) );
    options.add_options( )( "output", po::value< std::string >( ) );

    po::positional_options_description positional;
    positional.add( "input", 1 );
    positional.add( "output", 1 );

    po::variables_map vm;
    po::store( po::command_line_parser( argc, argv ).options( options ).positional( positional ).run( ), vm );
    po::notify( vm );

    if ( vm.count( "input" ) && vm.count( "output" ) ) {

        VRAM vram;

        std::vector< std::string > textures = vm[ "textures" ].as< std::vector< std::string > >( );
        for ( std::vector< std::string >::iterator it = textures.begin( ), end = textures.end( ); it != end; ++ it )
            TIM::fromFile( * it ).apply( vram );

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
