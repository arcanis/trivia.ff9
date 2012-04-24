#include <boost/spirit/include/qi.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "memoryrange.hpp"
#include "parse.hpp"
#include "path.hpp"

#define SECTOR 2048

namespace spirit = boost::spirit;
namespace qi = spirit::qi;

////////////
// 2 bytes : file ID
// 2 bytes : - unknown -
// 4 bytes : first sector

void parseFile(MemoryRange range, Path outputPath, unsigned long fileIndex, unsigned long & endSector) {

  boost::uint16_t id;
  boost::uint32_t beginSector;

  parse(range, qi::little_word, id);
  parse(range, qi::little_word);
  parse(range, qi::little_dword, beginSector);

  unsigned long offset = beginSector * SECTOR;
  unsigned long size = (endSector - beginSector) * SECTOR;

  MemoryRange dataRange(range);
  dataRange.crop(offset, size);

  std::stringstream pathBuilder;
  pathBuilder << std::setfill('0') << std::setw(3) << fileIndex;
  outputPath.push(pathBuilder.str());
  outputPath.dump(dataRange);

  endSector = beginSector;

}

////////////
// 2 bytes : fragment sector, or 0xFFFF

void parseFragment(MemoryRange range, Path outputPath, unsigned long fragmentIndex, unsigned long baseSector, unsigned long & endSector) {

  boost::uint16_t fragmentSector;

  parse(range, qi::little_word, fragmentSector);

  if (fragmentSector == 0xFFFF) return ;

  boost::uint32_t beginSector = baseSector + fragmentSector;

  unsigned long offset = beginSector * SECTOR;
  unsigned long size = (endSector - beginSector) * SECTOR;

  MemoryRange dataRange(range);
  dataRange.crop(offset, size);

  std::stringstream pathBuilder;
  pathBuilder << std::setfill('0') << std::setw(3) << fragmentIndex;
  outputPath.push(pathBuilder.str());
  outputPath.dump(dataRange);

  endSector = beginSector;

}

////////////
// 4 bytes : directory type
// 4 bytes : entries count
// 4 bytes : entries list sector
// 4 bytes : base sector

void parseSubDirectory(MemoryRange range, Path outputPath, unsigned long directoryIndex, unsigned long & endSector) {

  boost::uint32_t type;
  boost::uint32_t entriesCount;
  boost::uint32_t entriesListSector;
  boost::uint32_t baseSector;

  parse(range, qi::little_dword, type);
  parse(range, qi::little_dword, entriesCount);
  parse(range, qi::little_dword, entriesListSector);
  parse(range, qi::little_dword, baseSector);

  std::stringstream pathBuilder;
  pathBuilder << std::setfill('0') << std::setw(2) << directoryIndex;
  outputPath.push(pathBuilder.str());

  range.seek(MemoryRange::SeekSet, entriesListSector * SECTOR);

  for (unsigned long entryIndex = entriesCount; entryIndex --; ) {

	MemoryRange subRange(range);

	if (type == 0x02) {
	  subRange.seek(MemoryRange::SeekCur, 8 * entryIndex);
	  parseFile(subRange, outputPath, entryIndex, endSector);
	} else if (type == 0x03) {
	  subRange.seek(MemoryRange::SeekCur, 2 * entryIndex);
	  parseFragment(subRange, outputPath, entryIndex, baseSector, endSector);
	}

  }

  if (type == 0x04) {
	endSector = (range.end() - range.begin()) / SECTOR;
  } else {
	endSector = baseSector;
  }

}

// 4 bytes : magic 0x46463920
// 4 bytes : - unknown -
// 4 bytes : directories count
// 4 bytes : - unknown -

void parseRootDirectory(MemoryRange range, Path outputPath) {

  boost::uint32_t magicNumber;
  boost::uint32_t directoriesCount;

  parse(range, qi::big_dword, magicNumber);
  if (magicNumber != 0x46463920)
	throw std::runtime_error("Bad magic number.");

  parse(range, qi::little_dword);
  parse(range, qi::little_dword, directoriesCount);
  parse(range, qi::little_dword);

  outputPath.push("ff9");

  unsigned long endSector;

  for (unsigned long directoryIndex = directoriesCount; directoryIndex --; ) {

	MemoryRange subRange(range);
	subRange.seek(MemoryRange::SeekCur, 16 * directoryIndex);

	parseSubDirectory(subRange, outputPath, directoryIndex, endSector);

  }

}

int main(int argc, char ** argv) {

  if (argc >= 3) {

	std::ifstream archive(argv[1], std::ios::in | std::ios::binary | std::ios::ate);
	std::ifstream::pos_type size = archive.tellg();
	archive.seekg(0, std::ios::beg);

	char * begin = new char[size];
	archive.read(begin, size);
	archive.close();

	MemoryRange range(begin, begin + size);
	parseRootDirectory(range, Path(argv[2]));

	delete[] begin;

	return 0;

  } else {

	std::cout << "Usage: " << argv[0] << " <path to FF9.IMG> <path to extract>" << std::endl;
	return -1;

  }

}
