#include <stdexcept>

#include "memoryrange.hpp"

MemoryRange::MemoryRange(char const * begin, char const * end) {

  if (begin > end) {

	char const * temp;

	temp = begin;
	begin = end;
	end = temp;

  }

  this->m_begin = this->m_current = begin;
  this->m_end = end;

}

MemoryRange & MemoryRange::seek(SeekSource whence, long relativeOffset) {

  char const * current;

  switch (whence) {
  case SeekSet:
	current = this->m_begin + relativeOffset;
	break;
  case SeekCur:
	current = this->m_current + relativeOffset;
	break;
  case SeekEnd:
	current = this->m_end - relativeOffset;
	break;
  }

  if (current < this->m_begin || current > this->m_end)
	throw std::out_of_range("Invalid seek");

  this->m_current = current;

  return *this;

}

MemoryRange & MemoryRange::crop(unsigned long absoluteOffset, unsigned long size)
{

  char const * begin = this->m_begin + absoluteOffset;

  if (begin > this->m_end)
	throw std::out_of_range("Invalid crop");

  char const * end = begin + size;

  if (end > this->m_end)
	throw std::out_of_range("Invalid crop");

  this->m_begin = this->m_current = begin;
  this->m_end = end;

  return *this;

}
