#include <boost/spirit/include/qi.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "memoryrange.hpp"
#include "parse.hpp"
#include "path.hpp"

#define SECTOR 2048

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
// 2 bytes : file ID
// 2 bytes : - unknown -
// 4 bytes : first sector

void parseFile( MemoryRange range, Path outputPath, unsigned long & endSector )
{
    boost::uint32_t id;
    boost::uint32_t beginSector;

    parse( range, qi::little_word, id );
    parse( range, qi::little_word );
    parse( range, qi::little_dword, beginSector );

    unsigned long offset = beginSector * SECTOR;
    unsigned long size = ( endSector - beginSector ) * SECTOR;

    MemoryRange dataRange( range );
    dataRange.crop( offset, size );

    suffixize( outputPath, dataRange );
    outputPath.dump( dataRange );

    endSector = beginSector;
}

////////////
// 2 bytes : fragment sector, or 0xFFFF

void parseFragment( MemoryRange range, Path outputPath, unsigned long baseSector, unsigned long & endSector )
{
    boost::uint32_t fragmentSector;

    parse( range, qi::little_word, fragmentSector );

    if ( fragmentSector == 0xFFFF ) return ;

    boost::uint32_t beginSector = baseSector + fragmentSector;

    unsigned long offset = beginSector * SECTOR;
    unsigned long size = ( endSector - beginSector ) * SECTOR;

    MemoryRange dataRange( range );
    dataRange.crop( offset, size );

    suffixize( outputPath, dataRange );
    outputPath.dump( dataRange );

    endSector = beginSector;
}

////////////
// 4 bytes : directory type [0x02 = file, 0x03 = fragment]
// 4 bytes : entries count
// 4 bytes : entries list sector
// 4 bytes : base sector

void parseContainer( MemoryRange range, Path outputPath, unsigned long & endSector )
{
    boost::uint32_t type;
    boost::uint32_t entryCount;
    boost::uint32_t entryListSector;
    boost::uint32_t baseSector;

    parse( range, qi::little_dword, type );
    parse( range, qi::little_dword, entryCount );
    parse( range, qi::little_dword, entryListSector );
    parse( range, qi::little_dword, baseSector );

    std::cout << "  Container type : 0x" << std::hex << std::setfill( '0' ) << std::setw( 2 ) << type << std::endl;
    std::cout << "  Entry count    : "   << std::dec << entryCount << std::endl;

    range.seek( MemoryRange::SeekSet, entryListSector * SECTOR );

    for ( unsigned long entryIndex = entryCount; entryIndex --;  ) {

        MemoryRange subRange( range );

        std::stringstream pathBuilder;
        pathBuilder << std::setfill( '0' ) << std::setw( 3 ) << entryIndex;

        Path subOutputPath( outputPath );
        subOutputPath.push( pathBuilder.str( ) );

        switch ( type ) {

            case 0x02:
                subRange.seek( MemoryRange::SeekCur, 8 * entryIndex );
                parseFile( subRange, subOutputPath, endSector );
            break;

            case 0x03:
                subRange.seek( MemoryRange::SeekCur, 2 * entryIndex );
                parseFragment( subRange, subOutputPath, baseSector, endSector );
            break;

        }

    }

    if ( type == 0x04 ) {
        endSector = ( range.end( ) - range.begin( ) ) / SECTOR;
    } else {
        endSector = baseSector;
    }
}

////////////
// 4 bytes : magic 0x46463920
// 4 bytes : - unknown -
// 4 bytes : directories count
// 4 bytes : - unknown -

void parseImage( MemoryRange range, Path outputPath )
{
    boost::uint32_t magicNumber;
    boost::uint32_t containerCount;

    parse( range, qi::big_dword, magicNumber );
    if ( magicNumber != 0x46463920 )
        throw std::runtime_error( "Bad magic number." );

    parse( range, qi::little_dword );
    parse( range, qi::little_dword, containerCount );
    parse( range, qi::little_dword );

    unsigned long endSector;

    std::cout << "Container count : " << containerCount << std::endl;

    for ( unsigned long containerIndex = containerCount; containerIndex --;  ) {

        MemoryRange subRange( range );
        subRange.seek( MemoryRange::SeekCur, 16 * containerIndex );

        std::stringstream pathBuilder;
        pathBuilder << std::setfill( '0' ) << std::setw( 2 ) << containerIndex;

        Path subOutputPath( outputPath );
        subOutputPath.push( pathBuilder.str( ) );

        std::cout << std::endl << "* Extracting #" << containerIndex << std::endl;
        parseContainer( subRange, subOutputPath, endSector );

    }
}

int main( int argc, char ** argv )
{
    if ( argc >= 3 ) {

        std::ifstream archive( argv[ 1 ], std::ios::in | std::ios::binary | std::ios::ate );
        std::ifstream::pos_type size = archive.tellg( );
        archive.seekg( 0, std::ios::beg );

        char * begin = new char[ size ];
        archive.read( begin, size );
        archive.close( );

        Path outputPath( argv[ 2 ] );
        outputPath.push( "ff9" );

        MemoryRange range( begin, begin + size );
        parseImage( range, outputPath );

        delete[] begin;

        return 0;

    } else {

        std::cerr << "Usage: " << argv[ 0 ] << " \"<source path>\" \"<destination path>\"" << std::endl;
        return -1;

    }
}
