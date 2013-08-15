#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/spirit/include/qi.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "constants.hpp"
#include "memoryrange.hpp"
#include "parse.hpp"
#include "path.hpp"

namespace po = boost::program_options;
namespace qi = boost::spirit::qi;

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

    unsigned long offset = beginSector * SECTOR_LENGTH;
    unsigned long size = ( endSector - beginSector ) * SECTOR_LENGTH;

    MemoryRange dataRange( range );
    dataRange.crop( MemoryRange::SeekSet, offset, size );

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

    unsigned long offset = beginSector * SECTOR_LENGTH;
    unsigned long size = ( endSector - beginSector ) * SECTOR_LENGTH;

    MemoryRange dataRange( range );
    dataRange.crop( MemoryRange::SeekSet, offset, size );

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

    range.seek( MemoryRange::SeekSet, entryListSector * SECTOR_LENGTH );

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
        endSector = ( range.end( ) - range.begin( ) ) / SECTOR_LENGTH;
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

        parseImage( range, output );

        return 0;

    } else {

        std::cerr << "Usage: " << argv[ 0 ] << " [options] <FF9.IMG path> <destination path>" << std::endl;
        std::cerr << options;

        return -1;

    }
}
