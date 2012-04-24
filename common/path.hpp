#pragma once

#include <list>
#include <string>

#include "memoryrange.hpp"

class Path {

public:
  Path(void);
  Path(std::string const & orig);

public:
  Path & push(std::string const & part) {

	this->m_partList.push_back(part);

	return *this;

  }

  Path & pop(void) {

	this->m_partList.pop_back();

	return *this;

  }

public:
  std::string string(void) const;

public:
  Path const & dump(MemoryRange const & range) const;

private:
  std::list<std::string> m_partList;

};
