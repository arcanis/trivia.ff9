#pragma once

#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/tokenizer.hpp>

#include <iostream>
#include <list>
#include <string>

#include "memoryrange.hpp"

class Path {

public:
  Path(std::string const & orig) {
	typedef boost::char_separator<char> separator;
	typedef boost::tokenizer<separator> tokenizer;

	separator sep("/");
	tokenizer tokens(orig, sep);

	for (tokenizer::iterator it = tokens.begin(); it != tokens.end(); ++ it) {
	  m_partList.push_back(*it);
	}
  }

public:
  Path const & dump(MemoryRange const & range) {
	boost::filesystem::path pathname(str());

	std::string dirname = pathname.parent_path().string();
	boost::filesystem::create_directories(dirname);

	std::ofstream output;
	output.exceptions(std::ios_base::failbit | std::ios_base::badbit);
	output.open(pathname.string().c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
	output.write(range.current(), range.end() - range.current());
	output.close();

	return *this;
  }

public:
  std::string str(void) const {
	return boost::algorithm::join(m_partList, "/");
  }

public:
  Path & push(std::string const & part) {
	m_partList.push_back(part);
	return *this;
  }

  Path & pop(void) {
	m_partList.pop_back();
	return *this;
  }

private:
  std::list<std::string> m_partList;

};
