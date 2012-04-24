#pragma once

class MemoryRange {

public:
  enum SeekSource {
	SeekSet,
	SeekCur,
	SeekEnd
  };

public:
  MemoryRange(char const * begin, char const * end) {

	if (begin > end) {
	  char const * temp;

	  temp = begin;
	  begin = end;
	  end = temp;
	}

	m_begin = m_current = begin;
	m_end = end;

  }

public:
  MemoryRange & seek(SeekSource whence, long offset) {

	char const * current;

	switch (whence) {
	case SeekSet:
	  current = m_begin + offset;
	  break;
	case SeekCur:
	  current = m_current + offset;
	  break;
	case SeekEnd:
	  current = m_end - offset;
	  break;
	}

	if (current < m_begin || current > m_end)
	  throw std::out_of_range("Invalid seek");

	m_current = current;

	return *this;

  }

  MemoryRange & crop(unsigned long offset, unsigned long size) {

	char const * begin = m_begin + offset;

	if (begin > m_end)
	  throw std::out_of_range("Invalid crop");

	char const * end = begin + size;

	if (end > m_end)
	  throw std::out_of_range("Invalid crop");

	m_begin = m_current = begin;
	m_end = end;

	return *this;

  }

public:
  char const * begin(void) const {
	return m_begin;
  }

  char const * end(void) const {
	return m_end;
  }

public:
  char const * current(void) const {
	return m_current;
  }

  char const * & current(void) {
	return m_current;
  }

private:
  char const * m_begin;
  char const * m_current;
  char const * m_end;

};
