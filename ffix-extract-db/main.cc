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

void suffixize( Path & outputPath, MemoryRange range )
{
    boost::uint32_t mime;
    std::string suffix = ".raw";

    try {

        parse( range, qi::byte_, mime );

        if ( mime == 0xDB )
            suffix = ".ff9db";

    } catch ( ... ) {

    }

    outputPath.push( suffix );
}

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
        case 0x04 : extension = ".ff9id"; break ;
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
