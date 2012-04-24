#pragma once

#include <fstream>
#include <string>

#include "memoryiterator.hpp"

void memoryDump(MemoryIterator iterator, std::string const & path)
{
  std::ofstream output(path.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  output.write(iterator.current(), iterator.end() - iterator.begin());
  output.close();
}
