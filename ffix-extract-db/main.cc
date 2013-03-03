#include <boost/spirit/include/qi.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "memoryrange.hpp"
#include "parse.hpp"
#include "path.hpp"

namespace spirit = boost::spirit;
namespace qi = spirit::qi;

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

    std::cout << "  Data type    : 0x" << std::hex << std::setfill( '0' ) << std::setw( 2 ) << dataType << std::endl;
    std::cout << "  Object count : "   << std::dec << objectCount << std::endl;
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

        std::cout << "* Extracting #" << pointerIndex << std::endl;
        parsePack( subRange, outputPath );

    }
}

int main( int argc, char ** argv )
{
    if ( argc >= 3 ) {

        std::ifstream db( argv[ 1 ], std::ios::in | std::ios::binary | std::ios::ate );
        std::ifstream::pos_type size = db.tellg( );
        db.seekg( 0, std::ios::beg );

        char * begin = new char[ size ];
        db.read( begin, size );
        db.close( );

        MemoryRange range( begin, begin + size );
        parseDB( range, Path( argv[ 2 ] ) );

        delete[] begin;

        return 0;

    } else {

        std::cerr << "Usage: " << argv[ 0 ] << " \"<source path>\" \"<destination path>\"" << std::endl;
        return -1;

    }
}
