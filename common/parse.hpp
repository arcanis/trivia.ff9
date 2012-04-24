#pragma once

#include <boost/spirit/include/qi.hpp>

#include <iostream>
#include <stdexcept>

#include "memoryiterator.hpp"

template <typename T>
void parse(MemoryIterator & iterator, T const & grammar) {
  try {
	boost::spirit::qi::parse(iterator.current(), iterator.end(), grammar);
  } catch (boost::spirit::qi::expectation_failure<boost::spirit::istream_iterator> const & exception) {
	throw std::runtime_error("Invalid file");
  }
}

template <typename T, typename R>
void parse(MemoryIterator & iterator, T const & grammar, R & result) {
  try {
	boost::spirit::qi::parse(iterator.current(), iterator.end(), grammar, result);
  } catch (boost::spirit::qi::expectation_failure<boost::spirit::istream_iterator> const & exception) {
	throw std::runtime_error("Invalid file");
  }
}
