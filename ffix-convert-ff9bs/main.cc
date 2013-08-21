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

void parseTexture( VRAM const & vram, MemoryRange range, Path outputPath, boost::uint16_t textureIndex )
{
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
            boost::uint8_t const * binaryvram = reinterpret_cast< boost::uint8_t const * >( & vram );
            boost::uint32_t absoluteX = texX * BATTLESCENE_CELL_WIDTH + x;
            boost::uint32_t absoluteY = texY * BATTLESCENE_CELL_HEIGHT + y;
            data[ y * BATTLESCENE_TEXTURE_WIDTH + x ] = palette[ binaryvram[ absoluteY * VRAM_WIDTH * 2 + absoluteX ] ];
        }
    }

    // Store to disk

    outputPath.dumpBmp( BATTLESCENE_TEXTURE_WIDTH, BATTLESCENE_TEXTURE_HEIGHT, data );

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

    parse( range, qi::word );
    parse( range, qi::word );
    parse( range, qi::word );
    parse( range, qi::word );

    std::cout << "Object count  : " << objectCount << std::endl;
    std::cout << "Texture count : " << textureCount << std::endl;
    std::cout << std::endl;

    std::ostringstream material;
    std::ostringstream geometry;

    std::cout << "Parsing textures :" << std::endl;

    for ( boost::uint16_t textureIndex = 0; textureIndex < textureCount; ++ textureIndex ) {

        std::ostringstream pathBuilder;
        pathBuilder << std::setfill( '0' ) << std::setw( 3 ) << textureIndex << ".bmp";

        Path subOutputPath( outputPath );
        subOutputPath.push( pathBuilder.str( ) );

        std::cout << std::endl;
        std::cout << " - Processing texture #" << textureIndex << std::endl;

        MemoryRange subTexturesRange( range );
        subTexturesRange.seek( MemoryRange::SeekSet, texturesOffset );
        subTexturesRange.seek( MemoryRange::SeekCur, textureIndex * 4 );

        parseTexture( vram, subTexturesRange, subOutputPath, textureIndex );

        material << "newmtl tex" << static_cast< int >( textureIndex ) << std::endl;
        material << "Ka 1 1 1" << std::endl;
        material << "Kd 1 1 1" << std::endl;
        material << "Ks 0 0 0" << std::endl;
        material << "d 1" << std::endl;
        material << "illum 0" << std::endl;
        material << "map_Kd " << subOutputPath.filename( ) << std::endl;

    }

    std::cout << std::endl;

    std::cout << "Parsing geometry :" << std::endl;

    geometry << "mtllib materials.mtl" << std::endl;

    MemoryRange verticesRange( range );
    verticesRange.seek( MemoryRange::SeekSet, verticesOffset );

    for ( boost::uint16_t objectIndex = 0, verticesStart = 0, uvStart = 0; objectIndex < objectCount; ++ objectIndex ) {

        boost::uint16_t timp;
        parse( range, qi::word, timp );

        boost::uint16_t verticeCount;
        parse( range, qi::word, verticeCount );

        parse( range, qi::word );

        boost::uint16_t texidxOffset, facesOffset, texmapOffset;
        parse( range, qi::word, texidxOffset );
        parse( range, qi::word, facesOffset );
        parse( range, qi::word, texmapOffset );

        boost::uint16_t rectangleCount, triangleCount;
        parse( range, qi::word, rectangleCount );
        parse( range, qi::word, triangleCount );

        boost::uint32_t totalVerticeCount = rectangleCount * 4 + triangleCount * 3;

        std::cout << std::endl;
        std::cout << " - Processing object #" << static_cast< int >( objectIndex ) << std::endl;
        std::cout << "   Vertice count   : " << static_cast< int >( verticeCount ) << std::endl;
        std::cout << "   Rectangle count : " << static_cast< int >( rectangleCount ) << std::endl;
        std::cout << "   Triangle count  : " << static_cast< int >( triangleCount ) << std::endl;

        for ( boost::uint16_t verticeIndex = 0; verticeIndex < verticeCount; ++ verticeIndex ) {

            boost::int16_t x, y, z;
            parse( verticesRange, qi::word, x );
            parse( verticesRange, qi::word, y );
            parse( verticesRange, qi::word, z );

            double dx = + static_cast< double >( x ) / 100.0;
            double dy = - static_cast< double >( y ) / 100.0;
            double dz = + static_cast< double >( z ) / 100.0;

            geometry << "v " << std::fixed << dx << " " << std::fixed << dy << " " << std::fixed << dz << std::endl;

        }

        MemoryRange texmapRange( range );
        texmapRange.seek( MemoryRange::SeekCur, - 16 );
        texmapRange.seek( MemoryRange::SeekCur, texmapOffset );

        for ( boost::uint32_t verticeIndex = 0; verticeIndex < totalVerticeCount; ++ verticeIndex ) {

            boost::uint8_t tx, ty;
            parse( texmapRange, qi::byte_, tx );
            parse( texmapRange, qi::byte_, ty );

            double dtx = 0.0 + static_cast< double >( tx ) / 255.0;
            double dty = 1.0 - static_cast< double >( ty ) / 255.0;

            geometry << "vt" << " " << std::fixed << dtx << " " << std::fixed << dty << std::endl;

        }

        MemoryRange facesRange( range );
        facesRange.seek( MemoryRange::SeekCur, - 16 );
        facesRange.seek( MemoryRange::SeekCur, facesOffset );

        MemoryRange texidxRange( range );
        texidxRange.seek( MemoryRange::SeekCur, - 16 );
        texidxRange.seek( MemoryRange::SeekCur, texidxOffset );

        boost::int16_t previousTexidx = - 1;

        for ( boost::uint16_t rectangleIndex = 0; rectangleIndex < rectangleCount; ++ rectangleIndex ) {

            boost::uint32_t texidx;
            parse( texidxRange, qi::dword, texidx );
            texidx = ( texidx >> 24 ) & 0x1f;

            boost::uint16_t v1, v2, v3, v4;
            parse( facesRange, qi::word, v1 ); v1 /= 4;
            parse( facesRange, qi::word, v2 ); v2 /= 4;
            parse( facesRange, qi::word, v3 ); v3 /= 4;
            parse( facesRange, qi::word, v4 ); v4 /= 4;

            if ( texidx != previousTexidx ) {
                geometry << "usemtl tex" << texidx << std::endl;
                previousTexidx = texidx;
            }

            geometry << "f " << ( verticesStart + v1 + 1 ) << "/" << ( uvStart + rectangleIndex * 4 + 1 )
                     << " "  << ( verticesStart + v2 + 1 ) << "/" << ( uvStart + rectangleIndex * 4 + 2 )
                     << " "  << ( verticesStart + v3 + 1 ) << "/" << ( uvStart + rectangleIndex * 4 + 3 )
            << std::endl;

            geometry << "f " << ( verticesStart + v4 + 1 ) << "/" << ( uvStart + rectangleIndex * 4 + 4 )
                     << " "  << ( verticesStart + v3 + 1 ) << "/" << ( uvStart + rectangleIndex * 4 + 3 )
                     << " "  << ( verticesStart + v2 + 1 ) << "/" << ( uvStart + rectangleIndex * 4 + 2 )
            << std::endl;

        }

        for ( boost::uint16_t triangleIndex = 0; triangleIndex < triangleCount; ++ triangleIndex ) {

            boost::uint32_t texidx;
            parse( texidxRange, qi::dword, texidx );
            texidx = ( texidx >> 24 ) & 0x1f;

            boost::uint16_t v1, v2, v3;
            parse( facesRange, qi::word, v1 ); v1 /= 4;
            parse( facesRange, qi::word, v2 ); v2 /= 4;
            parse( facesRange, qi::word, v3 ); v3 /= 4;

            if ( texidx != previousTexidx ) {
                geometry << "usemtl tex" << texidx << std::endl;
                previousTexidx = texidx;
            }

            geometry << "f " << ( verticesStart + v1 + 1 ) << "/" << ( uvStart + rectangleCount * 4 + triangleIndex * 3 + 1 )
                     << " "  << ( verticesStart + v2 + 1 ) << "/" << ( uvStart + rectangleCount * 4 + triangleIndex * 3 + 2 )
                     << " "  << ( verticesStart + v3 + 1 ) << "/" << ( uvStart + rectangleCount * 4 + triangleIndex * 3 + 3 )
            << std::endl;

        }

        verticesStart += verticeCount;
        uvStart += totalVerticeCount;

    }

    Path materialPath( outputPath );
    materialPath.push( "materials.mtl" );
    materialPath.dump( material.str( ) );

    Path geometryPath( outputPath );
    geometryPath.push( "geometry.obj" );
    geometryPath.dump( geometry.str( ) );
}

int main( int argc, char ** argv )
{
    std::cout.precision( 9 );

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
            std::cout << "    Import takes place at X " << tim.left( ) << ", Y " << tim.top( ) << ", W " << tim.width( ) << " and H " << tim.height( ) << " (" << static_cast< int >( tim.bpp( ) ) << " bpp)" << std::endl;
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
