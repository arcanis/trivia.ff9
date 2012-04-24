#include <boost/spirit/include/qi.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "memoryiterator.hpp"
#include "parse.hpp"

namespace spirit = boost::spirit;
namespace qi = spirit::qi;

// 2 bytes : file ID
// 2 bytes : - unknown -
// 4 bytes : first file sector

void parseFile(MemoryIterator iterator, unsigned long fileIndex, unsigned long & nextFileSectorOffset) {

  boost::uint16_t id;
  boost::uint32_t firstSector;

  parse(iterator, qi::little_word, id);
  parse(iterator, qi::little_word);
  parse(iterator, qi::little_dword, firstSector);

  unsigned long size = nextFileSectorOffset - firstSector;

  nextFileSectorOffset = firstSector;

}

// 4 bytes : directory type
// 4 bytes : files count
// 4 bytes : directory informations sector
// 4 bytes : first file sector

void parseRegularDirectory(MemoryIterator iterator, unsigned long directoryIndex, unsigned long & nextFileSectorOffset) {

  boost::uint32_t type;
  boost::uint32_t filesCount;
  boost::uint32_t directoryInformationsSector;
  boost::uint32_t firstFileSector;

  parse(iterator, qi::little_dword, type);
  parse(iterator, qi::little_dword, filesCount);
  parse(iterator, qi::little_dword, directoryInformationsSector);
  parse(iterator, qi::little_dword, firstFileSector);

  iterator.seek(MemoryIterator::SeekSet, directoryInformationsSector * 2048);

  for (unsigned long fileIndex = filesCount; fileIndex --; ) {

	MemoryIterator subIterator(iterator);
	subIterator.seek(MemoryIterator::SeekCur, 8 * fileIndex);
	parseFile(subIterator, fileIndex, nextFileSectorOffset);

  }

  nextFileSectorOffset = directoryInformationsSector;

}

// 4 bytes : magic 0x46463920
// 4 bytes : - unknown -
// 4 bytes : directories count
// 4 bytes : - unknown -

void parseRootDirectory(MemoryIterator iterator) {

  boost::uint32_t magicNumber;
  boost::uint32_t directoriesCount;

  parse(iterator, qi::big_dword, magicNumber);
  if (magicNumber != 0x46463920)
	throw std::runtime_error("Bad magic number.");

  parse(iterator, qi::little_dword);
  parse(iterator, qi::little_dword, directoriesCount);
  parse(iterator, qi::little_dword);

  unsigned long nextFileSectorOffset;

  for (unsigned long directoryIndex = directoriesCount; directoryIndex --; ) {

	MemoryIterator subIterator(iterator);
	subIterator.seek(MemoryIterator::SeekCur, 16 * directoryIndex);
	parseRegularDirectory(subIterator, directoryIndex, nextFileSectorOffset);

  }

}

int main(int argc, char ** argv) {

  if (argc > 1) {

	std::ifstream archive(argv[1], std::ios::in | std::ios::binary | std::ios::ate);
	std::ifstream::pos_type size = archive.tellg();
	archive.seekg(0, std::ios::beg);

	char * begin = new char[size];
	archive.read(begin, size);
	archive.close();

	MemoryIterator iterator(begin, begin + size);
	parseRootDirectory(iterator);

	delete[] begin;

	return 0;

  } else {

	std::cout << "Usage: " << argv[0] << " <path to FF9.IMG>" << std::endl;
	return -1;

  }

}
