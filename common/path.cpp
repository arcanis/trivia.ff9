#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/tokenizer.hpp>

#include <fstream>
#include <string>

#include "memoryrange.hpp"
#include "path.hpp"

Path::Path(void) {

}

Path::Path(std::string const & orig) {

  typedef boost::char_separator<char> separator;
  typedef boost::tokenizer<separator> tokenizer;

  separator sep("/");
  tokenizer tokens(orig, sep);

  for (tokenizer::iterator it = tokens.begin(); it != tokens.end(); ++ it) {
	this->m_partList.push_back(*it);
  }

}

Path const & Path::dump(MemoryRange const & range) const {

  boost::filesystem::path pathname(this->string());

  std::string dirname = pathname.parent_path().string();
  boost::filesystem::create_directories(dirname);

  std::ofstream output;
  output.exceptions(std::ios_base::failbit | std::ios_base::badbit);
  output.open(pathname.string().c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  output.write(range.current(), range.end() - range.current());
  output.close();

  return *this;

}

std::string Path::string(void) const {

  return boost::algorithm::join(this->m_partList, "/");

}
