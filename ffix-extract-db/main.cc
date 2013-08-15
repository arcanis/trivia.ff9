#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/spirit/include/qi.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "memoryrange.hpp"
#include "parse.hpp"
#include "path.hpp"

namespace po = boost::program_options;
namespace qi = boost::spirit::qi;

////////////
// 1 byte  : data type
// 1 byte  : object count
// 2 bytes : padding (0x0000)

void parsePack( MemoryRange range, Path outputPath )
{
    boost::uint32_t dataType;
    boost::uint32_t objectCount;

    parse( range, qi::byte_, dataType );
    parse( range, qi::byte_, objectCount );
    parse( range, qi::word );

    std::string extension;

    switch ( dataType ) {
        case 0x04 : extension = ".tim";   break ;
        case 0x0C : extension = ".ff9bs"; break ;
        case 0x1B : extension = ".ff9db"; break ;

        default :
            std::ostringstream extensionBuilder;
            extensionBuilder << ".raw" << std::hex << std::setfill( '0' ) << std::setw( 2 ) << dataType;
            extension = extensionBuilder.str( );
        break ;
    }

    std::cout << "  Data type    : 0x" << std::hex << std::setfill( '0' ) << std::setw( 2 ) << dataType << " (" << extension << ")" << std::endl;
    std::cout << "  Object count : "   << std::dec << objectCount << std::endl;

    #define CEIL_FACTOR( N, F ) ( ( N ) % ( F ) == 0 ? ( N ) : ( N ) + ( F ) - ( ( N ) % ( F ) ) )
    int identifierSize = CEIL_FACTOR( objectCount * 2, 4 );
    int startSize = CEIL_FACTOR( objectCount * 4, 4 );

    boost::uint32_t end;

    MemoryRange endRange( range );
    endRange.seek( MemoryRange::SeekCur, identifierSize + startSize );
    parse( endRange, qi::little_dword, end );

    for ( unsigned long objectIndex = objectCount; objectIndex --; ) {

        boost::uint32_t identifier, start;

        MemoryRange identifierRange( range );
        identifierRange.seek( MemoryRange::SeekCur, 0 );
        identifierRange.seek( MemoryRange::SeekCur, objectIndex * 2 );
        parse( identifierRange, qi::little_word, identifier );

        MemoryRange startRange( range );
        startRange.seek( MemoryRange::SeekCur, identifierSize );
        startRange.seek( MemoryRange::SeekCur, objectIndex * 4 );
        parse( startRange, qi::little_dword, start );

        unsigned long size = end - start;

        std::cout << std::endl;
        std::cout << "  * Unpacking #" << objectIndex << std::endl;
        std::cout << "    Identifier    : " << identifier << std::endl;
        std::cout << "    Pointer start : " << start << std::endl;
        std::cout << "    Size          : " << size << " byte(s)" << std::endl;

        MemoryRange dataRange( range );
        dataRange.seek( MemoryRange::SeekCur, identifierSize + objectIndex * 4 );
        dataRange.crop( MemoryRange::SeekCur, start, size );

        std::stringstream pathBuilder;
        pathBuilder << std::setfill( '0' ) << std::setw( 3 ) << objectIndex << extension;

        Path subOutputPath( outputPath );
        subOutputPath.push( pathBuilder.str( ) );

        subOutputPath.dump( dataRange );

        end = start;

    }
}

////////////
// 1 byte  : magic 0xDB
// 1 byte  : pointers count
// 2 bytes : padding (0x0000)
//
// Each pointer :
//
// 3 bytes : pointer
// 1 byte  : data type

void parseDB( MemoryRange range, Path outputPath )
{
    boost::uint32_t magicNumber;
    boost::uint32_t pointerCount;

    parse( range, qi::byte_, magicNumber );
    if ( magicNumber != 0xDB )
        throw std::runtime_error( "Bad magic number." );

    parse( range, qi::byte_, pointerCount );
    parse( range, qi::word );

    std::cout << "Pointer count : " << pointerCount << std::endl;

    for ( unsigned long pointerIndex = 0; pointerIndex < pointerCount; ++ pointerIndex ) {

        boost::uint32_t pointer;
        parse( range, qi::little_dword, pointer );
        pointer = pointer & 0xFFFFFF;

        MemoryRange subRange( range );
        subRange.seek( MemoryRange::SeekCur, pointer - 4 );

        std::stringstream pathBuilder;
        pathBuilder << std::setfill( '0' ) << std::setw( 3 ) << pointerIndex;

        Path subOutputPath( outputPath );
        subOutputPath.push( pathBuilder.str( ) );

        std::cout << std::endl << "* Extracting #" << pointerIndex << std::endl;
        parsePack( subRange, subOutputPath );

    }
}

int main( int argc, char ** argv )
{
    po::options_description options( "Allowed options" );
    options.add_options( )( "input", po::value< std::string >( ) );
    options.add_options( )( "output", po::value< std::string >( ) );

    po::positional_options_description positional;
    positional.add( "input", 1 );
    positional.add( "output", 1 );

    po::variables_map vm;
    po::store( po::command_line_parser( argc, argv ).options( options ).positional( positional ).run( ), vm );
    po::notify( vm );

    if ( vm.count( "input" ) && vm.count( "output" ) ) {

        Path input( vm[ "input" ].as< std::string >( ) );
        Path output( vm[ "output" ].as< std::string >( ) );

        auto content = input.read( );
        MemoryRange range( content );

        parseDB( range, output );

        return 0;

    } else {

        std::cerr << "Usage: " << argv[ 0 ] << " [options] <*.ff9db path> <destination path>" << std::endl;
        std::cerr << options;

        return -1;

    }
}
