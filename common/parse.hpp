#pragma once

#include <boost/spirit/include/qi.hpp>

#include <iostream>
#include <stdexcept>

#include "memoryrange.hpp"

template <typename T>
void parse(MemoryRange & range, T const & grammar) {
  if (!boost::spirit::qi::parse(range.current(), range.end(), grammar)) {
	throw std::runtime_error("Parsing failed");
  }
}

template <typename T, typename R>
void parse(MemoryRange & range, T const & grammar, R & result) {
  if (!boost::spirit::qi::parse(range.current(), range.end(), grammar, result)) {
	throw std::runtime_error("Parsing failed");
  }
}
