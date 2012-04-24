#include <boost/spirit/include/qi.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "memoryiterator.hpp"
#include "parse.hpp"

namespace spirit = boost::spirit;
namespace qi = spirit::qi;

////////////
// 2 bytes : file ID
// 2 bytes : - unknown -
// 4 bytes : first sector

void parseFile(MemoryIterator iterator, unsigned long fileIndex, unsigned long & endSector) {

  (void) fileIndex;

  boost::uint16_t id;
  boost::uint32_t beginSector;

  parse(iterator, qi::little_word, id);
  parse(iterator, qi::little_word);
  parse(iterator, qi::little_dword, beginSector);

  unsigned long offset = beginSector * 2048;
  unsigned long size = (endSector - beginSector) * 2048;

  (void) offset;
  (void) size;

  endSector = beginSector;

}

////////////
// 2 bytes : fragment sector, or 0xFFFF

void parseFragment(MemoryIterator iterator, unsigned long fragmentIndex, unsigned long baseSector, unsigned long & endSector) {

  (void) fragmentIndex;

  boost::uint16_t offsetSector;

  parse(iterator, qi::little_word, offsetSector);

  if (offsetSector == 0xFFFF)
	return ;

  boost::uint32_t beginSector = baseSector + offsetSector;

  unsigned long offset = beginSector * 2048;
  unsigned long size = (endSector - beginSector) * 2048;

  (void) offset;
  (void) size;

  endSector = beginSector;

}
////////////
// 4 bytes : directory type
// 4 bytes : entries count
// 4 bytes : entries list sector
// 4 bytes : base sector

void parseSubDirectory(MemoryIterator iterator, unsigned long directoryIndex, unsigned long & endSector) {

  (void) directoryIndex;

  boost::uint32_t type;
  boost::uint32_t entriesCount;
  boost::uint32_t entriesListSector;
  boost::uint32_t baseSector;

  parse(iterator, qi::little_dword, type);
  parse(iterator, qi::little_dword, entriesCount);
  parse(iterator, qi::little_dword, entriesListSector);
  parse(iterator, qi::little_dword, baseSector);

  iterator.seek(MemoryIterator::SeekSet, entriesListSector * 2048);

  for (unsigned long entryIndex = entriesCount; entryIndex --; ) {

	MemoryIterator subIterator(iterator);

	if (type == 0x02) {
	  subIterator.seek(MemoryIterator::SeekCur, 8 * entryIndex);
	  parseFile(subIterator, entryIndex, endSector);
	} else if (type == 0x03) {
	  subIterator.seek(MemoryIterator::SeekCur, 2 * entryIndex);
	  parseFragment(subIterator, entryIndex, baseSector, endSector);
	}

  }

  if (type == 0x04) {
	endSector = (iterator.end() - iterator.begin()) / 2048;
  } else {
	endSector = baseSector;
  }

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
	parseSubDirectory(subIterator, directoryIndex, nextFileSectorOffset);

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
