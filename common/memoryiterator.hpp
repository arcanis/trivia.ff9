#pragma once

class MemoryIterator {

public:
  enum SeekSource {
	SeekSet,
	SeekCur,
	SeekEnd
  };

public:
  MemoryIterator(char const * begin, char const * end) {

	m_begin = m_current = begin;
	m_end = end;

  }

  MemoryIterator(MemoryIterator const & other) {

	m_begin = other.m_begin;
	m_current = other.m_current;
	m_end = other.m_end;

  }

public:
  MemoryIterator & seek(SeekSource whence, long offset) {

	switch (whence) {
	case SeekSet:
	  m_current = m_begin + offset;
	  break;
	case SeekCur:
	  m_current += offset;
	  break;
	case SeekEnd:
	  m_current = m_end - offset;
	  break;
	}

	return *this;

  }

  MemoryIterator & crop(unsigned long offset, unsigned long size) {

	m_current = m_begin += offset;
	m_end = m_begin + size;

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
