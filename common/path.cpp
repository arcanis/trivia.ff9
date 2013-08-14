#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/tokenizer.hpp>

#include <fstream>
#include <string>

#include "memoryrange.hpp"
#include "path.hpp"

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

Path const & Path::dump( MemoryRange const & range ) const
{
    boost::filesystem::path pathname( this->string( ) );

    std::string dirname = pathname.parent_path( ).string( );
    boost::filesystem::create_directories( dirname );

    std::ofstream output;
    output.exceptions( std::ios_base::failbit | std::ios_base::badbit );
    output.open( pathname.string( ).c_str( ), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc );
    output.write( range.current( ), range.end( ) - range.current( ) );
    output.close( );

    return * this;

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
